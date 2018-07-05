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

const fs = require('fs');
const path = require('path');
const util = require('util');
const svg2png = require('svg2png'); // https://github.com/domenic/svg2png


// icon sizes based on:
// https://sympli.io/blog/2017/02/15/heres-everything-you-need-to-know-about-favicons-in-2017/
// https://developers.google.com/web/fundamentals/web-app-manifest/

const icon_sizes = [
    16, 32, 96,         // browser/taskbar/desktop icon
    120, 152, 167, 180, // apple icons
    192, 512            // Android (scaled by Chrome)
];

// icon paths

const svg_path = path.resolve('./src/assets/icon.svg');
if (!fs.existsSync(svg_path)) {
    throw new Error(`${svg_path} does not exist`);
}

function png_path(size) {
    return path.resolve(`./src/assets/icon-${size}x${size}.png`);
}


// skip icon generation, if files are up to date

const svg_mtime = fs.statSync(svg_path).mtime;

const up_to_date = icon_sizes.every(size => {
    const path = png_path(size);
    return fs.existsSync(path) && (fs.statSync(path).mtime >= svg_mtime);
});

if (up_to_date) {
    console.log('icons are up to date');
    process.exit();
}


// generate icons

const write_file = util.promisify(fs.writeFile);

const svg_buffer = fs.readFileSync(svg_path);

Promise.all(
    icon_sizes.map(async (size) => {
        const icon_path = png_path(size);
        console.log(`generating ${icon_path} ...`);

        const icon_buffer = await svg2png(svg_buffer, {width: size, height: size});
        await write_file(icon_path, icon_buffer);
    })
)
    .then(() => console.log('done'))
    .catch(err => {
        console.error('ERROR', err);
        process.exit(1);
    });
