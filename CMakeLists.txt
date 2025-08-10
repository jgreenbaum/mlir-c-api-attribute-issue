cmake_minimum_required(VERSION 3.13)
project(MlirCPlayground LANGUAGES C CXX)

# Pass -DMLIR_INSTALL_DIR=<path to where MLIR is installed>
list(APPEND CMAKE_PREFIX_PATH "${MLIR_INSTALL_DIR}/lib/cmake")

# Find MLIR and LLVM
find_package(MLIR REQUIRED CONFIG)
find_package(LLVM REQUIRED CONFIG)
find_package(CIRCT REQUIRED CONFIG)
# find_package(MLIR_C REQUIRED CONFIG)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include MLIR and LLVM headers
include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CIRCT_INCLUDE_DIRS})
#include_directories(${MLIR_C_INCLUDE_DIRS})

# Link MLIR and LLVM libraries
link_directories(${MLIR_LIBRARY_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
link_directories(${CIRCT_LIBRARY_DIRS})

add_executable(ifdef     
    ifdef.c
)
target_link_libraries(ifdef 
    PRIVATE

    CIRCTCAPIHW
    CIRCTCAPISV
)
