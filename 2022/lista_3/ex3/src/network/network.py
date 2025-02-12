import numpy as np

class NeuralNetwork:
    """Class representing a neural network.
    
    Args:
        input_size (int): number of inputs
        hidden_size (int): number of neurons in the hidden layer
        output_size (int): number of outputs
        activation_function (function): activation function
        activation_derivative (function): derivative of the activation function
    """
    def __init__(self, input_size, hidden_size, output_size, activation_function, activation_derivative):
        self.input_size = input_size
        self.hidden_size = hidden_size
        self.output_size = output_size
        self.activation_function = activation_function
        self.activation_derivative = activation_derivative

        # Initialize weights
        self.weights_input_hidden = np.random.randn(input_size, hidden_size)
        self.weights_hidden_output = np.random.randn(hidden_size, output_size)
        
    def forward(self, X):
        # Forward propagation
        self.hidden_input = np.dot(X, self.weights_input_hidden)  # original x for the hidden layer
        self.hidden_output = self.activation_function(self.hidden_input)  # result after activation

        self.output_input = np.dot(self.hidden_output, self.weights_hidden_output)  # original x for the output layer
        self.output = self.activation_function(self.output_input)  # result after activation

        return self.output

    def backward(self, X, y, learning_rate):
        error = y - self.output
        # Use original input values for the activation function:
        output_delta = error * self.activation_derivative(self.output_input)

        hidden_error = output_delta.dot(self.weights_hidden_output.T)
        hidden_delta = hidden_error * self.activation_derivative(self.hidden_input)
        
        # Update weights
        self.weights_hidden_output += self.hidden_output.T.dot(output_delta) * learning_rate
        self.weights_input_hidden += X.T.dot(hidden_delta) * learning_rate

    def train(self, X, y, epochs, learning_rate):
        """Train the neural network."""
        for epoch in range(epochs):
            self.forward(X)
            self.backward(X, y, learning_rate)
            if epoch % 1000 == 0:
                print(f'Epoch {epoch}, Loss: {np.mean(np.square(y - self.output))}')



