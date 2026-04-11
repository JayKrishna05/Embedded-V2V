import pandas as pd
import numpy as np
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_absolute_error
import matplotlib.pyplot as plt

# Load dataset
data = pd.read_csv("data.csv")

X = data[['RSSI']]
y = data['Distance']

# Train model
model = RandomForestRegressor(n_estimators=200)
model.fit(X, y)

# Predictions
y_pred = model.predict(X)

# -----------------------------
# Log-distance model
# -----------------------------
A = -58  # RSSI at 1m
n = 2.5  # path loss exponent

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
# Polynomial fit
# -----------------------------
coeffs = np.polyfit(data['RSSI'], y_pred, 2)
print("\nPolynomial Coefficients:")
print(coeffs)

# -----------------------------
# Create table
# -----------------------------
table = pd.DataFrame({
    'RSSI (dBm)': data['RSSI'],
    'Actual Distance (m)': y,
    'Log Model (m)': log_pred,
    'ML Model (m)': y_pred
})

# Round values for neatness
table = table.round(2)

print("\n=== ML vs Log Table ===")
print(table.to_string(index=False))

# Optional: Save table to CSV
table.to_csv("comparison_table.csv", index=False)

# -----------------------------
# Plot graph
# -----------------------------
plt.figure()

plt.scatter(data['RSSI'], y, label="Actual")
plt.plot(data['RSSI'], y_pred, label="ML Prediction")
plt.plot(data['RSSI'], log_pred, label="Log Model")

plt.legend()
plt.xlabel("RSSI (dBm)")
plt.ylabel("Distance (m)")
plt.title("ML vs Log Model")

plt.grid()
plt.show()