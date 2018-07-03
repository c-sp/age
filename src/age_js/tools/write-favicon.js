const fs = require('fs');
const path = require('path');
const os = require('os');
const icon_gen = require('icon-gen');

// check file modification time
// (skip icon generation, if it's up to date)
const path_svg = path.resolve('./src/favicon.svg');
const path_ico = path.resolve('./src/favicon.ico');

if (!fs.existsSync(path_svg)) {
    throw new Error(`${path_svg} does not exist`);
}

let generate = true;
if (fs.existsSync(path_ico)) {
    const stat_svg = fs.statSync(path_svg);
    const stat_ico = fs.statSync(path_ico);

    if (stat_svg.mtime < stat_ico.mtime) {
        console.log(`${path_ico} is up to date, will not generate it from svg`);
        process.exit();
    }
}

// generate icon from svg
const options = {
    modes: ['favicon'],
    report: true
};

const tmp = fs.mkdtempSync(path.resolve(os.tmpdir(), 'age-favicon'));

icon_gen(path_svg, tmp, options)
    .then((results) => {
        console.log(results);
        fs.copyFileSync(path.resolve(tmp, 'favicon.ico'), path_ico);
    })
    .catch((err) => {
        console.error(err);
    });
