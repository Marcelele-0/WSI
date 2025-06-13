#pragma once

#include <pybind11/numpy.h>
#include <string>

// Forward declarations of activation functions
__host__ __device__ float sigmoid(float x);
__host__ __device__ float sigmoid_derivative(float x);
__host__ __device__ float relu(float x);
__host__ __device__ float relu_derivative(float x);

// Main functions
float propagate(
    pybind11::array_t<float> X,
    pybind11::array_t<float> y,
    pybind11::array_t<float> W1,
    pybind11::array_t<float> b1,
    pybind11::array_t<float> W2,
    pybind11::array_t<float> b2,
    float learning_rate,
    const std::string& activation_function
);

pybind11::array_t<float> predict(
    pybind11::array_t<float> X,
    pybind11::array_t<float> W1,
    pybind11::array_t<float> b1,
    pybind11::array_t<float> W2,
    pybind11::array_t<float> b2,
    const std::string& activation_function
);