#include "popragation_algorithms.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <cmath>

namespace py = pybind11;

/**
 * @brief Sigmoid activation function
 * @param x Input value
 * @return Sigmoid of x
 */
__host__ __device__ float sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));  // Use expf instead of exp for GPU compatibility
}

/**
 * @brief Sigmoid derivative function
 * @param x Input value
 * @return Derivative of sigmoid at x
 */
__host__ __device__ float sigmoid_derivative(float x) {
    return x * (1.0f - x);
}

/**
 * @brief ReLU activation function
 * @param x Input value
 * @return ReLU of x
 */
__host__ __device__ float relu(float x) {
    return x > 0.0f ? x : 0.0f;
}

/**
 * @brief ReLU derivative function
 * @param x Input value
 * @return Derivative of ReLU at x
 */
__host__ __device__ float relu_derivative(float x) {
    return x > 0.0f ? 1.0f : 0.0f;
}


/**
 * @brief Forward propagation through a simple 2-layer neural network
 * @param x Input vector (2 elements)
 * @param W1 Weights for the first layer (2x4)
 * @param b1 Biases for the first layer (4)
 * @param W2 Weights for the second layer (4x1)
 * @param b2 Biases for the second layer (1)
 * @param z1 Output of the first layer before activation (4)
 * @param a1 Output of the first layer after activation (4)
 * @param z2 Output of the second layer before activation (1)
 * @param a2 Output of the second layer after activation (1)
 * @param use_relu Whether to use ReLU activation in the hidden layer
 */
__host__ __device__
void forward_propagation(
    const float* x,                     // input 2-elementowy
    const float* W1, const float* b1,   // W1: 2x4, b1: 4
    const float* W2, const float* b2,   // W2: 4x1, b2: 1
    float* z1, float* a1,               // z1: 4, a1: 4
    float& z2, float& a2,               // z2, a2: 1
    bool use_relu = false               // wybór aktywacji
) {
    // Warstwa ukryta
    for (int i = 0; i < 4; ++i) {
        z1[i] = b1[i];
        for (int j = 0; j < 2; ++j) {
            z1[i] += x[j] * W1[j * 4 + i];  // W1: [2][4]
        }
        a1[i] = use_relu ? relu(z1[i]) : sigmoid(z1[i]);
    }

    // Warstwa wyjściowa
    z2 = b2[0];
    for (int i = 0; i < 4; ++i) {
        z2 += a1[i] * W2[i];  // W2: [4][1]
    }
    a2 = sigmoid(z2);  // Na wyjściu zawsze sigmoid
}

/**
 * @brief Backward propagation to compute gradients for weights and biases
 * @param x Input vector (2 elements)
 * @param y Target output (1 element)
 * @param W1 Weights for the first layer (2x4)
 * @param W2 Weights for the second layer (4x1)
 * @param z1 Output of the first layer before activation (4)
 * @param a1 Output of the first layer after activation (4)
 * @param z2 Output of the second layer before activation (1)
 * @param a2 Output of the second layer after activation (1)
 * @param dW1 Gradient for W1 (2x4)
 * @param db1 Gradient for b1 (4)
 * @param dW2 Gradient for W2 (4x1)
 * @param db2 Gradient for b2 (1)
 * @param use_relu Whether to use ReLU activation in the hidden layer
 */
__host__ __device__
void backward_propagation(
    const float* x,                      // input 2-elementowy
    const float* y,                      // target (1-elementowy)
    const float* W1, const float* W2,    // wagi
    const float* z1, const float* a1,    // z i a z forwarda
    float z2, float a2,                  // z2 i a2 z forwarda
    float* dW1, float* db1,              // gradienty
    float* dW2, float* db2,
    bool use_relu = false
) {
    // Warstwa wyjściowa
    float dz2 = a2 - y[0];
    for (int i = 0; i < 4; ++i) {
        dW2[i] = a1[i] * dz2;
    }
    db2[0] = dz2;

    // Warstwa ukryta
    for (int i = 0; i < 4; ++i) {
        float da1 = W2[i] * dz2;
        float dz1 = da1 * (use_relu ? relu_derivative(z1[i]) : sigmoid_derivative(a1[i]));

        db1[i] = dz1;

        for (int j = 0; j < 2; ++j) {
            dW1[j * 4 + i] = x[j] * dz1;  // W1: [2][4]
        }
    }
}

// Pybind11 module definition
PYBIND11_MODULE(cuda_nn, m) {
    m.doc() = "CUDA Neural Network Propagation Module";
    
    // Expose activation functions
    m.def("sigmoid", &sigmoid, "Sigmoid activation function");
    m.def("sigmoid_derivative", &sigmoid_derivative, "Sigmoid derivative");
    m.def("relu", &relu, "ReLU activation function");
    m.def("relu_derivative", &relu_derivative, "ReLU derivative");
    
    // Expose main functions (you'll need to implement propagate and predict)
    // m.def("propagate", &propagate, "Forward and backward propagation");
    // m.def("predict", &predict, "Prediction function");
}
