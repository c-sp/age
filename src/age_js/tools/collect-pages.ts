import {argv} from "process";
import {combineLatest, Observable, of} from "rxjs";
import {catchError, map, switchMap, tap} from "rxjs/operators";
import {httpRequest$, IHttpsResponse} from "./utilities/https";

/* tslint:disable:no-any */


const args = argv.slice();
console.log("###", args);

main$().pipe(
    map(() => 0),
    catchError(err => {
        console.error("ERROR", err);
        return of(1);
    }),
).subscribe(exitCode => process.exit(exitCode));


function main$(): Observable<any> {
    return combineLatest([
        // get current branches
        gitlabApiJson$("GET", "/repository/branches?per_page=100").pipe(
            map<any[], Map<string, any>>(branchesJson => {
                const latestBranchJobMap = new Map<string, any>();
                branchesJson
                    .map(branchJson => branchJson.name)
                    .forEach(branchName => latestBranchJobMap.set(branchName, {}));
                return latestBranchJobMap;
            }),
        ),
        // get all jobs
        getCiJobs$(),
    ]).pipe(
        map(values => {
            const branchesMap = values[0];

            const assemblePagesJobName = "assemble-pages";
            const jobs = values[1];
            const artifactJobs = jobs
                .filter(job => job.name === assemblePagesJobName)
                .filter(job => !!job.artifacts_file)
                .sort(compareAssemblePagesJobs); // sort first by ref, second by finished_at

            let foundJobsToDelete = false;
            artifactJobs.forEach(job => {
                // branch still exists and it's the latest job => use this artifact
                if (branchesMap.get(job.ref) && !branchesMap.get(job.ref).id) {
                    branchesMap.set(job.ref, job);
                } else {
                    foundJobsToDelete = job.__del__ = true;
                }
            });

            console.log(`\nfound ${branchesMap.size} active branch(es):`);
            console.log(`    ${[...branchesMap.keys()].sort().join("\n    ")}`);
            console.log(`\nfound ${jobs.length} AGE CI jobs`);
            console.log(`found ${artifactJobs.length} '${assemblePagesJobName}' jobs with artifacts:`);
            artifactJobs.forEach(
                job => console.log(`    ${job.finished_at}  ${job.__del__ ? "D" : " "} ${job.ref}  (id ${job.id})`),
            );

            const jobsToKeep = artifactJobs.filter(job => !job.__del__);
            const jobsToDelete = artifactJobs.filter(job => !!job.__del__);
            if (jobsToKeep.length < 1) {
                throw new Error("found no artifacts to depoy"); // should never happen, but just in case ...
            }

            return {jobsToKeep, jobsToDelete};
        }),

        // delete artifacts we don't need anymore
        switchMap(jobs => {
            if (jobs.jobsToDelete.length) {
                console.log(`\ndeleting old artifacts ...`);
            }
            return combineLatest([
                of(true),
                ...jobs.jobsToDelete.map(
                    job => gitlabApi$("DELETE", `/jobs/${job.id}/artifacts`).pipe(
                        tap(() => console.log(`    ${job.finished_at}    ${job.ref}  (id ${job.id})`)),
                    ),
                ),
            ]).pipe(
                map(() => jobs.jobsToKeep),
            );
        }),

        // download artifacts to deploy
        switchMap(jobsToKeep => combineLatest(
            jobsToKeep.map(job => gitlabApiBinary$("GET", `/jobs/${job.id}/artifacts`)),
        )),

        // TODO assemble final GitLab pages
        map(artifacts => {
            console.log("artifacts", artifacts);
        }),

        // TODO adjust base-href
    );
}


function getCiJobs$(): Observable<any[]> {
    const jobs = new Array<any>();
    return _getCiJobs(1);

    function _getCiJobs(startPage: number): Observable<any[]> {
        return combineLatest(
            new Array(10)
                .fill(0)
                .map(() => gitlabApiJson$("GET", `/jobs?per_page=100&page=${startPage++}`)),
        ).pipe(
            switchMap(values => {
                let finished = false;
                values.forEach(jobsList => {
                    finished = finished || !jobsList.length;
                    jobs.push(...jobsList);
                });
                return finished ? of(jobs) : _getCiJobs(startPage);
            }),
        );
    }
}


function compareAssemblePagesJobs(a: any, b: any) {
    const refCompare = a.ref.localeCompare(b.ref);
    return refCompare
        // same ref => newest first
        || new Date(b.finished_at).getTime() - new Date(a.finished_at).getTime();
}


function gitlabApiJson$(method: string, path: string): Observable<any> {
    return gitlabApi$(method, path).pipe(
        map(httpsResponse => JSON.parse(httpsResponse.data.toString("utf8"))),
    );
}

function gitlabApiBinary$(method: string, path: string): Observable<Uint8Array> {
    return gitlabApi$(method, path).pipe(
        map(httpsResponse => new Uint8Array(httpsResponse.data.buffer)),
    );
}

function gitlabApi$(method: string, path: string): Observable<IHttpsResponse> {
    const gitlabApiToken = process.env.GITLAB_API_TOKEN;
    if (!gitlabApiToken) {
        throw new Error("GitLab API token not set");
    }
    const options = {
        host: "gitlab.com",
        path: `/api/v4/projects/2686832${path}`, // AGE Project ID: 2686832
        method,
        headers: {
            "Private-Token": gitlabApiToken,
        },
    };
    return httpRequest$(options);
}
