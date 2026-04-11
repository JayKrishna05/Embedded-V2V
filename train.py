import pandas as pd
import numpy as np
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_absolute_error
import matplotlib.pyplot as plt

# -----------------------------
# Load dataset
# -----------------------------
data = pd.read_csv("data.csv")

X = data[['RSSI']]
y = data['Distance']

# -----------------------------
# Train Random Forest
# -----------------------------
model = RandomForestRegressor(n_estimators=200, random_state=42)
model.fit(X, y)

# Predictions on training data
y_pred = model.predict(X)

# -----------------------------
# Log-distance model
# -----------------------------
A = -58
n = 2.5

def log_model(rssi):
    return 10 ** ((A - rssi) / (10 * n))

log_pred = np.array([log_model(r) for r in data['RSSI']])

# -----------------------------
# Errors
# -----------------------------
ml_error = mean_absolute_error(y, y_pred)
log_error = mean_absolute_error(y, log_pred)

print("\nML Mean Absolute Error:", ml_error)
print("Log Model Error:", log_error)

# -----------------------------
# SMOOTH RF → Polynomial Fit
# -----------------------------
# Create smooth RSSI range
rssi_range = np.linspace(data['RSSI'].min(), data['RSSI'].max(), 200)

# Predict using RF (smooth curve)
rf_smooth = model.predict(rssi_range.reshape(-1, 1))

# Fit polynomial on smooth curve
coeffs = np.polyfit(rssi_range, rf_smooth, 2)

print("\nPolynomial Coefficients (ESP32 ready):")
print("a =", coeffs[0])
print("b =", coeffs[1])
print("c =", coeffs[2])

# Polynomial prediction function
def poly_model(rssi):
    return coeffs[0]*rssi**2 + coeffs[1]*rssi + coeffs[2]

poly_pred = np.array([poly_model(r) for r in data['RSSI']])

# -----------------------------
# Create table
# -----------------------------
table = pd.DataFrame({
    'RSSI (dBm)': data['RSSI'],
    'Actual Distance (m)': y,
    'Log Model (m)': log_pred,
    'RF Model (m)': y_pred,
    'Polynomial Model (m)': poly_pred
})

table = table.round(2)

print("\n=== Comparison Table ===")
print(table.to_string(index=False))

# Save CSV
table.to_csv("comparison_table.csv", index=False)

# -----------------------------
# Plot graph
# -----------------------------
plt.figure()

plt.scatter(data['RSSI'], y, label="Actual", s=30)
plt.plot(rssi_range, rf_smooth, label="RF (Smooth)", linewidth=2)
plt.plot(rssi_range, poly_model(rssi_range), label="Polynomial Approx", linewidth=2)
plt.plot(data['RSSI'], log_pred, label="Log Model", linestyle='--')

plt.xlabel("RSSI (dBm)")
plt.ylabel("Distance (m)")
plt.title("ML vs Log vs Polynomial")
plt.legend()
plt.grid()

plt.show()