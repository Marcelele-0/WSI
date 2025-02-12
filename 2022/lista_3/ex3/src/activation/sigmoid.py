import numpy as np

import numpy as np

def sigmoid(x):
    """
    Funkcja aktywacji - Sigmoid.

    Args:
        x (float lub np.array): Oryginalna wartość wejściowa.

    Returns:
        float lub np.array: Wynik działania funkcji sigmoidalnej.
    """
    return 1 / (1 + np.exp(-x))

def sigmoid_derivative(x):
    """
    Pochodna funkcji sigmoidalnej, obliczana na podstawie oryginalnego x.

    Wzór: sigma'(x) = sigma(x) * (1 - sigma(x))

    Args:
        x (float lub np.array): Oryginalna wartość wejściowa.

    Returns:
        float lub np.array: Pochodna funkcji sigmoidalnej.
    """
    s = sigmoid(x)
    return s * (1 - s)
