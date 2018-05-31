//
// This is a utility script for adding a license header to all AGE source files.
// Run with:
//      node write_license_header.js
//

const child_process = require('child_process');
const path = require('path');
const fs = require('fs');
const readline = require('readline');
const os = require('os');


const license_header =
    `//
// Copyright ${(new Date()).getFullYear()} Christoph Sprenger
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
`;


(async function main() {

    // get a list of all files in AGE/src except for the ignored ones
    const git_ls_files = child_process.execSync('git ls-files', {cwd: __dirname});

    // filter files that shall contain the license header
    const files = await filter_git_ls_files(git_ls_files);

    // display the files that are about to be changed
    display_files(files);

    // let the user confirm any updates and terminate if there are no updates
    const update = await confirm_updates(files);
    if (!update) {
        console.log('terminating without updating any license header');
        process.exit();
    }

    // update license header
    await update_headers(files);

    process.exit();
})();


async function filter_git_ls_files(git_ls_files) {

    async function read_license_header(file) {
        return new Promise(function (resolve, reject) {

            const readlineInterface = readline.createInterface({
                input: fs.createReadStream(file),
                crlfDelay: Infinity
            });
            let header = '';
            let line_count = 0;
            let closed = false; // used to ignore lines that are already buffered when closing the interface

            readlineInterface
                .on('line', line => {
                    if (!closed) {
                        if (!line.startsWith('//')) {
                            readlineInterface.close();
                            closed = true;
                        } else {
                            header += (header ? '\n' : '') + line;
                            ++line_count;
                        }
                    }
                })
                .on('close', () => resolve({header, line_count}))
                .on('error', err => reject(err));
        });
    }

    const ls_files = git_ls_files.toString().trim().split('\n');

    return await Promise.all(ls_files.map(async (ls_file) => {
        const file = path.resolve(__dirname, ls_file)
        const ignore = ['.hpp', '.cpp', '.ts', '.js'].indexOf(path.extname(file)) < 0;
        const header_info = await read_license_header(file);
        const update = !ignore && (header_info.header !== license_header);
        return {
            file: file,
            ignore: ignore,
            update: update,
            header_line_count: header_info.line_count
        };
    }));
}


function display_files(files) {

    const color_reset = '\x1b[0m';
    const color_green = '\x1b[32m';
    const color_yellow = '\x1b[33m';
    const color_white = '\x1b[37m';

    function status_string(color, label) {
        const lbl = `[${label}]`;
        return `${color}${lbl.padEnd(10)}${color_reset}`;
    }

    files.forEach(file => {
        const status = file.ignore
            ? status_string(color_white, 'ignore')
            : (file.update
                ? status_string(color_yellow, 'update')
                : status_string(color_green, 'valid'));

        console.log(`${status}${file.file}`);
    });
}


async function confirm_updates(files) {
    return new Promise(function (resolve, reject) {

        const num_updates = files
            .map(file => file.update)
            .reduce((updates, update) => updates + update, 0);

        if (num_updates === 0) {
            console.log('\nall license headers are up to date');
            resolve(false);

        } else {
            const readlineInterface = readline.createInterface({
                input: process.stdin,
                output: process.stdout
            });

            readlineInterface.question(`\nupdate the license header of ${num_updates} file(s)? [y/N]\n`, function (answer) {
                resolve((answer === 'y') || (answer === 'Y'));
            });
        }
    });
}


async function update_headers(files) {

    function handle(resolve, reject, err) {
        if (err) {
            reject(err);
        } else {
            resolve();
        }
    }

    async function writeFile(file, data) {
        return new Promise(function (resolve, reject) {
            fs.writeFile(file, data, err => handle(resolve, reject, err));
        });
    }

    async function renameFile(old_name, new_name) {
        return new Promise(function (resolve, reject) {
            fs.rename(old_name, new_name, err => handle(resolve, reject, err));
        });
    }

    const files_to_update = files.filter(file => file.update).slice(0, 1); // TODO REMOVE SLICE!
    console.log(`\nupdating ${files_to_update.length} file(s) ...\n`);

    const prefix = `lic_head_${Math.random()}_`;
    let file_counter = 0;

    await Promise.all(files_to_update.map(async file => {
        // write license header to temporary file
        const tmp_file = path.resolve(os.tmpdir(), prefix + (++file_counter));
        await writeFile(tmp_file, license_header);

        // TODO append file contents without the old license header
        console.log(tmp_file);

        // replace file with temporary file
        // TODO await renameFile(tmp_file, file.file);
    }));
}
