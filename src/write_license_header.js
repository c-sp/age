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
const util = require('util');


/**
 * the license header to put into every file
 */
const license_header = `
//
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
`.trim();

/**
 * extensions of the files that should include a license header
 */
const file_extensions = ['.hpp', '.cpp', '.ts', '.js'];

/**
 * names of the files to ignore
 */
const ignore_files = [
    'karma.conf.js',
    'environment.prod.ts',
    'environment.ts',
    'polyfills.ts'
];


(async function main() {

    // get a list of all files in AGE/src that should contain a license header
    // except for the ignored ones
    const files = await get_files_to_check();

    // display the files that are about to be changed,
    // let the user confirm any updates and terminate if there are no updates
    const update = await confirm_updates(files);
    if (!update) {
        console.log('terminating without updating any license header');

    } else {
        // update license header
        await update_headers(files);
    }
})()
    .then(() => process.exit(0))
    .catch(err => {
        console.error('ERROR', err);
        process.exit(1);
    });


async function get_files_to_check() {

    async function read_license_header(file) {
        return new Promise((resolve, reject) => {

            const readlineInterface = readline.createInterface({
                input: fs.createReadStream(file),
                crlfDelay: Infinity
            });
            let header = '';
            let closed = false; // used to ignore lines that were already buffered when closing the interface

            readlineInterface
                .on('line', line => {
                    if (!closed) {
                        if (!line.startsWith('//')) {
                            readlineInterface.close();
                            closed = true;
                        } else {
                            header += (header ? '\n' : '') + line;
                        }
                    }
                })
                .on('close', () => resolve(header))
                .on('error', err => reject(err));
        });
    }

    const git_ls_files = child_process.execSync('git ls-files -co --exclude-standard', {cwd: __dirname});
    const ls_files = git_ls_files.toString().trim().split('\n').sort();

    return await Promise.all(ls_files.map(async (ls_file) => {
        const file = path.resolve(__dirname, ls_file)
        const ignore = (file_extensions.indexOf(path.extname(file)) < 0)
            || (ignore_files.indexOf(path.basename(file)) >= 0);
        const header = await read_license_header(file);
        const update = !ignore && (header.trim() !== license_header);
        return {
            file: file,
            ignore: ignore,
            update: update,
        };
    }));
}


async function confirm_updates(files) {

    files.forEach(file => {
        const color_reset = '\x1b[0m';
        const color_green = '\x1b[32m';
        const color_yellow = '\x1b[33m';
        const color_white = '\x1b[37m';

        function status_string(color, label) {
            const lbl = `[${label}]`;
            return `${color}${lbl.padEnd(10)}${color_reset}`;
        }

        const status = file.ignore
            ? status_string(color_white, 'ignore')
            : (file.update
                ? status_string(color_yellow, 'update')
                : status_string(color_green, 'valid'));

        console.log(`${status}${file.file}`);
    });

    const num_updates = files
        .map(file => file.update)
        .reduce((updates, update) => updates + update, 0);

    if (num_updates === 0) {
        console.log('\nall license headers are up to date');
        return false;

    } else {
        return new Promise((resolve, reject) => {
            readline.createInterface({
                input: process.stdin,
                output: process.stdout
            }).question(
                `\nupdate license header of ${num_updates} file(s)? [y/N]\n`,
                answer => resolve((answer === 'y') || (answer === 'Y'))
            );
        });
    }
}


async function update_headers(files) {

    const write_file = util.promisify(fs.writeFile);
    const open_file = util.promisify(fs.open);
    const copy_file = util.promisify(fs.copyFile);
    const unlink_file = util.promisify(fs.unlink);

    async function append_file_contents(dst_file, src_file) {
        const fd = await open_file(dst_file, 'a');

        return new Promise((resolve, reject) => {
            let reading_old_header = true;
            let closed = false;

            function close_and_do(action, param) {
                if (!closed) {
                    fs.close(fd, err => err);
                    closed = true;
                    action(param);
                }
            }

            function append_line(line) {
                reading_old_header = reading_old_header && line.startsWith('//');
                if (!closed && !reading_old_header) {
                    try {
                        fs.appendFileSync(fd, line + '\n', 'utf8');
                    } catch (err) {
                        close_and_do(reject, err);
                    }
                }
            }

            readline.createInterface({
                input: fs.createReadStream(src_file),
                crlfDelay: Infinity
            })
                .on('line', line => append_line(line))
                .on('close', () => close_and_do(resolve))
                .on('error', err => close_and_do(reject, err));
        });
    }

    const prefix = `lh_${Math.random()}_`;
    let file_counter = 0;
    const files_to_update = files
        .filter(file => file.update)
        .map(file => file.file);

    console.log(`\nupdating ${files_to_update.length} file(s) ...\n`);
    await Promise.all(files_to_update.map(async file => {
        const tmp_file = path.resolve(os.tmpdir(), prefix + (++file_counter));

        // write license header to temporary file
        await write_file(tmp_file, license_header + '\n');

        // append file contents without the old license header
        await append_file_contents(tmp_file, file);

        // replace file with temporary file
        await copy_file(tmp_file, file);
        await unlink_file(tmp_file);

        console.log(`updated ${file}`);
    }));
}
