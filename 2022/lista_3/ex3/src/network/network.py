import numpy as np

class NeuralNetwork:
    """Klasa reprezentująca sieć neuronową.
    
    Args:
        input_size (int): liczba wejść
        hidden_size (int): liczba neuronów w warstwie ukrytej
        output_size (int): liczba wyjść
        activation_function (function): funkcja aktywacji
        activation_derivative (function): pochodna funkcji aktywacji
    """
    def __init__(self, input_size, hidden_size, output_size, activation_function, activation_derivative):
        self.input_size = input_size
        self.hidden_size = hidden_size
        self.output_size = output_size
        self.activation_function = activation_function
        self.activation_derivative = activation_derivative

        # Inicjalizacja wag
        self.weights_input_hidden = np.random.randn(input_size, hidden_size)
        self.weights_hidden_output = np.random.randn(hidden_size, output_size)
        
    def forward(self, X):
        # Propagacja do przodu
        self.hidden_input = np.dot(X, self.weights_input_hidden)        # oryginalne x dla warstwy ukrytej
        self.hidden_output = self.activation_function(self.hidden_input)  # wynik po aktywacji

        self.output_input = np.dot(self.hidden_output, self.weights_hidden_output)  # oryginalne x dla warstwy wyjściowej
        self.output = self.activation_function(self.output_input)        # wynik po aktywacji

        return self.output

    def backward(self, X, y, learning_rate):
        error = y - self.output
        # Używamy oryginalnych wartości wejściowych do funkcji aktywacji:
        output_delta = error * self.activation_derivative(self.output_input)

        hidden_error = output_delta.dot(self.weights_hidden_output.T)
        hidden_delta = hidden_error * self.activation_derivative(self.hidden_input)
        
        # Aktualizacja wag
        self.weights_hidden_output += self.hidden_output.T.dot(output_delta) * learning_rate
        self.weights_input_hidden += X.T.dot(hidden_delta) * learning_rate


    def train(self, X, y, epochs, learning_rate):
        """Trenuje sieć neuronową."""
        for epoch in range(epochs):
            self.forward(X)
            self.backward(X, y, learning_rate)
            if epoch % 1000 == 0:
                print(f'Epoch {epoch}, Loss: {np.mean(np.square(y - self.output))}')


                
