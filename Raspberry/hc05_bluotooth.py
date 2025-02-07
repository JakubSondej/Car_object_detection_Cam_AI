import serial
import threading
import time

# Globalne zmienne
ser = None

running = True
lock = threading.Lock()

# Słownik przechowujący dane z UART
phone_box = {
    "velocity": 0,
    "direction": 0,
    "poz_x": 0,
    "poz_y": 0,
    "around": "neutral"
}

# Inicjalizacja połączenia UART
def initialize_uart_hc():
    """
    Inicjalizacja połączenia UART.
    """
    global ser
    try:
        ser = serial.Serial('/dev/serial0', 9600, timeout=1) # /dev/serial0 , /dev/ttyS0
        ser.flush()
        print("UART HC został zainicjalizowany.")
        return True
    except serial.SerialException as e:
        print(f"Błąd inicjalizacji UART HC: {e}")
        return False

"""
def parse_data(data):
    
    #Parsowanie danych odebranych z UART.
    #Oczekiwany format: 'Velocity= 0, Direction= 0, PozX= 140, PozY= 140, Around= left'
    
    global phone_box
    try:
        parts = data.split(',')
        normalized_keys = {
            "velocity": "velocity",
            "direction": "direction",
            "pozx": "poz_x",
            "pozy": "poz_y",
            "around": "around"
        }

        temp_data = {}
        for part in parts:
            try:
                key, value = part.split('=')
                key = key.strip().lower()
                value = int(value.strip())

                # Obsługa klucza "around" (tekst)
                if key == "around":
                    value = value.lower()  # Normalizacja wartości tekstowych (np. "left", "neutral")
                else:
                    value = int(value)  # Konwersja wartości liczbowych

                if key.lower() == "around":
                    value = value.lower()  # Przykładowe wartości: "left", "right", "neutral"
                else:
                    value = int(value)

                # Konwersja wartości liczbowych
                if key.lower() != "around":
                    value = int(value)
                    
                # Normalizacja kluczy
                normalized_key = normalized_keys.get(key, key)
                temp_data[normalized_key] = value
            except ValueError:
                print(f"Nieprawidłowy fragment danych o aroundd: {part}")

        with lock:
            phone_box.update(temp_data)  # Aktualizacja danych w słowniku

        print(f"Parsed data: {phone_box}")
    except Exception as e:
        print(f"Błąd podczas parsowania danych: {e}")"""

# Funkcja parsująca dane z UART
def parse_data(data):
    """
    Parsowanie danych odebranych z UART.
    Oczekiwany format: 'Velocity= 0, Direction= 0, PozX= 140, PozY= 140, Around= 1'
    Wartości pola "around":
    - 0 -> neutral
    - 1 -> left
    - 2 -> right
    """
    global phone_box
    try:
        print(f"Raw data for parsing: {data}")  # Log danych wejściowych

        parts = data.split(',')
        normalized_keys = {
            "velocity": "velocity",
            "direction": "direction",
            "pozx": "poz_x",
            "pozy": "poz_y",
            "around": "around"
        }

        # Mapa konwersji dla pola "around"
        around_map = {
            "0": "neutral",
            "1": "left",
            "2": "right"
        }

        temp_data = {}
        for part in parts:
            try:
                # Rozdzielenie klucza i wartości
                if '=' in part:
                    key, value = part.split('=')
                    key = key.strip().lower()
                    value = value.strip()

                    # Obsługa pola "around"
                    if key == "around":
                        # Zamiana wartości numerycznej na tekstową (np. 1 -> left)
                        value = around_map.get(value, "unknown")
                    else:
                        value = int(value)  # Konwersja wartości liczbowych

                    # Normalizacja kluczy
                    normalized_key = normalized_keys.get(key, key)
                    temp_data[normalized_key] = value
                else:
                    print(f"Nieprawidłowy fragment danych: {part}")

            except ValueError as ve:
                print(f"Nieprawidłowy fragment danych: {part} -> {ve}")

        with lock:
            phone_box.update(temp_data)

        print(f"Parsed phone_box: {phone_box}")
    except Exception as e:
        print(f"Błąd podczas parsowania danych: {e}")

# Wątek do odbioru i przetwarzania danych z UART
def uart_hc_processing():
    """
    Funkcja wątku do odbioru i przetwarzania danych z UART.
    """
    global running
    try:
        while running:
            if ser and ser.in_waiting > 0:
                received_data = ser.readline().decode('utf-8', errors='ignore').strip()
                #print(f"Odebrano z telefonu: {received_data}")

                if "velocity" in received_data.lower() and "direction" in received_data.lower():
                    parse_data(received_data)
                    print(f"Odebrano z telefonu 2: {received_data}")
            else:
                print("Brak danych na porcie UART.")  # Log gdy brak danych

            time.sleep(0.15)  # Ograniczenie obciążenia procesora
    except serial.SerialException as e:
        print(f"Błąd portu szeregowego: {e}")
    except OSError as e:
        print(f"Błąd wejścia/wyjścia: {e}")
    except KeyboardInterrupt:
        print("Program zatrzymany.")
    finally:
        close_uart_hc()

# Funkcja zwracająca aktualne dane z UART
def get_phone_box():
    """
    Zwraca aktualne dane z UART.
    """
    global phone_box
    print(f"phone_box w get_phone_box: {phone_box}")  # Diagnostyka
    with lock:
        return phone_box.copy()

# Funkcja zamykająca port UART
def close_uart_hc():
    """
    Zamykanie portu UART.
    """
    global ser
    if ser and ser.is_open:
        ser.close()
        print("Port UART HC został zamknięty.")
