import time
import os
import sys
import cv2
from flask import Flask, Response
from edge_impulse_linux.image import ImageImpulseRunner

# Flask app
app = Flask(__name__)

# Globalne zmienne
camera = None
runner = None
bbox_data = {}

###
def initialize_system(model_file="/home/ubuntu/Desktop/linefollower/modelfile.eim", cam_width=320, cam_height=320):
    """
    Inicjalizuje kamerę, model Edge Impulse i przygotowuje system do pracy.
    
    Args:
        model_file (str): Ścieżka do pliku modelu Edge Impulse.
        cam_width (int): Szerokość obrazu kamery.
        cam_height (int): Wysokość obrazu kamery.
    """
    global camera, runner, bbox_data

    print("Rozpoczynam inicjalizację systemu...")

    # Inicjalizacja modelu Edge Impulse
    runner = ImageImpulseRunner(model_file)
    runner.init()
    print("Model Edge Impulse został pomyślnie zainicjalizowany.")

    # Inicjalizacja kamery
    camera = cv2.VideoCapture(0)
    camera.set(cv2.CAP_PROP_FRAME_WIDTH, cam_width)
    camera.set(cv2.CAP_PROP_FRAME_HEIGHT, cam_height)

    if not camera.isOpened():
        raise Exception("Nie udało się otworzyć kamery.")
    print("Kamera została pomyślnie zainicjalizowana.")


    # Inicjalizacja pustego bbox_data
    bbox_data = {
        "label": None,
        "confidence": 0.0,
        "x": 0,
        "y": 0,
        "width": 0,
        "height": 0
    }

    print("System został pomyślnie zainicjalizowany i jest gotowy do pracy.")
###

# Function to get bounding box data
def get_bbox_data():
    return bbox_data

# Włączenie najpierw strony powitalnej z linkiem do obrazu z kamery w przeglądarce
'''
@app.route('/')
def index():
    return "Witaj! Przejdź do <a href='/video_feed'>/video_feed</a> aby zobaczyć strumień wideo."

@app.route('/video_feed')
def video_feed():
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')
'''

# Włączenie bezpośrednio obrazu z kamery w przeglądarce
@app.route('/')
def index():
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')
    
def start_flask():
    """
    Uruchamia serwer Flask w osobnym wątku.
    """
    try:
        app.run(host="0.0.0.0", port=5000, debug=False)
    except Exception as e:
        print(f"Błąd w serwerze Flask: {e}")


"""
Funkcja do pobierania danych z kamery w przeglądarce
@app.route('/get_bbox_data')
def get_bbox():
    data = get_bbox_data()
    return {
        "label":        data.get("label", None),
        "confidence":   data.get("confidence", 0.0),
        "x":            data.get("x", 0),
        "y":            data.get("y", 0),
        "width":        data.get("width", 0),
        "height":       data.get("height", 0)
    }
"""

# Funkcja do pobierania danych z kamery
def get_bbox():
    """
    Pobiera dane z get_bbox_data i zwraca w uporządkowanym formacie.
    """
    data = get_bbox_data()
    
    # Zabezpieczenie przed brakującymi kluczami
    return {
        "label":        data.get("label", None),
        "confidence":   data.get("confidence", 0.0),
        "x":            data.get("x", 0),
        "y":            data.get("y", 0),
        "width":        data.get("width", 0),
        "height":       data.get("height", 0)
    }

def generate_frames():
    global fps, bbox_data
    fps_limit = 10
    frame_counter = 0  # Counter to track frames
    next_frame_time = time.time()  # Timestamp for controlling FPS
    last_frame_time = time.time()  # Timestamp for measuring FPS
    
    while True:
        current_time = time.time()

        # Skip frames to maintain target FPS
        if current_time < next_frame_time:
            time.sleep(next_frame_time - current_time)
            continue

        # Schedule next frame
        next_frame_time += 1 / fps_limit

        # Capture and process frame
        ret, img = camera.read()
        if not ret:
            print("ERROR: Failed to capture image.")
            break

        frame_counter += 1

        if frame_counter % 3 == 0:  # Infer every 3rd frame
            img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
            features, _ = runner.get_features_from_image(img_rgb)

            try:
                res = runner.classify(features)
                bboxes = res['result'].get('bounding_boxes', [])
                 
                if bboxes:  # If there are detected objects
                    for bbox in bboxes: 
                        if bbox['label'] == 'tennis-ball':  # Sprawdzanie, czy etykieta to 'tennis-ball'
                            best_bbox = max(bboxes, key=lambda bbox: bbox['value'])
                            
                            # Update global bbox_data
                            bbox_data.update({
                                "label":      best_bbox['label'],
                                "confidence": best_bbox['value'],
                                "x":          best_bbox['x'],
                                "y":          best_bbox['y'],
                                "width":      best_bbox['width'],
                                "height":     best_bbox['height']
                            })

                            # Draw bounding box for the highest-confidence object
                            b_x0, b_y0 = best_bbox['x'], best_bbox['y']  
                            b_x1, b_y1 = best_bbox['x'] + best_bbox['width'], best_bbox['y'] + best_bbox['height']
                            print('\t%s (%.2f): x=%d y=%d w=%d h=%d' % (best_bbox['label'], best_bbox['value'], best_bbox['x'], best_bbox['y'], best_bbox['width'], best_bbox['height']))
                            cv2.rectangle(img, (b_x0, b_y0), (b_x1, b_y1), (255, 255, 255), 1)
                            cv2.putText(img, f"{best_bbox['label']}: {round(best_bbox['value'], 2)}",
                                        (b_x0, b_y0 + 12), cv2.FONT_HERSHEY_PLAIN, 1, (255, 255, 255))
                
                # If no objects are detected                 
                if not bboxes:
                    bbox_data.update({
                        "label": 'brak',
                        "confidence": 0.0,
                        "x": 0,
                        "y": 0,
                        "width": 0,
                        "height": 0
                    })
                    print("Nie wykrytooo piłki tenisowej.")
                    #else:
                    #    print("Nie wykryto piłkiii tenisowej.")

            except Exception as e:
                print("ERROR during inference:", e)   
            except Exception as e:
                print(f"Błąd w generate_frames: {e}")
                break
            except BrokenPipeError:
                print("Broken pipe: Klient zamknął połączenie.")
                break

        # Measure time taken for the frame
        current_frame_time = time.time()
        frame_time = current_frame_time - last_frame_time
        last_frame_time = current_frame_time

        # Calculate FPS
        fps = 1 / frame_time if frame_time > 0 else 0
        #cv2.putText(img, f"FPS: {round(fps, 2)}", (0, 12), cv2.FONT_HERSHEY_PLAIN, 1, (255, 255, 255))
        
        # Encode frame as JPEG
        ret, jpeg = cv2.imencode('.jpg', img)
        if not ret:
            continue
        frame = jpeg.tobytes()
        
        # Yield frame in MJPEG format
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

"""
# Funkcja do zatrzymywania serwera Flask

@app.route('/shutdown', methods=['POST'])
def shutdown():
    func = request.environ.get('werkzeug.server.shutdown')
    if func is None:
        raise RuntimeError("Nie można zatrzymać serwera Flask.")
    func()
    return "Serwer Flask został zatrzymany."
"""

# Funkcja do zamykania zasobów
def close_resources():
    """
    Zamyka kamerę i model Edge Impulse.
    """
    global camera, runner
    if camera is not None:
        camera.release()
        print("Kamera została zamknięta.")
    if runner is not None:
        runner.stop()
        print("Model Edge Impulse został zamknięty.")
    cv2.destroyAllWindows()
    print("Zasoby zostały pomyślnie zamknięte.")
    
