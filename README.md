"The idea behind this project was to use existing knowledge from various fields to create a larger project"

# AI-Powered Mobile Vehicle for Object Detection  

This project presents a mobile vehicle controlled by an **STM32 microcontroller** and a **Raspberry Pi 4**, using an **AI model** to detect a tennis ball and navigate autonomously.  
![1 (2)](https://github.com/user-attachments/assets/4f17d9a7-0ca0-4be2-9d32-d2b4f99cb9cf)

## 🛠 Build  
- **Electronics**: Raspberry Pi4 4GB, STM32 L152RE, INA3221, camera logitech C165, HC-05 Bluetooth 
- **Construction**: Lego Technic and 3D-printed components.  
- **Drive system**: BLDC motors with encoders and omnidirectional wheels.  
- **Power supply**: 2S2P **18650 battery pack** with a BMS circuit.  
- **Communication**:
![aasd](https://github.com/user-attachments/assets/6bea847d-5e31-4e62-9705-56f43df58f70)

  - **UART**:      STM32 ↔️ RPi  
  - **Bluetooth**: Phone control  ↔️ Raspberry Pi 4 4GB
  - **I2C**:       Power monitoring ↔️ STM32

## 🤖 AI and Object Detection  

![22](https://github.com/user-attachments/assets/79a115d1-6867-4e7a-b19e-829e43556b24)

- AI model trained using **Edge Impulse** (**MobileNetV2 SSD FPN-Lite**).  
- Detects the ball and determines its position and size.  
- Video streaming via a **Flask server**.  

## 🔗 Architecture & Control  
- **Manual Mode** – controlled via a phone over Bluetooth.  
- **Autonomous Mode** – the vehicle scans the environment and follows the ball.  
- Data is stored in a **MySQL database** on the Raspberry Pi.  

## 🏗 Key Files  
- `cameraAI.py` – Camera handling & AI processing.  
- `main_threading.py` – Thread management for vehicle control.  
- `sql_&_stm.py` – Communication with **STM32** & database.  
- `hc05_bluotooth.py` – Bluetooth HC-05 module control.  
- `bib.h`, `INA3221.h` – Motor & sensor control on **STM32**.  

## 📊 Data Visualization  
Sensor data is stored in the database and visualized using **Jupyter Notebook**.  

🎯 **Project Goal**: Practical AI implementation in a mobile system and integration of multiple technologies into a single solution.  

