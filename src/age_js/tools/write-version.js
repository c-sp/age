const child_process = require('child_process');
const path = require('path');
const fs = require('fs');

const git_log = child_process.execSync('git log -n1 --pretty=format:"%ad  |  %an  |  %H" --date=iso8601').toString();
const parts = git_log.split('  |  ');

const version_info = {
    date: new Date(Date.parse(parts[0])),
    author: parts[1],
    hash: parts[2]
};

const file = path.resolve(__dirname, '..', 'src', 'environments', 'version.ts');
fs.writeFileSync(file, `
// THIS FILE IS AUTO GENERATED, DON'T EDIT IT MANUALLY!
/* tslint:disable */
export const VERSION_INFO = ${JSON.stringify(version_info, null, 4)};
/* tslint:enable */
`);
