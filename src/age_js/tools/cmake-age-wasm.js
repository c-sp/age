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

//
// use this file to build the AGE wasm binary
// and it's "glue code"
//

const child_process = require('child_process');
const path = require('path');
const fs = require('fs');



// path to Emscripten.cmake

const emscripten = process.env.EMSCRIPTEN;
if (!emscripten) {
    throw new Error('EMSCRIPTEN not set, did you source emsdk_env.sh?');
}

const toolchain_file_path = path.resolve(emscripten, 'cmake', 'Modules', 'Platform', 'Emscripten.cmake');

if (!fs.existsSync(toolchain_file_path)) {
    throw new Error('Emscripten.cmake not found: ' + toolchain_file_path);
}
console.log('found Emscripten.cmake at: ' + toolchain_file_path);


// path to CMakeLists.txt

const cmake_file_path = path.resolve(__dirname, '..', '..', '..', 'build', 'wasm', 'CMakeLists.txt');

if (!fs.existsSync(cmake_file_path)) {
    throw new Error('CMakeLists.txt not found: ' + cmake_file_path);
}
console.log('found CMakeLists.txt at: ' + cmake_file_path);


// path to the cmake build-tree

const build_path = path.resolve(__dirname, '..', '..', '..', 'build', 'artifacts', 'wasm');
console.log('building age-wasm in: ' + build_path);

if (!fs.existsSync(build_path)) {
    const artifacts_path = path.dirname(build_path);
    if (!fs.existsSync(artifacts_path)) {
        fs.mkdirSync(artifacts_path); // create 'artifacts' directory first
    }
    fs.mkdirSync(build_path);
}


// run cmake && make

const build_type = process.argv.some(arg => arg.toLowerCase() === 'debug') ? 'Debug' : 'Release';

const cmd = 'cmake -G "Unix Makefiles"'
    + ' -DCMAKE_BUILD_TYPE=' + build_type
    + ' -DCMAKE_TOOLCHAIN_FILE=' + toolchain_file_path
    + ' ' + path.dirname(cmake_file_path);

const cmake_out = child_process.execSync(cmd, {cwd: build_path}).toString();
console.log(cmake_out);

const make_out = child_process.execSync('make', {cwd: build_path}).toString();
console.log(make_out);


// copy build artifacts

const asset_path = path.resolve(__dirname, '..', 'src', 'assets');

const src_age_wasm_js = path.resolve(build_path, 'age_wasm.js');
const dst_age_wasm_js = path.resolve(asset_path, path.basename(src_age_wasm_js));

console.log(`copying ${src_age_wasm_js} to ${dst_age_wasm_js}`);
fs.copyFileSync(src_age_wasm_js, dst_age_wasm_js);

const src_age_wasm_wasm = path.resolve(build_path, 'age_wasm.wasm');
const dst_age_wasm_wasm = path.resolve(asset_path, path.basename(src_age_wasm_wasm));

console.log(`copying ${src_age_wasm_wasm} to ${dst_age_wasm_wasm}`);
fs.copyFileSync(src_age_wasm_wasm, dst_age_wasm_wasm);
