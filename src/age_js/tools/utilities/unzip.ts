import {mkdir, writeFile} from "fs";
import * as JSZip from "jszip";
import {combineLatest, from, Observable} from "rxjs";
import {switchMap} from "rxjs/operators";
import {promisify} from "util";


export function unzipArchive$(archiveData: ArrayBuffer, destPath: string): Observable<void[]> {
    return from(JSZip.loadAsync(archiveData)).pipe(
        switchMap(zipArchive => {
            return combineLatest(
                Object.keys(zipArchive.files).map(fileName => unzipFile$(zipArchive, fileName, destPath)),
            );
        }),
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
