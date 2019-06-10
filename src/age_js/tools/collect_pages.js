'use strict';

const https = require('https');
// const path = require('path');
// const fs = require('fs');

// we need a token to access the GitLab API
const gitlab_api_token = process.env.GITLAB_API_TOKEN;

const assemble_pages_job_name = 'assemble-pages';
const age_project_id = '2686832';


(async function main() {
    if (!gitlab_api_token) {
        throw new Error('GITLAB_API_TOKEN not set');
    }

    // process.argv.shift(); // node path
    // process.argv.shift(); // script path
    // if (process.argv.length !== 1) {
    //     throw new Error(`expected exactly 1 argument, found ${process.argv.length} instead`);
    // }
    // const out_dir = prepare_out_dir(process.argv[0]);

    // get active branches
    console.log('\nrequesting AGE branches ...');
    const branches = await https_request('GET', '/repository/branches?per_page=100');
    const branches_map = branches.reduce((prev, cur) => Object.assign(prev, {...prev, [cur.name]: {}}), {});
    console.log(`found ${branches.length} active branch(es):\n    ${Object.keys(branches_map).sort().join('\n    ')}`);

    // collect information for all AGE CI jobs
    console.log('\nrequesting AGE CI job list ...');
    let jobs = await get_age_ci_jobs();
    console.log(`found ${jobs.length} AGE CI jobs`);

    // find and sort assemble-pages jobs with artifact (first by ref, then by finished_at)
    jobs = jobs
        .filter(job => job.name === assemble_pages_job_name)
        .filter(job => !!job.artifacts_file)
        .sort(compare_assemble_pages);

    console.log(`found ${jobs.length} '${assemble_pages_job_name}' jobs:`);
    let will_delete_jobs = false;
    jobs.forEach(job => {
        // branch still exists and it's the latest job => use this artifact
        if (branches_map[job.ref] && !branches_map[job.ref].id) {
            branches_map[job.ref] = job;
        } else {
            will_delete_jobs = job.__delete = true;
        }
        console.log(`    ${job.finished_at}  ${job.__delete ? 'D' : ' '} ${job.ref}  (id ${job.id})`);
    });

    // download & extract pages artifacts, put master into root
    console.log(`\ndownloading & extracting artifacts ...`);
    await Promise.all(
        jobs.filter(job => !job.__delete)
            .map(async job => {
                const artifacts = await https_request('GET', `/jobs/${job.id}/artifacts`);
                // TODO
                console.log(`    ${job.finished_at}    ${job.ref}  (id ${job.id})`);
                console.log('###', artifacts);
            })
    );

    // cleanup garbage: delete artifacts that were not deployed
    if (will_delete_jobs) {
        console.log(`\ndeleting old artifacts ...`);
        await Promise.all(
            jobs.filter(job => job.__delete)
                .map(async job => {
                    await https_request('DELETE', `/jobs/${job.id}/artifacts`);
                    console.log(`    ${job.finished_at}    ${job.ref}  (id ${job.id})`);
                })
        );
    }

    console.log('');
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
            [...new Array(10)].map(async () => {
                const page = ++last_page;
                return await https_request('GET', `/jobs?per_page=100&page=${page}`);
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


async function https_request(method, path) {
    return new Promise((resolve, reject) => {
        const options = {
            host: 'gitlab.com',
            path: `/api/v4/projects/${age_project_id}${path}`,
            method,
            headers: {
                'Private-Token': gitlab_api_token,
            }
        };

        https.request(options, res => {
            let body = '';

            res.setEncoding('utf8');
            res.on('data', data => body += data);

            res.on('end', () => {
                if (res.statusCode === 200) {
                    const body_json = JSON.parse(body);
                    resolve(body_json);
                } else if (res.statusCode < 200 || res.statusCode >= 300) {
                    reject(`${method}  ${options.path}  ${res.statusCode}  ${body}`);
                } else {
                    resolve(); // e.g. 204 - no further data
                }
            });

        }).on('error', (e) => {
            reject(e);
        }).end();
    });
}


function compare_assemble_pages(a, b) {
    const ref_compare = a.ref.localeCompare(b.ref);
    return ref_compare
        // same ref => newest first
        || new Date(b.finished_at).getTime() - new Date(a.finished_at).getTime();
}


// function prepare_out_dir(out_dir) {
//     out_dir = path.resolve(__dirname, '../..', out_dir);
//     if (fs.existsSync(out_dir)) {
//         throw new Error(`directory already exists: ${out_dir}`);
//     }
//
//     fs.mkdirSync(out_dir);
//     console.log(`\ncreated output directory: ${out_dir}`);
//     return out_dir;
// }
