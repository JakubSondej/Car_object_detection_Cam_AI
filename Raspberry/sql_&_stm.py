import mysql.connector
import serial
import threading
import time

# Globalne zmienne
connection = None
received_data_from_stm = {}
ser = None
running = True 
lock = threading.Lock()

# Funkcja inicjalizująca połączenie UART
def initialize_uart():
    global ser
    try:
        ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)  # Ustawienia portu UART
        ser.flush()
        print("UART został zainicjalizowany.")
    except Exception as e:
        print(f"Błąd podczas inicjalizacji UART: {e}")
        ser = None

# Funkcja inicjalizująca połączenie MySQL
def initialize_mysql():
    global connection
    try:
        connection = mysql.connector.connect(
            host='localhost',
            user='root',
            password='123',
            database='Projekt_db',
            port=3306
        )
        if connection.is_connected():
            print("Połączenie z bazą danych MySQL zostało nawiązane.")
        else:
            print("Nie udało się połączyć z bazą danych MySQL.")
    except mysql.connector.Error as err:
        print(f"Błąd połączenia z MySQL: {err}")
        connection = None

# Funkcja wysyłająca dane do STM32
def send_message(data_to_stm):
    global running
    while running:
        try:
            if ser.is_open:
                with lock:
                    command = data_to_stm.get('command', '0')  # Domyślnie '0' (Stop)
                    ser.write(f"{command}".encode())
                    print(f"Wiadomość do STM32 wysłana!: {command}")
            else:
                print("Port UART nie jest otwarty.")
            time.sleep(0.5)
        except Exception as e:
            print(f"Błąd podczas wysyłania wiadomości: {e}")
            break

# Funkcja parsująca odebrane dane
def parse_received_data(data):
    try:
        data_dict = {}
        for item in data.split(','):
            key, value = item.split('=')
            data_dict[key.strip()] = float(value.strip())
        return data_dict
    except Exception as e:
        print(f"Błąd podczas parsowania danych: {e}")
        return {}

# Funkcja zapisująca dane w formacie słownika
def save_received_data(data):
    try:
        parsed_data = parse_received_data(data)
        if parsed_data:
            saved_data = {
                "V": parsed_data.get("V", 0.0),
                "C1": parsed_data.get("C1", 0.0),
                "N1": parsed_data.get("N1", 0.0),
                "C2": parsed_data.get("C2", 0.0),
                "N2": parsed_data.get("N2", 0.0),
                "C3": parsed_data.get("C3", 0.0),
                "N3": parsed_data.get("N3", 0.0),
                "P": parsed_data.get("P", 0.0),
                "E": parsed_data.get("E", 0.0)
            }
            print(f"Dane zostały zapisane: {saved_data}")
            return saved_data
        else:
            print("Brak danych do zapisania.")
            return None
    except Exception as e:
        print(f"Błąd podczas zapisywania danych: {e}")
        return None

# Funkcja odbierająca dane z STM32
def receive_message():
    global received_data_from_stm
    while running:
        try:
            if ser.is_open:
                with lock:
                    line = ser.readline().decode('utf-8').strip()
                    if line:
                        print(f"Odebrano dane: {line}")
                        processed_data = save_received_data(line)
                        if processed_data:
                            received_data_from_stm = processed_data
                        else:
                            print("Nie udało się przetworzyć odebranych danych.")
            else:
                print("Port UART nie jest otwarty.")
            time.sleep(0.1)
        except Exception as e:
            print(f"Błąd podczas odbierania wiadomości: {e}")

# Funkcja zapisująca dane do MySQL
def get_connection_to_SQL(data_to_stm):
    global connection, received_data_from_stm
    stan_zapisany = False
    garaz_zapisany = False
    pojazd_zapisany = False

    direction_mapping = {
        "A": ("Przód", "Przód", "Przód", "Przód", "Przód"),
        "a": ("Przód", "Przód", "Przód", "Przód", "Przód"),
        "E": ("Obrót w prawo", "Przód", "Tył", "Przód", "Tył"),
        "e": ("Obrót w prawo", "Przód", "Tył", "Przód", "Tył"),
        "f": ("Obrót w lewo", "Tył", "Przód", "Tył", "Przód"),
        "F": ("Obrót w lewo", "Tył", "Przód", "Tył", "Przód"),
        "b": ("Tył", "Tył", "Tył", "Tył", "Tył"),
        "B": ("Tył", "Tył", "Tył", "Tył", "Tył"),
        "c": ("Lewo", "Tył", "Przód", "Przód", "Tył"),
        "C": ("Lewo", "Tył", "Przód", "Przód", "Tył"),
        "D": ("Prawo", "Przód", "Tył", "Tył", "Przód"),
        "d": ("Prawo", "Przód", "Tył", "Tył", "Przód")
    }

    pojazd_mapping = {
        "A": "1",
        "a": "1",
        "e": "4",
        "E": "4",
        "f": "3",
        "F": "3",
        "b": "2",
        "B": "2",
        "c": "5",
        "C": "5",
        "d": "6",
        "D": "6"
    }

    while running:
        try:
            if connection and connection.is_connected():
                with lock:
                    if received_data_from_stm:
                        cursor = connection.cursor()
                        data = received_data_from_stm
                        command = data_to_stm.get('command', '0')  # Pobierz komendę z data_to_stm
                        print(f"Zapisuję dane do MySQL: {command}")

                        # Dodanie rekordu do tabeli Garaz
                        if not garaz_zapisany:
                            cursor.execute("""
                                INSERT INTO Garaz (Nazwa, Adres, kod_pocztowy, Miasto)
                                VALUES (%s, %s, %s, %s)
                            """, ("Politechnika", "Wincentego Pola", "35-021", "Rzeszów"))
                            garaz_zapisany = True

                        # Dodanie rekordu do tabeli Pojazdy
                        if not pojazd_zapisany:
                            cursor.execute("""
                                INSERT INTO Pojazdy (Nazwa, ID_garazu)
                                VALUES (%s, LAST_INSERT_ID())
                            """, ("Samochodzik",))
                            pojazd_zapisany = True

                        kierunek_pojazdu = pojazd_mapping.get(command, "Nieznany")
                        # Zapis do tabeli Pomiary
                        cursor.execute("""
                            INSERT INTO Pomiary (ID_stanu, ID_pojazdu, Predkosc, Prad_K1, Napiecie_K1, Prad_K2, Napiecie_K2, Prad_K3, Napiecie_K3, Moc, Energia)
                            VALUES (%s,1, %s, %s, %s, %s, %s, %s, %s, %s, %s)
                        """, (
                            kierunek_pojazdu,data["V"], data["C1"], data["N1"], data["C2"], 
                            data["N2"],data["C3"], data["N3"], data["P"], data["E"]
                        ))

                        """# Zapis do tabeli Stan tylko raz
                        
                        stan = 'ruch' if command != '0' else 'postoj'
                        cursor.execute("""
                            #INSERT INTO Stan (ID_pojazdu, stan, godzina, data)
                            #VALUES (%s, %s, CURRENT_TIME(), CURRENT_DATE())
                        """, (1, stan))  # ID_pojazdu na sztywno, można zmienić na dynamiczne
                        stan_zapisany = True"""
                        
                        connection.commit()
                        cursor.close()
                        print("Dane zostały zapisane do bazy.")
                    else:
                        print("Brak danych do zapisania w MySQL.")
            else:
                print("Połączenie z bazą danych nie jest aktywne.")
        except mysql.connector.Error as err:
            print(f"Błąd podczas zapisywania danych do MySQL: {err}")
        time.sleep(1)

# Zamknięcie portu UART
def close_port():
    global ser
    if ser and ser.is_open:
        ser.close()
        print("Port UART został zamknięty.")
    else:
        print("Port UART już był zamknięty.")

# Funkcja do zamykania połączenia MySQL
def close_connection_to_SQL():
    global connection
    try:
        if connection and connection.is_connected():
            connection.close()
            print("Połączenie z bazą danych zostało zamknięte.")
    except mysql.connector.Error as err:
        print(f"Błąd podczas zamykania połączenia z bazą danych: {err}")
