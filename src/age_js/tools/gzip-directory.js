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

const path = require('path');
const fs = require('fs');
const zlib = require('zlib');

// evaluate parameter, use the Angular CLI dist directory as default
let dir_path = path.resolve(__dirname, '../dist/age-js');
if (process.argv.length > 2) {
    dir_path = path.resolve(process.argv[2]);
}

// make sure the directory exists
if (!fs.existsSync(dir_path)) {
    throw new Error(`${dir_path} does not exist`);
}

// find all files to gzip
function findFiles(file) {
    return fs.statSync(file).isFile()
        ? (file.endsWith('gz') ? [] : [file])
        : fs.readdirSync(file).reduce(
            (prev, cur) => prev.concat(...findFiles(path.resolve(file, cur))),
            []
        );
}
const files = findFiles(dir_path);

// gzip all files
files.forEach(file => {
    // based on https://www.w3schools.com/nodejs/ref_zlib.asp
    var gzip = zlib.createGzip();
    var r = fs.createReadStream(file);
    var w = fs.createWriteStream(`${file}.gz`);
    r.pipe(gzip).pipe(w);
});
