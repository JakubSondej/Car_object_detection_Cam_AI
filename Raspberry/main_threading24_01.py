import threading
import time
from cameraAI           import get_bbox_data, initialize_system, start_flask, close_resources, generate_frames 
from sql_&_stm          import initialize_uart,send_message,receive_message,close_port,  received_data_from_stm,   initialize_mysql, get_connection_to_SQL, close_connection_to_SQL
from hc05_bluotooth     import initialize_uart_hc, uart_hc_processing, close_uart_hc, get_phone_box
import requests



""" Logika wysyłana do STM32
direction_stm: do zmiany neutral nie musi go byc
data_to_stm = {
    0- stop , a - slow forward, A - fast forward, b - slow backward, B - fast backward, c - around slow left, C - around fast left, d - around slow right, D - around fast right, e - left straight slow, E - left straight fast, f - right straight slow, F - right straight fast
}
    """

# Globalne dane
bbox_data = {}
data_to_stm = {}
#drive_mode = "manual"  # Domyślny tryb
drive_mode = "auto"    # Tryb automatyczny
phone_box = {}  # Domyślne wartoścSi
received_data_from_stm = {}
saved_data = {}  # Zapisane dane do SQL

# Lock do synchronizacji dostępu
lock = threading.Lock()

running = True

# Funkcja wątku dla kamery (użycie generate_frames)
def camera_processing():
    global running
    frame_generator = generate_frames()  # Generator ramek z kamery
    while running:
        try:
            next(frame_generator)  # Pobierz kolejną ramkę
        except StopIteration:
            print("Generator ramek zakończył działanie.")
            break
        except Exception as e:
            print(f"Błąd w wątku kamery: {e}")
            break


# Funkcja wątku dla logiki sterowania do STM
def control_processing():
    """
    Funkcja wątku do przetwarzania logiki sterowania dla STM32.
    """
    global data_to_stm, bbox_data, running, previous_bbox_data, drive_mode

    previous_bbox_data = None  # Zainicjowanie zmiennej do przechowywania poprzednich danych

    while running:
        try:
            # Pobranie nowych danych z kamery
            new_data = get_bbox_data()  # Zakładamy, że funkcja get_bbox_data() zwraca dane
            if new_data != previous_bbox_data:
                previous_bbox_data = new_data  # Zapis nowego stanu
                print(f"Otrzymano nowe dane: {new_data}")

            # Podstawienie bieżących danych do zmiennej bbox_data
            bbox_data = previous_bbox_data

            # Tryb manualny
            if drive_mode == "manual":
                phone_data = get_phone_box()

                # Walidacja danych z telefonu
                if not phone_data or not isinstance(phone_data.get('direction'), (int, float)) or not isinstance(phone_data.get('velocity'), (int, float)):
                    print("Błąd danych z telefonu! Nie można wysłać komendy.")
                    command = "0"  # Domyślny znak w przypadku błędu
                else:
                    print(f"PozX: {phone_data.get('poz_x', 0)}, PozY: {phone_data.get('poz_y', 0)}, "
                        f"Velocity: {phone_data.get('velocity')}, Direction: {phone_data.get('direction')}, Around: {phone_data.get('around')}")

                    # Przypisanie danych do zmiennych
                    direction = phone_data.get('direction', 0)
                    velocity = phone_data.get('velocity', 0)
                    around = phone_data.get('around', 'neutral')

                    # Logika wyboru komendy
                    if around == "left":
                        command = "C"  # Obrót w lewo
                        print(f"Komenda: Obrót w lewo ({command})")
                    elif around == "right":
                        command = "D"  # Obrót w prawo
                        print(f"Komenda: Obrót w prawo ({command})")
                    elif direction != 0:
                        if -45 < direction < 45:
                            command = "A" if velocity > 50 else "a"
                            print(f"Komenda: Jedź prosto ({command})")
                        elif 45 <= direction <= 135:
                            command = "E" if velocity > 50 or velocity < -50 else "e"
                            print(f"Komenda: Jedź prosto w prawo ({command})")
                        elif -135 <= direction <= -45:
                            command = "F" if velocity > 50 or velocity < -50 else "f"
                            print(f"Komenda: Jedź prosto w lewo ({command})")
                        elif direction >= 135 or direction <= -135:
                            command = "B" if velocity < -50 else "b"
                            print(f"Komenda: Jedź do tyłu ({command})")
                        else:
                            command = "0"  # Domyślna komenda w przypadku niejasnych danych
                            print("Nieokreślony manewr dla podanych danych kierunku i prędkości.")
                    else:
                        command = "0"  # Zatrzymanie w przypadku braku prędkości
                        print("Komenda: Stop")

                # Aktualizacja `data_to_stm` i wysyłanie komendy do STM32
                with lock:
                    data_to_stm.update({"command": command})
                print(f"Wysłano komendę: {command}")
    
            # Tryb automatyczny
            if drive_mode == "auto":
                if bbox_data.get('label') == "tennis-ball" and bbox_data.get('confidence', 0) > 0.54:
                    if bbox_data['x'] <= 55 and bbox_data['width'] <= 120 and bbox_data['height'] <= 120:
                        command = "f"
                        print("Skret w lewo prosto")
                    elif bbox_data['x'] >= 200 and bbox_data['width'] <= 120 and bbox_data['height'] <= 120:
                        command = "e"
                        print("Skret w prawo prosto")
                    else:
                        if bbox_data['width'] >= 120 or bbox_data['height'] >= 120:
                            command = "0"
                            print("Stop")
                        elif bbox_data['width'] <= 40 and bbox_data['height'] <= 40:
                            command = "A"
                            print("Jedź prosto szybko")
                        elif bbox_data['width'] > 40 and bbox_data['height'] > 40:
                            command = "a"
                            print("Jedź prosto wolno")
                        else:
                            command = "0"
                            print("Stopp")
                elif bbox_data['label'] == 'brak':
                        command = "D" # Obrót w prawo
                        print("Obrót w prawoooo")
                
                else:
                    command = "0"  # Obrót w prawo
                    print("NieWiemCoRobie Stop")

                # Wysyłanie pojedynczego znaku do STM32
                data_to_stm.update({"command": command})
                print(f"Wysłano komendę: {command}")

            # Opcjonalne opóźnienie, aby ograniczyć częstotliwość iteracji.
            time.sleep(0.1)

        except KeyError as e:
            print(f"Błąd: brak klucza w danych bbox_data: {e}")
        except Exception as e:
            print(f"Wystąpił nieoczekiwany błąd: {e}")


def main():
    global running, received_data_from_stm
    received_data_from_stm = {}
    camera_thread   = None
    flask_thread    = None
    control_thread  = None
    uart_thread     = None
    sql_thread      = None
    hc_thread       = None
    receive_thread  = None

    try:
        #Inicjalizacja systemu
        initialize_system()
        initialize_uart()
        '''if not (initialize_uart()): # Czy nawiasy są dobrze???
            print("Nie udało się zainicjalizować UART. Program zostaje zakończony.")
            return'''
        
        initialize_mysql()
        initialize_uart_hc()
        
        # Tworzenie wątków
        camera_thread   = threading.Thread(target=camera_processing)
        flask_thread    = threading.Thread(target=start_flask)
        control_thread  = threading.Thread(target=control_processing)
        uart_thread     = threading.Thread(target=send_message, args=(data_to_stm,))
        sql_thread      = threading.Thread(target=get_connection_to_SQL, args=(data_to_stm,))
        hc_thread       = threading.Thread(target=uart_hc_processing)
        receive_thread  = threading.Thread(target=receive_message)
        #uart_thread     = threading.Thread(target=send_message)
        #sql_thread      = threading.Thread(target=get_connection_to_SQL)

        # Startowanie wątków
        camera_thread.start()
        flask_thread.start()
        control_thread.start()
        uart_thread.start()
        sql_thread.start()
        hc_thread.start()
        receive_thread.start()

        # Główna pętla programu
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("Program zatrzymany przez użytkownika.")
        running = False  # Zatrzymaj wątki
        return
    except Exception as e:
        print(f"Błąd w programie: {e}")
        running = False
        return
    finally:
        print("Zamykanie zasobów...")
        running = False

        # Zatrzymywanie wątków (jeśli istnieją)
        if camera_thread is not None:
            camera_thread.join()
        if flask_thread is not None:
            flask_thread.join()
        if control_thread is not None:
            control_thread.join()
        if uart_thread is not None:
            uart_thread.join()
        if sql_thread is not None:
            sql_thread.join()
        if hc_thread is not None:
            hc_thread.join()
        if receive_thread is not None:
            receive_thread.join()
       
        close_resources()         #Zamknięcie kamery, flaska
        close_port()              #UART close
        close_connection_to_SQL() #Zamknięcie połączenia z bazą danych
        close_uart_hc()           #Zamknięcie portu UART HC`
        print("Zakończono działanie programu.")

if __name__ == "__main__":
    main()
