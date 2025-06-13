#pragma once

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

// CUDA kernels declarations
__global__ void forward_propagation_kernel(
    const float* x,
    const float* W1, const float* b1,
    const float* W2, const float* b2,
    float* z1, float* a1,
    float* z2, float* a2,
    bool use_relu
);

__global__ void backward_propagation_kernel(
    const float* x,
    const float* y,
    const float* W1, const float* W2,
    const float* z1, const float* a1,
    float z2, float a2,
    float* dW1, float* db1,
    float* dW2, float* db2,
    bool use_relu
);

// GPU wrapper functions for Python
py::array_t<float> forward_propagation_gpu(
    py::array_t<float> x_input,
    py::array_t<float> W1_input,
    py::array_t<float> b1_input,
    py::array_t<float> W2_input,
    py::array_t<float> b2_input,
    bool use_relu
);

py::dict backward_propagation_gpu(
    py::array_t<float> x_input,
    py::array_t<float> y_input,
    py::array_t<float> W1_input,
    py::array_t<float> W2_input,
    py::array_t<float> z1_input,
    py::array_t<float> a1_input,
    float z2,
    float a2,
    bool use_relu
);
