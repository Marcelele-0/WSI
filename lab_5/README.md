# CUDA Neural Network Lab 5

Projekt laboratoryjny implementujący sieć neuronową z wykorzystaniem CUDA i Python.

## 🛠️ Wymagania

- **System:** Linux (testowane na Arch Linux)
- **Python:** 3.11+
- **CUDA:** 12.0+ (NVIDIA GPU wymagana)
- **Conda/Miniconda**

## 🚀 Szybki start

### 1. Klonowanie i setup środowiska

```bash
cd lab_5

# Stwórz środowisko conda
conda env create -f environment.yml

# Aktywuj środowisko
conda activate cuda-nn-lab5
```

### 2. Kompilacja modułu CUDA

```bash
# Przejdź do folderu propagation
cd src/propagation

# Skompiluj moduł CUDA
make build

# (Opcjonalnie) Przetestuj moduł
make test
```

### 3. Uruchomienie treningu

```bash
# Wróć do głównego folderu src
cd ..

# Uruchom skrypt treningowy
python train.py
```

## 📁 Struktura projektu

```
lab_5/
├── README.md                    # Ten plik
├── environment.yml              # Definicja środowiska conda
├── requirements.txt             # Alternatywna lista zależności
├── setup.sh                     # Skrypt automatycznego setup
├── config/
│   └── config_train.yaml        # Konfiguracja Hydra
├── src/
│   ├── train.py                 # Główny skrypt treningowy
│   ├── utils/
│   │   └── data.py              # Narzędzia do obsługi danych
│   └── propagation/
│       ├── popragation_algorithms.cu   # Implementacja CUDA
│       ├── popragation_algorithms.hpp  # Nagłówek C++
│       ├── CMakeLists.txt       # Konfiguracja CMake
│       └── Makefile             # Uproszczone polecenia budowania
└── tests/                       # Testy jednostkowe
```

## ⚙️ Konfiguracja

Główna konfiguracja znajduje się w `config/config_train.yaml`:

```yaml
data:
  n_samples: 1000                # Liczba próbek danych
  normalization_method: "l2"     # Metoda normalizacji: "l1", "l2", None

training:
  epochs: 500                    # Liczba epok
  lr: 0.1                       # Learning rate
  activation: "relu"            # Funkcja aktywacji: "relu", "sigmoid"
```

## 🔧 Polecenia budowania

W folderze `src/propagation/`:

```bash
make build    # Kompiluj moduł CUDA
make test     # Kompiluj i przetestuj
make clean    # Wyczyść pliki budowania
make help     # Pokaż wszystkie dostępne polecenia
```

## 🐍 Funkcje CUDA

Dostępne funkcje w module `cuda_nn`:

```python
import cuda_nn

# Funkcje aktywacji
cuda_nn.sigmoid(x)              # Funkcja sigmoid
cuda_nn.sigmoid_derivative(x)   # Pochodna sigmoid
cuda_nn.relu(x)                 # Funkcja ReLU
cuda_nn.relu_derivative(x)      # Pochodna ReLU
```

## 🚨 Rozwiązywanie problemów

### Błąd: `cmake: command not found`
```bash
# Na Arch Linux
sudo pacman -S cmake

# Na Ubuntu/Debian
sudo apt install cmake
```

### Błąd: `nvcc: command not found`
```bash
# Na Arch Linux
sudo pacman -S cuda

# Na Ubuntu/Debian
sudo apt install nvidia-cuda-toolkit
```

### Błąd importu `cuda_nn`
Sprawdź czy moduł został skompilowany:
```bash
cd src/propagation
make build
ls build/  # Powinien być plik cuda_nn.cpython-*.so
```

## 📊 Przykładowe uruchomienie

```bash
$ python src/train.py
hydra.core.utils - DEBUG - Setting JobRuntime:name=UNKNOWN_NAME
hydra.core.utils - DEBUG - Setting JobRuntime:name=train
INFO - Starting training with config: n_samples=1000, normalization=l2
INFO - Testing CUDA neural network functions:
INFO - sigmoid(0.5) = 0.622459352016449
INFO - relu(0.5) = 0.5
```

## 🔍 Testowanie

```bash
# Uruchom testy jednostkowe
python -m pytest tests/

# Test modułu CUDA
cd src/propagation && make test
```

## 💡 Rozwój

- Dodaj więcej funkcji aktywacji
- Implementuj pełny forward/backward pass
- Dodaj różne optymalizatory
- Rozszerz testy jednostkowe

## 📝 Licencja

Projekt edukacyjny - WSI Lab 5