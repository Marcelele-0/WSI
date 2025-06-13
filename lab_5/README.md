# CUDA Neural Network - Lab 5

A neural network implementation using CUDA for forward and backward propagation with Python bindings.

## 🛠️ Requirements

- **System:** Linux (tested on Arch Linux)
- **Python:** 3.11+
- **CUDA:** 12.0+ (NVIDIA GPU required)
- **Dependencies:** CMake, GCC, pybind11, numpy, hydra-core

## 🚀 Quick Start

### 1. Clone and Setup Environment

```bash
cd lab_5

# Create conda environment
conda env create -f environment.yml
conda activate cuda-nn-lab5

# Or install manually
pip install numpy hydra-core pybind11
```

### 2. Build CUDA Module

```bash
cd src/propagation
make clean && make
```

### 3. Run Training

```bash
cd ../..
python src/train.py
```

## 📁 Project Structure

```
lab_5/
├── README.md
├── environment.yaml
├── config/
│   └── config_train.yaml       # Hydra configuration
├── src/
│   ├── train.py                # Main training script
│   ├── models/
│   │   ├── __init__.py
│   │   └── neural_network.py   # Neural network implementation
│   ├── utils/
│   │   └── data.py             # Data utilities
│   └── propagation/
│       ├── popragation_algorithms.cu    # CUDA implementation
│       ├── popragation_algorithms.hpp   # C++ headers
│       ├── CMakeLists.txt
│       └── Makefile
└── tests/                      # Unit tests
```

## ⚙️ Configuration

Main configuration in `config/config_train.yaml`:

```yaml
data:
  n_samples: 1000
  normalization_method: "l2"     # "l1", "l2", or None

training:
  epochs: 500
  learning_rate: 0.1
  use_relu: true                 # true for ReLU, false for sigmoid
```

## 🧠 Neural Network Architecture

- **Input Layer:** 2 neurons (2D input data)
- **Hidden Layer:** 4 neurons with ReLU/Sigmoid activation
- **Output Layer:** 1 neuron with Sigmoid activation
- **Task:** Binary classification (XOR-like problem)

## � CUDA Functions

Available functions in `cuda_nn` module:

```python
import cuda_nn

# Activation functions
cuda_nn.sigmoid(x)
cuda_nn.relu(x)
cuda_nn.sigmoid_derivative(x)
cuda_nn.relu_derivative(x)

# Propagation functions
cuda_nn.forward_propagation(x, W1, b1, W2, b2, use_relu=False)
cuda_nn.backward_propagation(x, y, W1, W2, z1, a1, z2, a2, use_relu=False)
```

## � Example Output

```bash
$ python src/train.py
INFO - Starting training with config: n_samples=1000, normalization=l2
INFO - Training neural network for 500 epochs with learning_rate=0.1
INFO - Using ReLU activation in hidden layer
INFO - Epoch 0, Average Loss: 0.052257
INFO - Epoch 100, Average Loss: 0.002886
INFO - Epoch 400, Average Loss: 0.001527
INFO - Training completed. Final loss: 0.002053
INFO - Sample 0: Input=[ 0.136 -0.991], Target=0.0000, Prediction=0.0000
INFO - Sample 1: Input=[-0.913  0.408], Target=0.0000, Prediction=0.0001
```

## 🚨 Troubleshooting

### CUDA Not Found
```bash
# Install CUDA on Arch Linux
sudo pacman -S cuda gcc

# Install CUDA on Ubuntu/Debian
sudo apt install nvidia-cuda-toolkit build-essential
```

### Build Errors
```bash
# Clean and rebuild
cd src/propagation
make clean && make

# Check if CUDA module was built
ls build/cuda_nn.cpython-*.so
```

## 🔍 Testing

```bash
# Run unit tests
python -m pytest tests/

# Test CUDA module
cd src/propagation && make test
```

## � License

Educational project - WSI Lab 5