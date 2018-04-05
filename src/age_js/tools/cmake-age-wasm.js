const child_process = require('child_process');
const path = require('path');
const fs = require('fs');


// path to Emscripten.cmake

const emscripten = process.env.EMSCRIPTEN;
if (!emscripten) {
    throw new Error('EMSCRIPTEN not set, did you call emsdk_env.sh?');
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

const cmd = 'cmake -G "Unix Makefiles"'
    + ' -DCMAKE_BUILD_TYPE=Release'
    + ' -DCMAKE_TOOLCHAIN_FILE=' + toolchain_file_path
    + ' ' + path.dirname(cmake_file_path);

child_process.execSync(cmd, {cwd: build_path});
child_process.execSync('make', {cwd: build_path});


// copy build artifacts
