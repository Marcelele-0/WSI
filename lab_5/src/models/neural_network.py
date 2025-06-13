"""
Simple Neural Network implementation using CUDA propagation functions.
"""
import logging
import os
import sys
import numpy as np

# Add path to CUDA module
current_dir = os.path.dirname(__file__)
cuda_path = os.path.join(current_dir, "..", "propagation", "build")
sys.path.append(cuda_path)
import cuda_nn  # type: ignore


class SimpleNeuralNetwork:
    """Simple 2-layer neural network using CUDA propagation functions."""
    
    def __init__(self, input_size=2, hidden_size=4, output_size=1, use_relu=False):
        """Initialize the neural network with random weights."""
        self.input_size = input_size
        self.hidden_size = hidden_size
        self.output_size = output_size
        self.use_relu = use_relu
        
        # Initialize weights and biases
        np.random.seed(42)
        self.W1 = np.random.randn(input_size, hidden_size).astype(np.float32) * 0.5
        self.b1 = np.zeros(hidden_size, dtype=np.float32)
        self.W2 = np.random.randn(hidden_size, output_size).astype(np.float32) * 0.5
        self.b2 = np.zeros(output_size, dtype=np.float32)
    
    def forward(self, x):
        """Forward propagation using CUDA function."""
        x = x.astype(np.float32)
        result = cuda_nn.forward_propagation(x, self.W1, self.b1, self.W2.flatten(), self.b2, self.use_relu)
        
        # Extract hidden layer activations (a1), z2, and output (a2)
        self.a1 = result[:4]  # Hidden layer activations
        self.z2 = result[4]   # Output layer before activation
        self.a2 = result[5]   # Final output
        
        return self.a2
    
    def backward(self, x, y):
        """Backward propagation using CUDA function."""
        x = x.astype(np.float32)
        y = np.array([y], dtype=np.float32)
        
        # We need z1 for backward propagation - let's compute it
        z1 = np.zeros(4, dtype=np.float32)
        for i in range(4):
            z1[i] = self.b1[i]
            for j in range(2):
                z1[i] += x[j] * self.W1[j, i]
        
        gradients = cuda_nn.backward_propagation(
            x, y, self.W1, self.W2.flatten(), z1, self.a1, self.z2, self.a2, self.use_relu
        )
        
        return gradients
    
    def update_weights(self, gradients, learning_rate):
        """Update weights using computed gradients."""
        self.W1 -= learning_rate * gradients['dW1']
        self.b1 -= learning_rate * gradients['db1']
        self.W2 -= learning_rate * gradients['dW2'].reshape(-1, 1)
        self.b2 -= learning_rate * gradients['db2']
    
    def train_step(self, x, y, learning_rate):
        """Perform one training step."""
        # Forward pass
        prediction = self.forward(x)
        
        # Backward pass
        gradients = self.backward(x, y)
        
        # Update weights
        self.update_weights(gradients, learning_rate)
        
        # Compute loss
        loss = 0.5 * (prediction - y) ** 2
        return loss, prediction


def train_neural_network(X, y, epochs=1000, learning_rate=0.1, use_relu=False):
    """Train the neural network using CUDA propagation functions."""
    logger = logging.getLogger(__name__)
    
    # Initialize network
    nn = SimpleNeuralNetwork(use_relu=use_relu)
    
    logger.info(f"Training neural network for {epochs} epochs with learning_rate={learning_rate}")
    logger.info(f"Using {'ReLU' if use_relu else 'Sigmoid'} activation in hidden layer")
    
    losses = []
    
    for epoch in range(epochs):
        epoch_loss = 0
        
        # Training on each sample
        for i in range(len(X)):
            loss, prediction = nn.train_step(X[i], y[i], learning_rate)
            epoch_loss += float(loss)  # Convert to scalar
        
        avg_loss = epoch_loss / len(X)
        losses.append(avg_loss)
        
        if epoch % 100 == 0:
            logger.info(f"Epoch {epoch}, Average Loss: {avg_loss:.6f}")
    
    logger.info(f"Training completed. Final loss: {losses[-1]:.6f}")
    
    # Test predictions
    logger.info("Final predictions vs targets:")
    for i in range(min(10, len(X))):  # Show first 10 samples
        pred = nn.forward(X[i])
        logger.info(f"Sample {i}: Input={X[i]}, Target={float(y[i]):.4f}, Prediction={float(pred):.4f}")
    
    return nn, losses
