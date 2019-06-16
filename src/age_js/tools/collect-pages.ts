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

import {exec} from "child_process";
import {createReadStream, createWriteStream, existsSync, mkdir, mkdirSync, readFile, writeFile} from "fs";
import * as JSZip from "jszip";
import {resolve} from "path";
import {argv} from "process";
import {combineLatest, from, Observable, of} from "rxjs";
import {catchError, map, switchMap, tap} from "rxjs/operators";
import {promisify} from "util";
import {createGzip} from "zlib";
import {httpRequest$, IHttpsResponse} from "./utilities/https";

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
                .sort(
                    // sort first by ref, second by finished_at
                    (a, b) => a.ref.localeCompare(b.ref)
                        || (new Date(b.finished_at).getTime() - new Date(a.finished_at).getTime()),
                );

            artifactJobs.forEach(job => {
                // branch still exists and it's the latest job => use this artifact
                if (branchesMap.get(job.ref) && !branchesMap.get(job.ref).id) {
                    branchesMap.set(job.ref, job);
                } else {
                    job.__del__ = true;
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

        // copy "master" files up one level
        // (do this before adjusting base-href)
        tap(() => console.log(`copying master files ...`)),
        switchMap(indexHtmlFiles => from(promisify(exec)("cp -r master/* .", {cwd: outputPath})).pipe(
            map(() => indexHtmlFiles),
        )),

        // adjust base-href in non-master index.html files
        tap(() => console.log(`adjusting index.html base-href in subdirectories ...`)),
        // TODO why is indexHtmlFiles inferred as "any"?
        switchMap(indexHtmlFiles => combineLatest((indexHtmlFiles as string[]).map(adjustBaseHref$))),
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


export function unzipArchive$(archiveData: ArrayBuffer, destPath: string): Observable<string> {
    let indexHtmlPath = "";

    return from(JSZip.loadAsync(archiveData)).pipe(
        switchMap(zipArchive => {
            indexHtmlPath = Object.keys(zipArchive.files).find(fileName => fileName.endsWith("index.html")) || "";
            indexHtmlPath = indexHtmlPath ? resolve(destPath, indexHtmlPath) : indexHtmlPath;
            return combineLatest(
                Object.keys(zipArchive.files).map(fileName => unzipFile$(zipArchive, fileName, destPath)),
            );
        }),
        map(() => indexHtmlPath),
    );
}

function unzipFile$(zipArchive: JSZip, fileName: string, destPath: string): Observable<void> {
    // file is a directory => create it
    if (fileName.endsWith("/")) {
        const dirPath = `${destPath}/${fileName}`;
        return from(promisify(mkdir)(dirPath, {recursive: true}));
    }

    // write file
    return from(zipArchive.file(fileName).async("nodebuffer")).pipe(
        switchMap(fileContents => {
            const destFilePath = `${destPath}/${fileName}`;
            return from(promisify(writeFile)(destFilePath, fileContents));
        }),
    );
}


function adjustBaseHref$(filePath: string): Observable<{}> {
    return from(promisify(readFile)(filePath, "utf-8")).pipe(
        switchMap(fileContents => {
            // extract branch name from path
            if (!filePath.startsWith(outputPath)) {
                throw new Error(`invalid index.html path: ${filePath}`);
            }
            const lastSlashIdx = filePath.lastIndexOf("/"); // TODO this might not work for Windows
            const subDirBeginIdx = outputPath.length + 1; // don't include the slash
            const subDir = filePath.substring(subDirBeginIdx, lastSlashIdx);

            // find "base href" element
            const baseHrefBeginIdx = fileContents.indexOf("<base href=\"");
            const baseHrefEndQuoteIdx = fileContents.indexOf("\">", baseHrefBeginIdx);
            if ((baseHrefBeginIdx < 0) || (baseHrefEndQuoteIdx < 0)) {
                throw new Error(`${filePath}: base-href not found`);
            }

            // insert branch name
            const firstPart = fileContents.slice(0, baseHrefEndQuoteIdx);
            const lastPart = fileContents.slice(baseHrefEndQuoteIdx);
            const newFileContents = `${firstPart}${subDir}/${lastPart}`;

            //  write file
            return from(promisify(writeFile)(filePath, newFileContents)).pipe(
                map(() => newFileContents),
            );
        }),

        // gzip new index.html
        switchMap(() => from(new Promise((_resolve, _reject) => {
            const gzip = createGzip();
            const readStream = createReadStream(filePath);
            const writeStream = createWriteStream(`${filePath}.gz`);
            readStream.on("error", _reject);
            writeStream.on("error", _reject);
            writeStream.on("finish", _resolve); // wait until finished
            readStream.pipe(gzip).pipe(writeStream);
        }))),
    );
}
