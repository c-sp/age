'use strict';

const https = require('https');

// we need a token to access the GitLab API
const gitlab_api_token = process.env.GITLAB_API_TOKEN;

const assemble_pages_job_name = 'assemble-pages';


(async function main() {
    if (!gitlab_api_token) {
        throw new Error('GITLAB_API_TOKEN not set');
    }

    // collect information for all AGE CI jobs
    console.log('requesting AGE CI job list ...');
    let jobs = await get_age_ci_jobs();
    console.log(`found ${jobs.length} AGE CI jobs`);

    // find and sort assemble-pages jobs (first by ref, then by finished_at)
    jobs = jobs
        .filter(job => job.name === assemble_pages_job_name)
        .sort(compare_assemble_pages);

    console.log(`found ${jobs.length} '${assemble_pages_job_name}' jobs:`);
    jobs.forEach(job => console.log(`   ${job.finished_at} ${job.artifacts_file ? '(A)' : '   '} ${job.ref}`));

    // TODO download & extract pages artifacts, put master into root

    // TODO cleanup garbage: delete artifacts that were not deployed

})()
    .then(() => process.exit(0))
    .catch(err => {
        console.error('ERROR', err);
        process.exit(1);
    });


async function get_age_ci_jobs() {
    const result = [];
    let last_page = 0;
    let finished = false;

    while (!finished) {
        const jobs_list = await Promise.all(
            // https://itnext.io/heres-why-mapping-a-constructed-array-doesn-t-work-in-javascript-f1195138615a
            [...new Array(10)]
                .map(async () => {
                    const page = ++last_page;
                    const url = `https://gitlab.com/api/v4/projects/csprenger%2FAGE/jobs?per_page=100&page=${page}`;
                    return await https_get(url);
                })
        );

        jobs_list.forEach(jobs => {
            if (!jobs.length) {
                finished = true;
            } else {
                result.push(...jobs);
            }
        });
    }

    return result;
}


async function https_get(url) {
    return new Promise((resolve, reject) => {
        const options = {
            headers: {
                'Private-Token': gitlab_api_token
            }
        };

        https.get(url, options, res => {
            let body = '';

            res.setEncoding('utf8');
            res.on('data', data => body += data);

            res.on('end', () => {
                if (res.statusCode !== 200) {
                    reject(`${url}: ${body}`);
                } else {
                    const body_json = JSON.parse(body);
                    resolve(body_json);
                }
            });

        }).on('error', (e) => {
            reject(e);
        });
    });
}


function compare_assemble_pages(a, b) {
    const ref_compare = a.ref.localeCompare(b.ref);
    return ref_compare
        // same ref => newest first
        || new Date(b.finished_at).getTime() - new Date(a.finished_at).getTime();
}
