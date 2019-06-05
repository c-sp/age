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

// TODO adjust to observe TSLint rules
const file_path = path.resolve(__dirname, '..', 'src', 'environments', 'version.ts');
fs.writeFileSync(file_path, `
// THIS FILE IS AUTO GENERATED, DON'T EDIT IT MANUALLY!
/* tslint:disable */
export const VERSION_INFO = ${JSON.stringify(version_info, null, 4)};
/* tslint:enable */
`);
