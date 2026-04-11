
ESP32 V2V Collision Warning System (RSSI + ML)
A low-cost, real-time Vehicle-to-Vehicle (V2V) collision warning system built using ESP32, leveraging WiFi RSSI, Kalman filtering, and a lightweight machine learning model for distance estimation.
Features
RSSI-based distance estimation
ML-enhanced distance prediction (Random Forest → Polynomial)
Kalman filtering for noise reduction
Time-to-Collision (TTC) based alerts
OLED display and buzzer feedback
Real-time multitasking using FreeRTOS
Fully edge-based system (no cloud dependency)
System Pipeline
RSSI → ML / Log Distance → Kalman Filter → TTC → Alert

Project Structure
├── esp32_v2v.ino          # ESP32 firmware
├── train_model.py         # ML training and evaluation
├── data.csv               # RSSI-distance dataset
├── comparison_table.csv   # Generated evaluation results
├── README.md

ESP32 Firmware (.ino)
Description
The ESP32 firmware performs the following operations:
Acquires RSSI from WiFi signals
Estimates distance using both log-distance and ML models
Applies Kalman filtering for noise reduction
Computes Time-to-Collision (TTC)
Generates alerts using OLED display and buzzer
Requirements
ESP32 development board
Arduino IDE or PlatformIO
Required libraries:
WiFi
WebServer
Adafruit SSD1306
FreeRTOS (built-in with ESP32)
Setup
Open the .ino file in Arduino IDE
Select the ESP32 board
Install required libraries
Upload the code to the ESP32
Key Function (ML Distance Estimation)
float predict_distance(float rssi) {
    float a = 0.009403;
    float b = 1.0079;
    float c = 27.6746;

    float distance = a*rssi*rssi + b*rssi + c;
    return distance;
}

RTOS Tasks
Task NameFunctionalityvTaskProcessRSSI → Distance → Kalman → TTCvTaskCommHTTP communication and data exchangevTaskDisplayOLED rendering and buzzer alertsML Training Script (train_model.py)
Description
This script:
Trains a Random Forest regression model
Compares ML predictions with the log-distance model
Computes error metrics (Mean Absolute Error)
Generates a comparison table
Plots RSSI vs distance graph
Extracts a polynomial approximation for embedded deployment
Requirements
Install dependencies:
pip install pandas numpy scikit-learn matplotlib

Usage
python train_model.py

Output
Console output:
ML error vs Log model error
Graph:
RSSI vs Distance comparison
File:
comparison_table.csv
Model Details
Model: Random Forest Regressor
Number of trees: 200
Input: RSSI
Output: Distance
Polynomial Approximation
The trained ML model is approximated as:
d = 0.009403*rssi^2 + 1.0079*rssi + 27.6746

This enables efficient real-time execution on ESP32 without requiring heavy ML inference.
Results
ModelPerformanceLog-distanceHigh error at long distancesML modelLower error, improved stability
The ML model significantly improves distance estimation accuracy, particularly in noisy and non-linear conditions.
Dataset
The dataset consists of RSSI-distance pairs collected under:
Line-of-Sight (LOS)
Non-Line-of-Sight (NLOS)
Distance range: 0.5 m to 14 m
Key Observations
Real-time performance achieved (< 200 ms latency)
ML reduces estimation error compared to analytical models
System remains stable under fluctuating RSSI conditions
Fully functional on resource-constrained hardware
Future Work
Multi-node localization
Temporal ML models using RSSI history
Integration with UWB or GPS
Adaptive model retraining
Author
Jay Krishna Kamlekar
ESP32 V2V Collision Detection Project
If you want, I can also:
generate a short GitHub repo description (one line)
or help you make this look stronger for your resume (very useful)
