#include "popragation_algorithms.hpp"
#include "propagation_algorithms_gpu.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <cuda_runtime.h>

namespace py = pybind11;

/**
 * @brief CUDA kernel for forward propagation
 */
__global__ void forward_propagation_kernel(
    const float* x,                     // input 2-elementowy
    const float* W1, const float* b1,   // W1: 2x4, b1: 4
    const float* W2, const float* b2,   // W2: 4x1, b2: 1
    float* z1, float* a1,               // z1: 4, a1: 4
    float* z2, float* a2,               // z2: 1, a2: 1
    bool use_relu                       // wybór aktywacji
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    // Warstwa ukryta - każdy wątek oblicza jeden neuron
    if (idx < 4) {
        z1[idx] = b1[idx];
        for (int j = 0; j < 2; ++j) {
            z1[idx] += x[j] * W1[j * 4 + idx];  // W1: [2][4]
        }
        a1[idx] = use_relu ? relu(z1[idx]) : sigmoid(z1[idx]);
    }
    
    // Synchronizacja przed obliczeniem warstwy wyjściowej
    __syncthreads();
    
    // Warstwa wyjściowa - tylko pierwszy wątek
    if (idx == 0) {
        *z2 = b2[0];
        for (int i = 0; i < 4; ++i) {
            *z2 += a1[i] * W2[i];  // W2: [4][1]
        }
        *a2 = sigmoid(*z2);  // Na wyjściu zawsze sigmoid
    }
}

/**
 * @brief CUDA kernel for backward propagation
 */
__global__ void backward_propagation_kernel(
    const float* x,                      // input 2-elementowy
    const float* y,                      // target (1-elementowy)
    const float* W1, const float* W2,    // wagi
    const float* z1, const float* a1,    // z i a z forwarda
    float z2, float a2,                  // z2 i a2 z forwarda
    float* dW1, float* db1,              // gradienty
    float* dW2, float* db2,
    bool use_relu
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    // Warstwa wyjściowa - obliczana przez pierwszy wątek
    float dz2;
    if (idx == 0) {
        dz2 = a2 - y[0];
        for (int i = 0; i < 4; ++i) {
            dW2[i] = a1[i] * dz2;
        }
        db2[0] = dz2;
    }
    
    // Synchronizacja aby mieć dz2
    __syncthreads();
    dz2 = a2 - y[0];  // Ponowne obliczenie dla wszystkich wątków
    
    // Warstwa ukryta - każdy wątek oblicza gradienty dla jednego neuronu
    if (idx < 4) {
        float da1 = W2[idx] * dz2;
        float dz1 = da1 * (use_relu ? relu_derivative(z1[idx]) : sigmoid_derivative(a1[idx]));

        db1[idx] = dz1;

        for (int j = 0; j < 2; ++j) {
            dW1[j * 4 + idx] = x[j] * dz1;  // W1: [2][4]
        }
    }
}

// GPU-accelerated wrapper functions
py::array_t<float> forward_propagation_gpu(
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
    
    // Allocate GPU memory
    float *d_x, *d_W1, *d_b1, *d_W2, *d_b2;
    float *d_z1, *d_a1, *d_z2, *d_a2;
    
    cudaMalloc(&d_x, 2 * sizeof(float));
    cudaMalloc(&d_W1, 8 * sizeof(float));  // 2x4
    cudaMalloc(&d_b1, 4 * sizeof(float));
    cudaMalloc(&d_W2, 4 * sizeof(float));
    cudaMalloc(&d_b2, 1 * sizeof(float));
    cudaMalloc(&d_z1, 4 * sizeof(float));
    cudaMalloc(&d_a1, 4 * sizeof(float));
    cudaMalloc(&d_z2, 1 * sizeof(float));
    cudaMalloc(&d_a2, 1 * sizeof(float));
    
    // Copy data to GPU
    cudaMemcpy(d_x, x_buf.ptr, 2 * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_W1, W1_buf.ptr, 8 * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_b1, b1_buf.ptr, 4 * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_W2, W2_buf.ptr, 4 * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_b2, b2_buf.ptr, 1 * sizeof(float), cudaMemcpyHostToDevice);
    
    // Launch kernel
    dim3 blockSize(4);
    dim3 gridSize(1);
    forward_propagation_kernel<<<gridSize, blockSize>>>(
        d_x, d_W1, d_b1, d_W2, d_b2, d_z1, d_a1, d_z2, d_a2, use_relu
    );
    
    // Wait for kernel to complete
    cudaDeviceSynchronize();
    
    // Copy results back to host
    float h_a1[4], h_z2, h_a2;
    cudaMemcpy(h_a1, d_a1, 4 * sizeof(float), cudaMemcpyDeviceToHost);
    cudaMemcpy(&h_z2, d_z2, 1 * sizeof(float), cudaMemcpyDeviceToHost);
    cudaMemcpy(&h_a2, d_a2, 1 * sizeof(float), cudaMemcpyDeviceToHost);
    
    // Create result array
    auto result = py::array_t<float>(6);
    auto result_buf = result.request();
    float* result_ptr = static_cast<float*>(result_buf.ptr);
    
    for (int i = 0; i < 4; i++) {
        result_ptr[i] = h_a1[i];
    }
    result_ptr[4] = h_z2;
    result_ptr[5] = h_a2;
    
    // Free GPU memory
    cudaFree(d_x); cudaFree(d_W1); cudaFree(d_b1); cudaFree(d_W2); cudaFree(d_b2);
    cudaFree(d_z1); cudaFree(d_a1); cudaFree(d_z2); cudaFree(d_a2);
    
    return result;
}

py::dict backward_propagation_gpu(
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
    
    // Allocate GPU memory
    float *d_x, *d_y, *d_W1, *d_W2, *d_z1, *d_a1;
    float *d_dW1, *d_db1, *d_dW2, *d_db2;
    
    cudaMalloc(&d_x, 2 * sizeof(float));
    cudaMalloc(&d_y, 1 * sizeof(float));
    cudaMalloc(&d_W1, 8 * sizeof(float));
    cudaMalloc(&d_W2, 4 * sizeof(float));
    cudaMalloc(&d_z1, 4 * sizeof(float));
    cudaMalloc(&d_a1, 4 * sizeof(float));
    cudaMalloc(&d_dW1, 8 * sizeof(float));
    cudaMalloc(&d_db1, 4 * sizeof(float));
    cudaMalloc(&d_dW2, 4 * sizeof(float));
    cudaMalloc(&d_db2, 1 * sizeof(float));
    
    // Copy data to GPU
    cudaMemcpy(d_x, x_buf.ptr, 2 * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_y, y_buf.ptr, 1 * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_W1, W1_buf.ptr, 8 * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_W2, W2_buf.ptr, 4 * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_z1, z1_buf.ptr, 4 * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_a1, a1_buf.ptr, 4 * sizeof(float), cudaMemcpyHostToDevice);
    
    // Launch kernel
    dim3 blockSize(4);
    dim3 gridSize(1);
    backward_propagation_kernel<<<gridSize, blockSize>>>(
        d_x, d_y, d_W1, d_W2, d_z1, d_a1, z2, a2, d_dW1, d_db1, d_dW2, d_db2, use_relu
    );
    
    // Wait for kernel to complete
    cudaDeviceSynchronize();
    
    // Copy results back to host
    float h_dW1[8], h_db1[4], h_dW2[4], h_db2[1];
    cudaMemcpy(h_dW1, d_dW1, 8 * sizeof(float), cudaMemcpyDeviceToHost);
    cudaMemcpy(h_db1, d_db1, 4 * sizeof(float), cudaMemcpyDeviceToHost);
    cudaMemcpy(h_dW2, d_dW2, 4 * sizeof(float), cudaMemcpyDeviceToHost);  
    cudaMemcpy(h_db2, d_db2, 1 * sizeof(float), cudaMemcpyDeviceToHost);
    
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
    for (int i = 0; i < 8; i++) dW1_ptr[i] = h_dW1[i];
    for (int i = 0; i < 4; i++) db1_ptr[i] = h_db1[i];
    for (int i = 0; i < 4; i++) dW2_ptr[i] = h_dW2[i];
    db2_ptr[0] = h_db2[0];
    
    // Free GPU memory
    cudaFree(d_x); cudaFree(d_y); cudaFree(d_W1); cudaFree(d_W2);
    cudaFree(d_z1); cudaFree(d_a1); cudaFree(d_dW1); cudaFree(d_db1);
    cudaFree(d_dW2); cudaFree(d_db2);
    
    py::dict gradients;
    gradients["dW1"] = dW1_array;
    gradients["db1"] = db1_array;
    gradients["dW2"] = dW2_array;
    gradients["db2"] = db2_array;
    
    return gradients;
}
