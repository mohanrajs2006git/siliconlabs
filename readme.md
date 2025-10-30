# 🧠 Edge AI-Based Smart Patient Monitoring and Assistance System  

## 📖 Overview  
**NeuroCare+** is an intelligent, edge AI-powered patient monitoring and assistance system designed to support bedridden, elderly, and neurologically impaired individuals in healthcare environments.  
Built on the **Silicon Labs SiWx917 SoC**, the system integrates multiple onboard and external sensors to enable **real-time health monitoring, emergency detection, and caregiver communication** — all while ensuring **low-power operation** and **data privacy through on-device inference**.  

---

## 🎯 Objectives  
- Enable real-time monitoring of patient health and movement.  
- Detect emergencies such as falls, fever, or panic episodes using onboard sensors.  
- Provide **Wi-Fi-based alerts** and **mobile notifications** to caregivers and family members.  
- Ensure **offline functionality** and **privacy-preserving edge AI inference**.  
- Enhance patient autonomy and reduce caregiver workload.  

---

## ⚙️ System Architecture  

### 🧩 Hardware Components  
- **Silicon Labs SiWx917 SoC** – Edge AI processor with integrated Wi-Fi, BLE, and sensor interfaces.  
- **Onboard Sensors:**  
  - 6-axis **Inertial Measurement Unit (IMU)** – motion & fall detection  
  - **Temperature and Humidity Sensor** – environment and body temperature tracking  
  - **MEMS Stereo Microphones** – voice command input (for future TinyML integration)  
  - **RGB LED and Push Buttons** – patient interaction and emergency triggers  

---

### 💡 System Workflow  
1. **Sensor Data Acquisition** – The SiWx917 reads data from the onboard IMU, temperature, humidity, and button inputs.  
2. **Edge AI Inference (Planned)** – Local TinyML models analyze sensor data to detect falls, anomalies, or distress events.  
3. **Wireless Communication** – Detected alerts are transmitted to the caretaker via Wi-Fi or BLE.  
4. **Mobile App Integration** – Caretakers receive instant notifications, confirm actions taken, and view patient history logs.  
5. **Data Privacy** – All sensitive processing happens locally on the device to preserve confidentiality.  

---

## 🚀 Features  
✅ Real-time monitoring using SiWx917 onboard sensors  
✅ Wi-Fi-based data transfer to caretaker web or mobile dashboard  
✅ Touch/button-based emergency assistance from the patient  
✅ Mobile app notifications and caregiver response tracking  
✅ Planned TinyML-based fall and voice recognition models  
✅ Scalable and energy-efficient architecture for continuous operation  
✅ Local processing ensuring privacy-first healthcare monitoring  

---

## 🧰 Software Implementation  
- **Platform:** Silicon Labs SiWx917 SDK  
- **Programming Language:** C / Embedded C  
- **Sensor APIs Used:**  
  - IMU (Accelerometer & Gyroscope)  
  - Relative Humidity & Temperature Sensor APIs  
  - Push Button & LED control libraries  
- **Communication:** Wi-Fi (HTTP / MQTT for web server updates)  
- **Data Visualization:** Web dashboard (live monitoring interface)  

---

## 📶 Communication Flow  
1. Patient unit acquires sensor data.  
2. SiWx917 processes and transmits data via Wi-Fi.  
3. Web server updates patient dashboard in real time.  
4. Caretaker mobile app receives notification and confirms response.  
5. History log maintained for family monitoring and reports.  

---

## 🧪 Current Progress  
- Successfully accessed **6-axis IMU, temperature, and humidity sensors** from SiWx917.  
- Implemented **data acquisition and Wi-Fi transmission** to a web server.  
- Integrated **push buttons** for patient input and alert generation.  
- Designed **mobile app interface** for notifications and history logging.  

---

