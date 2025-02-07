"The idea behind this project was to use existing knowledge from various fields to create a larger project"

# AI-Powered Mobile Vehicle for Object Detection  

This project presents a mobile vehicle controlled by an **STM32 microcontroller** and a **Raspberry Pi 4**, using an **AI model** to detect a tennis ball and navigate autonomously.  

## 🛠 Build  
- **Construction**: Lego Technic and 3D-printed components.  
- **Drive system**: BLDC motors with encoders and omnidirectional wheels.  
- **Power supply**: 2S2P **18650 battery pack** with a BMS circuit.  
- **Communication**:
![aasd](https://github.com/user-attachments/assets/6bea847d-5e31-4e62-9705-56f43df58f70)

  - **UART**:      STM32 ↔️ RPi  
  - **Bluetooth**: Phone control  ↔️ Raspberry Pi
  - **I2C**:       Power monitoring ↔️ STM32

## 🤖 AI and Object Detection  
- AI model trained using **Edge Impulse** (**MobileNetV2 SSD FPN-Lite**).  
- Detects the ball and determines its position and size.  
- Video streaming via a **Flask server**.  

## 🔗 Architecture & Control  
- **Manual Mode** – controlled via a phone over Bluetooth.  
- **Autonomous Mode** – the vehicle scans the environment and follows the ball.  
- Data is stored in a **MySQL database** on the Raspberry Pi.  

## 🏗 Key Files  
- `camera.py` – Camera handling & AI processing.  
- `main_threading.py` – Thread management for vehicle control.  
- `sql_stm.py` – Communication with **STM32** & database.  
- `hc05.py` – Bluetooth HC-05 module control.  
- `stm32_code.c` – Motor & sensor control on **STM32**.  

## 📊 Data Visualization  
Sensor data is stored in the database and visualized using **Jupyter Notebook**.  

🎯 **Project Goal**: Practical AI implementation in a mobile system and integration of multiple technologies into a single solution.  

