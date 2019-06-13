//
// Copyright 2018 Christoph Sprenger
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

import {existsSync, mkdirSync} from "fs";
import {resolve} from "path";
import {argv} from "process";
import {combineLatest, Observable, of} from "rxjs";
import {catchError, map, switchMap, tap} from "rxjs/operators";
import {httpRequest$, IHttpsResponse} from "./utilities/https";
import {unzipArchive$} from "./utilities/unzip";

/* tslint:disable:no-any */


if (argv.length < 3) {
    console.error("output path not specified");
    process.exit(1);
}
const outputPath = resolve(argv[2]);
console.log(`\noutput path: "${outputPath}"`);
if (!existsSync(outputPath)) {
    mkdirSync(outputPath, {recursive: true});
}


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

            artifactJobs.forEach(job => {
                // branch still exists and it's the latest job => use this artifact
                if (branchesMap.get(job.ref) && !branchesMap.get(job.ref).id) {
                    branchesMap.set(job.ref, job);
                }
            });

            console.log(`\nfound ${branchesMap.size} active branch(es):`);
            console.log(`    ${[...branchesMap.keys()].sort().join("\n    ")}`);
            console.log(`\nfound ${jobs.length} AGE CI jobs`);
            console.log(`found ${artifactJobs.length} '${assemblePagesJobName}' jobs with artifacts:`);
            artifactJobs.forEach(
                job => console.log(`    ${job.finished_at}  ${job.__del__ ? "D" : " "} (id ${job.id})  ${job.ref}`),
            );

            const jobsToKeep = artifactJobs.filter(job => !job.__del__);
            const jobsToDelete = artifactJobs.filter(job => !!job.__del__);
            if (jobsToKeep.length < 1) {
                throw new Error("found no artifacts to depoy"); // should never happen, but just in case ...
            }

            return {jobsToKeep, jobsToDelete};
        }),

        // delete artifacts we don't need anymore
        tap(jobs => {
            if (jobs.jobsToDelete.length) {
                console.log(`\ndeleting old artifacts ...`);
            }
        }),
        switchMap(jobs => combineLatest([
            of(true), // combineLatest requires at least one observable
            ...jobs.jobsToDelete.map(
                job => gitlabApi$("DELETE", `/jobs/${job.id}/artifacts`).pipe(
                    tap(() => console.log(`    ${job.finished_at}    (id ${job.id})  ${job.ref}`)),
                ),
            ),
        ]).pipe(
            map(() => jobs.jobsToKeep),
        )),

        // download artifacts to deploy
        tap(jobsToKeep => console.log(`\ndownloading ${jobsToKeep.length} artifact archives ...`)),
        switchMap(jobsToKeep => combineLatest(
            jobsToKeep.map(job => gitlabApiBinary$("GET", `/jobs/${job.id}/artifacts`).pipe(
                tap(archive => console.log(
                    `    (id ${job.id})  ${job.ref} finished: ${archive.byteLength} bytes`,
                )),
            )),
        )),

        // extract archives into outputPath
        tap(artifacts => console.log(`\nextracting ${artifacts.length} artifact archives ...`)),
        switchMap(artifacts => combineLatest(
            artifacts.map(artifact => unzipArchive$(artifact, outputPath)),
        )),

        // TODO move "master" files up one level

        // TODO adjust base-href in non-master index.html files
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

function gitlabApiBinary$(method: string, path: string): Observable<ArrayBuffer> {
    return gitlabApi$(method, path).pipe(
        map(httpsResponse => httpsResponse.data.buffer),
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
