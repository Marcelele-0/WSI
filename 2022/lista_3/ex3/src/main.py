import numpy as np
from activation.sigmoid import sigmoid, sigmoid_derivative
from activation.relu import relu, relu_derivative
from data.normalisation import normalize_data
from network.network import NeuralNetwork
import hydra 
from omegaconf import DictConfig, OmegaConf

@hydra.main(version_base=None, config_path="../config", config_name="main")
def main(cfg: DictConfig):
    print(OmegaConf.to_yaml(cfg))  # Print the configuration

    method = cfg.normalisation.method
    epochs = cfg.training.epochs
    learning_rate = cfg.training.learning_rate

    # Map activation functions
    activation_functions = {
        'sigmoid': sigmoid,
        'relu': relu
    }
    activation_derivatives = {
        'sigmoid_derivative': sigmoid_derivative,
        'relu_derivative': relu_derivative
    }

    activation_function = activation_functions[cfg.activation.function]
    activation_derivative = activation_derivatives[cfg.activation.derivative]

    X = np.array([[0.5, 0.5], [-0.5, 0.5], [0.5, -0.5], [-0.5, -0.5]])  # Inputs
    y = np.array([[1], [0], [0], [1]])  # Expected outputs

    # Normalize data (e.g., L2)
    X_normalized = np.array([normalize_data(x[0], x[1], method) for x in X])

    # Initialize the network
    nn = NeuralNetwork(input_size=2, hidden_size=4, output_size=1, 
                    activation_function=activation_function, 
                    activation_derivative=activation_derivative)

    # Train the network
    nn.train(X_normalized, y, epochs, learning_rate)

if __name__ == "__main__":
    main()