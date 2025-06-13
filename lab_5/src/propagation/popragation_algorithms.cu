#include "popragation_algorithms.hpp"
#include "propagation_algorithms_gpu.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;


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

// Wrapper functions for Python interface
py::array_t<float> forward_propagation_py(
    py::array_t<float> x_input,
    py::array_t<float> W1_input,
    py::array_t<float> b1_input,
    py::array_t<float> W2_input,
    py::array_t<float> b2_input,
    bool use_relu = false
) {
    auto x_buf = x_input.request();
    auto W1_buf = W1_input.request();
    auto b1_buf = b1_input.request();
    auto W2_buf = W2_input.request();
    auto b2_buf = b2_input.request();
    
    float* x = static_cast<float*>(x_buf.ptr);
    float* W1 = static_cast<float*>(W1_buf.ptr);
    float* b1 = static_cast<float*>(b1_buf.ptr);
    float* W2 = static_cast<float*>(W2_buf.ptr);
    float* b2 = static_cast<float*>(b2_buf.ptr);
    
    // Allocate outputs
    float z1[4], a1[4];
    float z2, a2;
    
    forward_propagation(x, W1, b1, W2, b2, z1, a1, z2, a2, use_relu);
    
    // Return results as numpy arrays
    auto result = py::array_t<float>(6);  // z1(4) + a1(4) + z2(1) + a2(1) = 10, but we'll return key values
    auto result_buf = result.request();
    float* result_ptr = static_cast<float*>(result_buf.ptr);
    
    // Store a1 (4 elements) and a2 (1 element) and z2 (1 element)
    for (int i = 0; i < 4; i++) {
        result_ptr[i] = a1[i];
    }
    result_ptr[4] = z2;
    result_ptr[5] = a2;
    
    return result;
}

py::dict backward_propagation_py(
    py::array_t<float> x_input,
    py::array_t<float> y_input,
    py::array_t<float> W1_input,
    py::array_t<float> W2_input,
    py::array_t<float> z1_input,
    py::array_t<float> a1_input,
    float z2,
    float a2,
    bool use_relu = false
) {
    auto x_buf = x_input.request();
    auto y_buf = y_input.request();
    auto W1_buf = W1_input.request();
    auto W2_buf = W2_input.request();
    auto z1_buf = z1_input.request();
    auto a1_buf = a1_input.request();
    
    float* x = static_cast<float*>(x_buf.ptr);
    float* y = static_cast<float*>(y_buf.ptr);
    float* W1 = static_cast<float*>(W1_buf.ptr);
    float* W2 = static_cast<float*>(W2_buf.ptr);
    float* z1 = static_cast<float*>(z1_buf.ptr);
    float* a1 = static_cast<float*>(a1_buf.ptr);
    
    // Allocate gradient arrays
    float dW1[8], db1[4], dW2[4], db2[1];  // dW1: 2x4=8
    
    backward_propagation(x, y, W1, W2, z1, a1, z2, a2, dW1, db1, dW2, db2, use_relu);
    
    // Create numpy arrays for gradients
    auto dW1_array = py::array_t<float>({2, 4});
    auto db1_array = py::array_t<float>(4);
    auto dW2_array = py::array_t<float>(4);
    auto db2_array = py::array_t<float>(1);
    
    auto dW1_buf = dW1_array.request();
    auto db1_buf = db1_array.request();
    auto dW2_buf = dW2_array.request();
    auto db2_buf = db2_array.request();
    
    float* dW1_ptr = static_cast<float*>(dW1_buf.ptr);
    float* db1_ptr = static_cast<float*>(db1_buf.ptr);
    float* dW2_ptr = static_cast<float*>(dW2_buf.ptr);
    float* db2_ptr = static_cast<float*>(db2_buf.ptr);
    
    // Copy gradients
    for (int i = 0; i < 8; i++) dW1_ptr[i] = dW1[i];
    for (int i = 0; i < 4; i++) db1_ptr[i] = db1[i];
    for (int i = 0; i < 4; i++) dW2_ptr[i] = dW2[i];
    db2_ptr[0] = db2[0];
    
    py::dict gradients;
    gradients["dW1"] = dW1_array;
    gradients["db1"] = db1_array;
    gradients["dW2"] = dW2_array;
    gradients["db2"] = db2_array;
    
    return gradients;
}

// Pybind11 module definition
PYBIND11_MODULE(cuda_nn, m) {
    m.doc() = "CUDA Neural Network Propagation Module";
    
    // Expose activation functions
    m.def("sigmoid", &sigmoid, "Sigmoid activation function");
    m.def("sigmoid_derivative", &sigmoid_derivative, "Sigmoid derivative");
    m.def("relu", &relu, "ReLU activation function");
    m.def("relu_derivative", &relu_derivative, "ReLU derivative");
    
    // Expose propagation functions (CPU)
    m.def("forward_propagation", &forward_propagation_py, 
          "Forward propagation through 2-layer neural network (CPU)",
          py::arg("x"), py::arg("W1"), py::arg("b1"), py::arg("W2"), py::arg("b2"), py::arg("use_relu") = false);
    
    m.def("backward_propagation", &backward_propagation_py,
          "Backward propagation to compute gradients (CPU)",
          py::arg("x"), py::arg("y"), py::arg("W1"), py::arg("W2"), 
          py::arg("z1"), py::arg("a1"), py::arg("z2"), py::arg("a2"), py::arg("use_relu") = false);
    
    // Expose GPU-accelerated propagation functions
    m.def("forward_propagation_gpu", &forward_propagation_gpu, 
          "Forward propagation through 2-layer neural network (GPU)",
          py::arg("x"), py::arg("W1"), py::arg("b1"), py::arg("W2"), py::arg("b2"), py::arg("use_relu") = false);
    
    m.def("backward_propagation_gpu", &backward_propagation_gpu,
          "Backward propagation to compute gradients (GPU)",
          py::arg("x"), py::arg("y"), py::arg("W1"), py::arg("W2"), 
          py::arg("z1"), py::arg("a1"), py::arg("z2"), py::arg("a2"), py::arg("use_relu") = false);
}