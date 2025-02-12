import numpy as np

# Funkcja normalizująca dane
def normalize_data(x1, x2, method):
    if method == 'L1':
        norm = abs(x1) + abs(x2)
    elif method == 'L2':
        norm = np.sqrt(x1**2 + x2**2)
    return x1 / norm, x2 / norm