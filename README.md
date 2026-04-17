# ESP32 V2V Collision Warning System (RSSI + ML)

A low-cost, real-time Vehicle-to-Vehicle (V2V) collision warning system built using ESP32, leveraging WiFi RSSI, Kalman filtering, and a lightweight machine learning model for distance estimation.



## Features

- RSSI-based distance estimation  
- ML-enhanced distance prediction (Random Forest → Polynomial)  
- Kalman filtering for noise reduction  
- Time-to-Collision (TTC) based alerts  
- OLED display and buzzer feedback  
- Real-time multitasking using FreeRTOS  
- Fully edge-based system (no cloud dependency)  

## System Pipeline


RSSI → ML / Log Distance → Kalman Filter → TTC → Alert


## Project Structure



├── esp32_v2v.ino          # ESP32 firmware

├── train_model.py         # ML training and evaluation

├── data.csv               # RSSI-distance dataset

├── comparison_table.csv   # Generated evaluation results

├── README.md






## ESP32 Firmware (`.ino`)

### Description

The ESP32 firmware performs the following operations:

- Acquires RSSI from WiFi signals  
- Estimates distance using both log-distance and ML models  
- Applies Kalman filtering for noise reduction  
- Computes Time-to-Collision (TTC)  
- Generates alerts using OLED display and buzzer  


### Requirements

- ESP32 development board  
- Arduino IDE or PlatformIO  

**Libraries:**
- WiFi  
- WebServer  
- Adafruit SSD1306  
- FreeRTOS (built-in with ESP32)  


### Setup

1. Open the `.ino` file in Arduino IDE  
2. Select the ESP32 board  
3. Install required libraries  
4. Upload the code  


### Key Function (ML Distance Estimation)

```cpp
float predict_distance(float rssi) {
  float a = 0.00867766;
  float b = 0.85337;
  float c = 20.5131;

    float distance = a * rssi * rssi + b * rssi + c;
    return distance;
}
```
## RTOS Tasks

| Task Name     | Functionality                          |
|--------------|----------------------------------------|
| vTaskProcess | RSSI → Distance → Kalman → TTC         |
| vTaskComm    | HTTP communication and data exchange   |
| vTaskDisplay | OLED rendering and buzzer alerts       |


## ML Training Script (`train_model.py`)

### Description

This script:

- Trains a Random Forest regression model  
- Compares ML predictions with the log-distance model  
- Computes error metrics (Mean Absolute Error)  
- Generates a comparison table  
- Plots RSSI vs distance graph  
- Extracts a polynomial approximation for embedded deployment  


### Requirements

```bash
pip install pandas numpy scikit-learn matplotlib
```
## Output

- **Console:**
  - ML error vs Log model error  

- **Graph:**
  - RSSI vs Distance comparison  

- **File:**
  - `comparison_table.csv`  


## Model Details

- Model: Random Forest Regressor  
- Number of trees: 200  
- Input: RSSI  
- Output: Distance  


Hybrid Distance Estimation (Model Switching)


To improve accuracy across all ranges, the system uses a hybrid approach that switches between models based on RSSI.


Switching Strategy


| RSSI Range           | Model Used              | Region        |
|----------------------|------------------------|--------------|
| RSSI > -55 dBm       | Log Model              | Close range  |
| -60 ≤ RSSI ≤ -55 dBm | Interpolation (Blend)  | Transition   |
| RSSI < -60 dBm       | Polynomial Model       | Far range    |






### Implementation Logic

```cpp
float predict_distance(float rssi) {

  rssi = constrain(rssi, -100, -20);

  float A = -58;
  float n = 2.5;
  float d_log = pow(10.0, (A - rssi) / (10.0 * n));

  float a = 0.00867766;
  float b = 0.85337;
  float c = 20.5131;
  float d_poly = a * rssi * rssi + b * rssi + c;

  if (rssi > -55) return d_log;
  if (rssi < -60) return d_poly;

  float t = (rssi + 60) / 5.0;
  return t * d_log + (1 - t) * d_poly;
}
```
## Benefits

- Accurate at both short and long distances using a **hybrid model (log + polynomial)**  
- Eliminates polynomial instability at close range through **model switching**  
- Smooth interpolation in the transition region prevents sudden jumps  
- Improves TTC reliability in real-time  


## Results

| Model                | Performance                              |
|---------------------|------------------------------------------|
| Log-distance        | Accurate at close range, high error far  |
| Polynomial (ML)     | Accurate at far range, unstable near     |
| Hybrid (Proposed)   | Consistent accuracy across all ranges    |

The hybrid model significantly improves distance estimation accuracy by combining the strengths of both physical and ML-based approaches.


## Dataset

The dataset consists of RSSI-distance pairs collected under:

- Line-of-Sight (LOS)  
- Non-Line-of-Sight (NLOS)  

**Distance range:** 0.25 m to 25 m  


## Key Observations

- Real-time performance (< 200 ms latency)  
- Hybrid model reduces estimation error compared to standalone models  
- Stable under fluctuating RSSI conditions due to filtering and model switching  
- Works efficiently on resource-constrained hardware  


## Future Work

- Multi-node localization  
- Temporal ML models using RSSI history  
- Integration with UWB or GPS  
- Adaptive model retraining  
