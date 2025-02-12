import numpy as np

# Funkcja aktywacji - ReLU
def relu(x):
    """Funkcja aktywacji - ReLU

    Args:
        x (float or np.array): wartość wejściowa

    Returns:
        float or np.array: wartość funkcji ReLU
    """
    return np.maximum(0, x)

# Pochodna ReLU
def relu_derivative(x):
    """Pochodna funkcji aktywacji ReLU

    Args:
        x (float or np.array): wartość wejściowa

    Returns:
        float or np.array: pochodna funkcji ReLU
    """
    return np.where(x > 0, 1, 0)