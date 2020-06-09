//
// Copyright 2019 Christoph Sprenger
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
const sharp = require('sharp'); // https://github.com/lovell/sharp


/**
 * icon sizes as defined in ngsw-config.json
 */
const icon_sizes = [72, 96, 128, 144, 152, 192, 384, 512];

// icon paths

const svg_path = path.resolve('./src/assets/icons/icon.svg');
if (!fs.existsSync(svg_path)) {
    throw new Error(`${svg_path} does not exist`);
}

function png_path(size) {
    return path.resolve(`./src/assets/icons/icon-${size}x${size}.png`);
}


// skip icon generation, if all icons are up to date

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

async function check_metadata(image_path, sharp_instance) {
    const metadata = await sharp_instance.metadata();

    if (metadata.width !== metadata.height) {
        throw new Error(`${image_path}: invalid image size: ${metadata.width} x ${metadata.height}`);
    }
    return metadata;
}


async function main() {
    const svg_buffer = fs.readFileSync(svg_path);
    const svg_size = (await check_metadata(svg_path, sharp(svg_buffer))).width;

    const sharp_default_density = 72; // see https://sharp.dimens.io/en/stable/api-constructor/#sharp

    await Promise.all(
        icon_sizes.map(async (size) => {
            const png_icon_path = png_path(size);
            console.log(`generating ${png_icon_path} ...`);

            //
            // We can adjust the resulting PNG image size only indirectly via non-fractional density parameter,
            // making it prone to rounding errors.
            // Thus the resulting PNG image size does not always match the desired size,
            // e.g. instead of a 32x32 PNG sharp would render a 31x31 PNG.
            //
            // Resizing the PNG by +1/-1 pixel produces a less "clean" image than
            // rendering the SVG to a PNG of the correct size.
            //
            // We thus render the SVG twice, once using Math.floor(density) and once using Math.ceil(density)
            // and use the result matching the desired image size.
            //

            const density = sharp_default_density * size / svg_size;
            const density_floor = Math.floor(density);
            const density_ceil = Math.ceil(density);

            const svg_floor = await sharp(svg_buffer, {density: density_floor});
            const svg_ceil = await sharp(svg_buffer, {density: density_ceil});

            const width_floor = (await check_metadata(png_icon_path, svg_floor)).width;
            const width_ceil = (await check_metadata(png_icon_path, svg_ceil)).width;

            // use the matching result
            let svg;
            if (width_floor === size) {
                svg = svg_floor;
            }
            if (width_ceil === size) {
                svg = svg_ceil;
            }
            if (!svg) {
                throw new Error(`${png_icon_path}: conversion failed, size ${size} not available`);
            }

            // generate PNG
            await svg.png().toFile(png_icon_path)
        })
    );
}

main()
    .then(() => console.log('done'))
    .catch(err => {
        console.error('ERROR', err);
        process.exit(1);
    });
