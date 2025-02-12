import numpy as np

def relu(x):
    """Activation function - ReLU

    Args:
        x (float or np.array): Input value

    Returns:
        float or np.array: Result of the ReLU function
    """
    return np.maximum(0, x)

def relu_derivative(x):
    """Derivative of the ReLU activation function

    Args:
        x (float or np.array): Input value

    Returns:
        float or np.array: Derivative of the ReLU function
    """
    return np.where(x > 0, 1, 0)