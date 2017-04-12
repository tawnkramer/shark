
add_library(mxnet mxnet_predict-all.cc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp -fpermissive -mfpu=neon-vfpv4 -funsafe-math-optimizations -ftree-vectorize -fomit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fopenmp")

# Make sure the compiler can find include files for our library
# when other libraries or executables link to us
target_include_directories (mxnet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
