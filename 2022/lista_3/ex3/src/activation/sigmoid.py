import numpy as np

def sigmoid(x):
    """
    Activation function - Sigmoid.

    Args:
        x (float or np.array): Original input value.

    Returns:
        float or np.array: Result of the sigmoid function.
    """
    return 1 / (1 + np.exp(-x))

def sigmoid_derivative(x):
    """
    Derivative of the sigmoid function, calculated based on the original x.

    Formula: sigma'(x) = sigma(x) * (1 - sigma(x))

    Args:
        x (float or np.array): Original input value.

    Returns:
        float or np.array: Derivative of the sigmoid function.
    """
    s = sigmoid(x)
    return s * (1 - s)
