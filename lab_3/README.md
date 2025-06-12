# Smart Bot - Inteligentny Bot do Gry w Kółko i Krzyżyk

## Autor
**[Marcel Musialek]**  
**[279704]**  
**Data:** Czerwiec 2025

## Opis Projektu

Smart Bot to zaawansowany bot do gry w kółko i krzyżyk na planszy 5x5, implementujący:
- **Algorytm Minimax** z przycinaniem alfa-beta
- **Książkę otwarć** z automatycznym uczeniem
- **Równoległe przetwarzanie** (OpenMP) dla optymalizacji
- **Zaawansowaną heurystykę** oceny pozycji
- **System symetrii** dla redukcji przestrzeni stanów

## Struktura Plików

```
├── game_smart_bot.c     # Główny kod bota i logika gry
├── heuristic.c          # Funkcje heurystyczne i algorytm minimax
├── heuristic.h          # Nagłówek dla funkcji heurystycznych
├── opening_book.c       # Implementacja książki otwarć
├── opening_book.h       # Nagłówek dla książki otwarć
├── opening_book.txt     # Plik z danymi książki otwarć
├── board.h              # Definicje planszy i podstawowych funkcji
├── Makefile             # Skrypt kompilacji
└── README.md            # Ten plik
```

## Wymagania Systemowe

### Kompilator i Biblioteki
- **GCC** (GNU Compiler Collection) z obsługą C2x
- **OpenMP** library (`libgomp`)
- **System operacyjny:** Linux/Unix
- **Architektura:** x86_64 (rekomendowane)

### Instalacja wymagań (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install gcc libc6-dev libgomp1
```

### Instalacja wymagań (Red Hat/CentOS)
```bash
sudo yum install gcc glibc-devel libgomp
```

## Kompilacja

### Standardowa kompilacja
```bash
make clean
make game_smart_bot
```

### Opcje kompilacji
Program jest kompilowany z następującymi flagami optymalizacji:
- `-O3` - maksymalna optymalizacja wydajności
- `-fopenmp` - wsparcie równoległego przetwarzania
- `-std=c2x` - najnowszy standard C
- `-W -pedantic` - dodatkowe ostrzeżenia

### Ręczna kompilacja
```bash
gcc -W -pedantic -std=c2x -O3 -fopenmp game_smart_bot.c heuristic.c opening_book.c -o game_smart_bot -lgomp
```

## Uruchamianie

### Tryb Gry Sieciowej
```bash
./game_smart_bot <IP_SERWERA> <PORT> <ID_GRACZA> <N> <GŁĘBOKOŚĆ>
```

**Parametry:**
- `IP_SERWERA` - adres IP serwera gry
- `PORT` - port serwera
- `ID_GRACZA` - identyfikator gracza (1 lub 2)
- `N` - dodatkowy parametr gry
- `GŁĘBOKOŚĆ` - głębokość przeszukiwania minimax (zalecane: 8-10)

**Przykład:**
```bash
./game_smart_bot 127.0.0.1 8080 1 0 8
```

### Tryb Uczenia Książki Otwarć
```bash
./game_smart_bot --learn-depth=<GŁĘBOKOŚĆ_UCZENIA> --search-depth=<GŁĘBOKOŚĆ_MINIMAX>
```

**Parametry:**
- `GŁĘBOKOŚĆ_UCZENIA` - ile ruchów otwarcia analizować (zalecane: 4-6)
- `GŁĘBOKOŚĆ_MINIMAX` - głębokość analizy minimax (zalecane: 6-10)


## Funkcjonalności

### 1. Algorytm Minimax
- **Przycinanie alfa-beta** dla optymalizacji
- **Konfigurowalna głębokość** przeszukiwania
- **Sprawdzanie legalnych ruchów** (unikanie 3 w rzędzie)
- **Detekcja stanów końcowych** (wygrana/przegrana)

### 2. Książka Otwarć
- **Automatyczne uczenie** optymalnych sekwencji otwarcia
- **Hash table** dla szybkiego wyszukiwania O(1)
- **System symetrii** redukujący przestrzeń stanów
- **Zapis/odczyt** z pliku tekstowego
- **Równoległe generowanie** z OpenMP

### 3. Heurystyka Oceny
- **Ocena pozycji** na planszy 5x5
- **Preferowanie środka** planszy
- **Blokowanie** zagrożeń przeciwnika
- **Tworzenie** własnych możliwości wygranej

### 4. Optymalizacje Wydajności
- **OpenMP parallelization** - do 28 wątków
- **Inteligentne cięcie** - analiza tylko najlepszych ruchów
- **Memory pooling** - efektywne zarządzanie pamięcią
- **Branch prediction** - optymalne uporządkowanie warunków

## Technologie i Biblioteki

### Języki i Standardy
- **C2x** (najnowszy standard C)
- **POSIX** sockets dla komunikacji sieciowej

### Biblioteki Zewnętrzne
- **OpenMP** (`libgomp`) - równoległe przetwarzanie
- **Standard C Library** (`libc`) - podstawowe funkcje
- **POSIX Threads** - zarządzanie wątkami

### Narzędzia Deweloperskie
- **GCC** - kompilator z pełną optymalizacją
- **Make** - automatyzacja kompilacji
- **Strip** - usuwanie symboli debug dla mniejszego rozmiaru

## Parametry Wydajności

### Rekomendowane Ustawienia
- **Głębokość minimax:** 8-10 (kompromis między jakością a szybkością)
- **Uczenie książki:** depth=4, search=8 (optymalne dla większości przypadków)
- **Wątki OpenMP:** automatycznie dostosowane do liczby rdzeni CPU

### Benchmark Wydajności
- **Procesor:** Intel/AMD wielordzeniowy
- **Pamięć:** 4GB+ RAM (zalecane dla głębokiego uczenia)
- **Czas uczenia:** 
  - Depth 4: ~30 sekund - 2 minuty
  - Depth 5: ~2-10 minut
  - Depth 6: ~10-60 minut

## Rozwiązywanie Problemów

### Błędy Kompilacji
```bash
# Jeśli brak OpenMP
sudo apt-get install libgomp1-dev

# Jeśli błędy linkowania
make clean && make
```

### Błędy Uruchomienia
```bash
# Sprawdź uprawnienia
chmod +x game_smart_bot

# Sprawdź pliki
ls -la opening_book.txt
```

### Optymalizacja Wydajności
```bash
# Sprawdź dostępne rdzenie
nproc

# Uruchom z ograniczeniem wątków
OMP_NUM_THREADS=4 ./game_smart_bot --learn-depth=4 --search-depth=8
```

## Licencja

Projekt edukacyjny - wykorzystanie zgodnie z zasadami uczelni.

---
**Ostatnia aktualizacja:** Czerwiec 2025
