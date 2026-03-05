# Electrical Outlet IoT

An environmental monitoring platform built around a **smart outlet form factor**.

The device focuses on **indoor air quality and comfort monitoring**, rather than electrical power measurement.  
The objective is to integrate multiple environmental sensors into a compact embedded system capable of collecting meaningful indoor data.

---

# Project Objectives

- Monitor key **indoor air quality parameters**
- Track **comfort indicators** in living environments
- Maintain a **modular hardware architecture**
- Provide a foundation for **future data logging and automation**

---

# Planned Measurements

The system aims to monitor the following environmental indicators:

- CO₂ concentration
- Temperature
- Relative humidity
- Atmospheric pressure
- TVOC concentration
- Carbon monoxide and flammable gases
- Particulate matter (PM2.5 / PM10)
- Ambient noise level
- Light spectrum
- Motion detection

---

# Hardware Platform

The prototype is built around an **ESP32-S3 microcontroller**, combined with several specialized environmental sensors.

## Core Controller

- `ESP32-S3` — main microcontroller and communication interface

## Sensor Stack

| Measurement            | Sensor |
|------------------------|------|
| CO₂                    | **Sensirion SCD40** |
| VOC / NOx              | **Sensirion SGP41** |
| PM particles           | **Plantower PMS7003** |
| Temperature / Humidity | **Sensirion SHT41** |
| Atmospheric pressure   | **Bosch BMP280** |
| Light spectrum         | **AMS AS7341** |
| Motion detection       | **AS312 PIR sensor** |
| Noise level            | **INMP441 MEMS microphone** |
| Combustible gases      | **MiCS-5524 gas sensor** |

### Audio Acquisition Options

Two possible microphone configurations are considered:

- `ICS-40730 + PCM1809` (high quality analog path)
- `INMP441` digital MEMS microphone (current prototype)

---

# Development Status

The current stage of the project focuses on:

- sensor integration
- hardware validation
- initial firmware testing

The firmware architecture and data infrastructure will evolve as the hardware platform stabilizes.

---

Dimensioni placca presa T13 suisse
Placca frontale:      ~86 mm × 86 mm
Foro incasso muro:    ~55–60 mm diametro
Profondità scatola:   ~45–60 mm

