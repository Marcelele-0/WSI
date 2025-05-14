import pandas as pd

def calculate_averages(input_file):
    # Wczytanie danych z pliku CSV do DataFrame
    df = pd.read_csv(input_file)

    # Obliczenie średnich dla każdej heurystyki
    averages = df.groupby('heuristic').agg(
        avg_visited=('visited', 'mean'),
        avg_steps=('steps', 'mean')
    ).reset_index()

    # Wydrukowanie tabeli w ładnej formie
    print(averages.to_string(index=False, float_format="%.2f"))

if __name__ == "__main__":
    input_file = 'results.csv'  # Ścieżka do pliku CSV z wynikami
    calculate_averages(input_file)
