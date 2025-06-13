# Simple Makefile for CUDA NN project
BUILD_DIR = build

.PHONY: all clean setup build test

all: build

setup:
	@echo "Setting up build directory..."
	mkdir -p $(BUILD_DIR)

build: setup
	@echo "Building CUDA NN module..."
	cd $(BUILD_DIR) && cmake .. && make -j4

clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)

test: build
	@echo "Testing the module..."
	cd $(BUILD_DIR) && python3 -c "import cuda_nn; print('sigmoid(0.5) =', cuda_nn.sigmoid(0.5))"

rebuild: clean build

install: build
	@echo "Module built successfully in $(BUILD_DIR)/"
	@echo "Add to Python path: sys.path.append('$(PWD)/$(BUILD_DIR)')"

help:
	@echo "Available targets:"
	@echo "  all/build  - Build the CUDA module"
	@echo "  clean      - Remove build directory"
	@echo "  test       - Build and test the module"
	@echo "  rebuild    - Clean and build"
	@echo "  help       - Show this help"
