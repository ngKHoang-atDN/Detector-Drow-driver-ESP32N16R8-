# Driver Drowsiness Detection System Using ESP32 + Laptop Webcam

## Overview
This project uses an ESP32 board as a sensor node for ECG signal acquisition. The ECG data is processed to extract HRV features, and a machine learning model is used to estimate sleepiness or drowsiness. A laptop webcam is used for computer vision to monitor the driver’s face and eyes in real time.

## Project Goal
The goal is to create a low-cost driver monitoring system that:
- reads ECG signals using ESP32 [x]
- extracts HRV features such as RMSSD, SDNN, and heart rate variability [x]
- uses machine learning to estimate sleepiness or drowsiness 'x
- uses the laptop webcam to monitor facial cues such as eye closure and head pose [!]
- warns the driver when drowsiness is detected [x]

## Key Features
- ECG data acquisition using ESP32
- HRV feature extraction from ECG signals
- Machine learning-based sleepiness/drowsiness estimation
- Wireless data transmission to a laptop over Wi-Fi
- Real-time face and eye tracking using a laptop webcam
- Alert system through laptop sound/notification and optional buzzer

## Hardware Requirements
- ESP32-S3 N16R8 (or another ESP32 board)
- ECG sensor module (for example AD8232)
- Laptop with built-in webcam (or an external camera if needed)
- Power supply and jumper wires
- Optional buzzer or LED for extra alerts

## Software Stack
- PlatformIO
- Arduino framework
- Python
- OpenCV
- Scikit-learn / TensorFlow / PyTorch for machine learning
- Wi-Fi communication between ESP32 and laptop

## How It Works
1. The ESP32 reads ECG data from the sensor.
2. The ESP32 sends the ECG data to the laptop over Wi-Fi (or cable) .
3. The laptop extracts HRV features from the ECG signal.
4. A machine learning model analyzes the HRV values to estimate sleepiness or drowsiness.
5. The laptop webcam monitors the driver’s face and eyes for visual confirmation.
6. If drowsiness is detected, the system triggers an alert.


## Project Structure
- `src/` - ESP32 firmware for ECG sensor reading and Wi-Fi communication
- `vision/` - Python/OpenCV drowsiness detection scripts
- `ml/` - Machine learning model training and inference code
- `docs/` - Documentation and notes
- `include/` - Header files

## Setup Instructions
1. Connect the ECG sensor to the ESP32.
2. Upload the ESP32 firmware using PlatformIO.
3. Run the Python program on the laptop to receive ECG data.
4. Extract HRV features and run the machine learning model.
5. Start the webcam-based monitoring and alerts.

## Usage
- Place the ECG sensor on the driver.
- Start the ESP32 device.
- Start the laptop monitoring program.
- The system will estimate drowsiness using ECG-based HRV and webcam-based visual analysis.

## Future Improvements
- Improve the accuracy of the ML model with more real-world data
- Combine ECG-based HRV with facial features for better prediction
- Add cloud or mobile monitoring
- Use a buzzer or LED as a stronger warning signal

## Conclusion
This cost-effective setup combines ESP32-based ECG sensing, HRV analysis, machine learning, and laptop webcam computer vision to create a practical driver sleepiness and drowsiness detection system.