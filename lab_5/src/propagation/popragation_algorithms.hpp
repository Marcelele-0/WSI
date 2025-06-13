#pragma once

#include <pybind11/numpy.h>
#include <string>
#include <cmath>

// Inline activation functions (available in both files)
__host__ __device__ inline float sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}

__host__ __device__ inline float sigmoid_derivative(float x) {
    return x * (1.0f - x);
}

__host__ __device__ inline float relu(float x) {
    return x > 0.0f ? x : 0.0f;
}

__host__ __device__ inline float relu_derivative(float x) {
    return x > 0.0f ? 1.0f : 0.0f;
}

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