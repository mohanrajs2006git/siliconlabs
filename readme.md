# Real-Time Interactive Sensor Dashboard

**A Smart Wearable Sensor Dashboard for Live Monitoring & Alerts**

Monitor acceleration, gyroscope, temperature, humidity, and button inputs in real time with an interactive web dashboard. Designed for wearable IoT devices using the Silicon Labs SiWx917 Dev Kit, the dashboard provides instant insights, trend analysis, and threshold-based alerts.

---

## Key Features

- **Real-Time Sensor Display:** Live values for motion, temperature, humidity, and buttons.  
- **Threshold-Based Alerts:** Color-coded cards and animated alerts for critical readings.  
- **Side-by-Side Value & Status:** Sensor value on left, status/alert on right for clarity.  
- **Trend Visualization:** Mini charts display recent readings for motion and environment.  
- **Dynamic Dashboard Image:** Changes based on sensor alerts for interactive feedback.  
- **Summary Panel:** Quick overview of total alerts, max temperature, and max motion.  
- **Sound Alerts:** Browser notifications for critical sensor events.  

---

## Workflow

### Sensor Data Acquisition
- Data collected from wearable sensors: accelerometer, gyroscope, temperature, humidity, and buttons.

### Data Processing & Threshold Detection
- Each reading is analyzed against predefined thresholds to determine the status (Normal, Alert, High Motion, Hot, etc.).

### Real-Time Dashboard Update
- Values and status are sent to the browser via HTTP.  
- Dashboard cards are dynamically updated with animations, charts, and alert effects.

### Visual & Audio Feedback
- Critical readings trigger card animations, color changes, and optional sound notifications for immediate attention.

### Trend Analysis & Summary
- Mini-charts display short-term trends for motion and environment data.  
- Summary panel provides quick insights on overall system status.

---

## Applications

- **Wearable Health Monitoring:** Track motion, activity, and environmental conditions for health or fitness applications.  
- **Industrial IoT:** Real-time monitoring of machine vibrations, temperature, and environment for predictive maintenance.  
- **Smart Environments:** Monitor humidity and temperature for homes, offices, or greenhouses.  
- **Education & Demonstrations:** Interactive tool for IoT and embedded system learning, hackathons, and live demos.
