add_executable(
        age_cgb_lut
        age_cgb_lut.cpp
)

# we require libpng to read the LUT image
find_package(PNG REQUIRED)

target_link_libraries(age_cgb_lut PRIVATE PNG::PNG age_common)
