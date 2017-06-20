add_library(wplite wiringPi_i2c_lite.cpp)

# Make sure the compiler can find include files for our car library
# when other libraries or executables link to Hello
target_include_directories (wplite PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})