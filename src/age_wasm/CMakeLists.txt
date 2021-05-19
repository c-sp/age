#
# Clion & Emscripten:
# https://tech-en.netlify.app/articles/en522284/index.html
#

add_executable(
        age_wasm
        age_wasm.cpp
)

target_link_libraries(age_wasm age_emulator_gb)

# MODULARIZE=1     to prevent using the global scope
# EXPORT_ES6=1     to generate an ES6 module export
# ENVIRONMENT=web  to omit node/shell/web-worker code
#
set(WASM_LINK_FLAGS "\
-s ALLOW_MEMORY_GROWTH=1 \
-s FILESYSTEM=0 \
-s MODULARIZE=1 \
-s EXPORT_ES6=1 \
-s ENVIRONMENT=web \
")
set_target_properties(age_wasm PROPERTIES LINK_FLAGS ${WASM_LINK_FLAGS})
