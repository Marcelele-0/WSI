cmake_minimum_required(VERSION 3.18)
project(cuda_nn_project LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development NumPy)
find_package(pybind11 REQUIRED)

include_directories(${Python3_INCLUDE_DIRS})
include_directories(${Python3_NumPy_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Create the pybind11 module (now files are in current directory)
pybind11_add_module(cuda_nn 
    popragation_algorithms.cu 
    propagation_algorithms_gpu.cu
)
target_link_libraries(cuda_nn PRIVATE ${Python3_LIBRARIES})
