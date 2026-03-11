# Electrical Outlet IoT

An embedded environmental monitoring platform designed to fit inside a **standard electrical outlet enclosure**.

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
- Led status (rgb led)

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
| Led status (rgb)       | **APHF1608SEEQBDZGKC** |

### Audio Acquisition Options

Two possible microphone configurations are considered:

- `ICS-40730 + PCM1809` (high quality analog path)
- `INMP441` digital MEMS microphone (current prototype)

---

### First layout of the prototype
this is a simple diagram that shows the layout of the prototype
```mermaid
flowchart LR

    A[ESP32-S3 Sensor Node]

    subgraph ENV[Environmental Sensors]
        direction TB
        X1[ ]:::ghost
        B[SCD40]
        C[SGP41]
        D[PMS7003]
        E[SHT41]
        F[BMP280]
        G[AS7341]
    end

    subgraph AUX[Presence, Audio, Safety and Status]
        direction TB
        X2[ ]:::ghost
        H[AS312]
        I[INMP441]
        J[MiCS-5524]
        Z[RGB LED]
    end

    subgraph DATA[Measured Quantities and Outputs]
        direction TB
        X3[ ]:::ghost
        K[CO2]
        L[VOC / NOx]
        R[Particulate Matter]
        M[Temperature / Humidity]
        N[Atmospheric Pressure]
        O[Light Spectrum]
        P[Motion Detection]
        Q[Noise Level]
        S[Combustible Gas]
        W[RGB Status Indication]
    end

    T[MQTT Broker]

    A -->|I2C| B
    A -->|I2C| C
    A -->|UART| D
    A -->|I2C| E
    A -->|I2C| F
    A -->|I2C| G
    A -->|GPIO| H
    A -->|I2S| I
    A -->|ADC| J
    A -->|GPIO / PWM| Z

    B --> K
    C --> L
    D --> R
    E --> M
    F --> N
    G --> O
    H --> P
    I --> Q
    J --> S
    Z --> W
    A <-->|Wi-Fi / MQTT| T

    classDef mcu fill:#1f2937,color:#ffffff,stroke:#111827,stroke-width:2px
    classDef sensor fill:#f8fafc,color:#111827,stroke:#94a3b8,stroke-width:1.5px
    classDef actuator fill:#fff7ed,color:#111827,stroke:#fb923c,stroke-width:1.5px
    classDef quantity fill:#ecfeff,color:#0f172a,stroke:#06b6d4,stroke-width:1.5px
    classDef broker fill:#fef3c7,color:#78350f,stroke:#f59e0b,stroke-width:2px
    classDef group fill:#ffffff,color:#111827,stroke:#cbd5e1,stroke-width:1.5px
    classDef ghost fill:transparent,stroke:transparent

    class X1,X2,X3 ghost
    class A mcu
    class B,C,D,E,F,G,H,I,J sensor
    class Z actuator
    class K,L,R,M,N,O,P,Q,S,W quantity
    class T broker
    class ENV,AUX,DATA group
```
# System Architecture

The system is structured around a sensor node based on an ESP32-S3 microcontroller.

All environmental sensors are connected directly to the microcontroller through I2C, UART, GPIO, ADC and I2S interfaces.

The firmware is implemented using **FreeRTOS**, where each sensor is managed by a dedicated acquisition task.  
Measurements are validated and aggregated before being transmitted through Wi-Fi using the **MQTT protocol**.

The MQTT broker acts as the central data hub for future integrations such as:

- home automation platforms
- data logging systems
- monitoring dashboards

# Development Status

The current stage of the project focuses on:

- sensor integration
- hardware validation
- initial firmware testing

The firmware architecture and data infrastructure will evolve as the hardware platform stabilizes.


# Project Progress Report

This section summarizes the current progress based on the repository contents.

## Completed

- System concept and sensor selection defined in the project documentation.
- Core firmware architecture implemented with FreeRTOS tasks and queues.
- SCD40 integration implemented (`scd40_task`) with data acquisition, validity checks, timeout supervision, and automatic recovery.
- RGB status LED task implemented with CO2 threshold mapping and offline indication.
- Watchdog configuration implemented and integrated into runtime tasks.
- First PCB work completed in KiCad with a main board project and multiple sensor-related schematic sheets.
- Initial bill of materials and component ordering tracked in `parts_shop/component.csv`.

## In Progress

- Full multi-sensor firmware integration beyond the currently active SCD40 path.
- Hardware bring-up and validation of all selected sensors on the prototype.
- End-to-end firmware robustness testing on real hardware.

## Open Points / Gaps -> fix these points before the first prototype

- `main.c` references `mq7_task` and related types, but no `mq7` implementation files are present yet in the repository.
- No automated test suite is currently implemented (the `test` folder still contains only the default template README).
- Data logging, communication pipeline, and automation features are not yet present.

## Next Milestones

- Implement and integrate the remaining sensor tasks one by one.
- Add a unified sensor data model and publish mechanism for all measurements.
- Add basic hardware-in-the-loop validation tests and fault-injection scenarios.
- Stabilize PCB revision after bring-up feedback and final pin mapping checks.


---
## Swiss T13 Outlet Plate Dimensions

- Front plate: ~86 mm x 86 mm
- Wall recess hole: ~55-60 mm diameter
- Box depth: ~45-60 mm

# Firmware Architecture

The firmware is built on **FreeRTOS** and follows a **minimal task architecture** designed for reliability, maintainability, and predictable timing behavior.

Instead of dedicating a separate task to each sensor, the system groups responsibilities into a small number of well-defined tasks.


This approach reduces context switching, memory usage, and synchronization complexity, which is important for a production-grade embedded system.

The firmware is structured around **four main tasks**.

---

# Task Overview

  -----------------------------------------------------------------------
  Task                                Responsibility
  ----------------------------------- -----------------------------------
  `acquisition_task`                  Polls environmental sensors and
                                      updates system state

  `audio_task`                        Handles I2S audio acquisition and
                                      noise level estimation

  `system_task`                       Supervises system health, alarms,
                                      watchdog and LED status

  `comm_task`                         Manages Wi‑Fi connectivity and MQTT
                                      communication
  -----------------------------------------------------------------------

This architecture provides clear separation between **sensor acquisition**, **signal processing**, **system supervision**, and **network communication**.

---

# System State Model

Sensor measurements and device status are stored in a centralized structure called `device_state`.

This structure represents the **current state of the device** and is shared between tasks. It acts as the single source of truth for the system.

Typical stored values include:

-   CO₂ concentration
-   VOC / NOx index
-   particulate matter
-   temperature and humidity
-   atmospheric pressure
-   light spectrum
-   motion detection
-   noise level
-   combustible gas level
-   network connectivity state
-   system fault flags

The acquisition tasks update this structure, while the communication task reads it to publish telemetry.

Access to `device_state` is protected using a **mutex** to guarantee thread‑safe access with minimal locking time.

---

# Inter‑Task Communication

The firmware uses a minimal set of synchronization primitives to keep the system deterministic and lightweight.

## Task Notifications

Direct **task notifications** are used for fast signaling between tasks.


For example, acquisition tasks can notify the `system_task` when:

-   a threshold is exceeded
-   a sensor failure occurs
-   a motion event is detected

Task notifications are preferred for one‑to‑one signaling because they are faster and require less RAM than queues.

---

## Publish Queue

`publish_queue`

A small queue is used to request outgoing telemetry or alarm publication.

Typical messages include:

-   periodic telemetry publication
-   alarm events
-   device status updates

The `comm_task` reads this queue and publishes messages over MQTT.

---

# Task Responsibilities

## Acquisition Task

`acquisition_task`

Responsible for managing the majority of environmental sensors.

Sensors handled by this task include:

-   SCD40 (CO₂)
-   SGP41 (VOC / NOx)
-   SHT41 (temperature / humidity)
-   BMP280 (pressure)
-   AS7341 (light spectrum)
-   PMS7003 (particulate matter)
-   AS312 (motion detection)
-   MiCS‑5524 (combustible gases)

Instead of running independent threads, sensors are polled using an **internal scheduler** based on time intervals.

Typical polling intervals:

  Sensor                   Interval
  ------------------------ ----------
  Motion detection         100 ms
  Gas sensor               1 s
  VOC / NOx                1 s
  Temperature / Humidity   2 s
  Pressure                 2 s
  Light spectrum           2--5 s
  CO₂                      5 s
  Particulate matter       5--10 s

The task updates the shared `device_state` structure and generates notifications when thresholds or faults occur.

---

## Audio Task (I2S)

`audio_task`

Handles audio acquisition from the **I2S MEMS microphone**.

Responsibilities include:

-   configuring the I2S peripheral
-   capturing audio samples using DMA
-   computing noise level metrics (RMS / dB)
-   updating the system noise measurement

This task is separated from the main acquisition loop because I2S audio streaming has different timing and buffering constraints.

Current prototype microphone:

INMP441 digital MEMS microphone

---

## System Supervision Task

`system_task`

Acts as the central control logic of the device.

Responsibilities include:

-   alarm detection
-   system fault monitoring
-   watchdog supervision
-   degraded mode handling
-   LED status control

The task reacts to notifications from acquisition tasks and decides whether telemetry or alarms must be published.

---

## Communication Task

`comm_task`

Responsible for all network communication.

Responsibilities include:

-   Wi‑Fi connection management
-   MQTT broker connection and reconnection
-   telemetry publishing
-   alarm message publishing
-   device status reporting

This task is the **single owner of the MQTT client**, avoiding concurrency issues.

When a publish request is received, the task reads the latest measurements from `device_state` and transmits them to the MQTT broker.

---

# Synchronization Mechanisms

The firmware relies on a minimal set of synchronization primitives.

## Mutex

`state_mutex`

Protects access to the shared `device_state` structure.

---

## Event Groups

Event groups are used to represent system state flags such as:

-   Wi‑Fi connected
-   MQTT connected
-   system ready
-   degraded mode
-   alarm active

These flags allow tasks to quickly react to changes in system status.

---

# Task Interaction Diagram

``` mermaid
flowchart LR

    A[acquisition_task]
    B[audio_task]
    S[system_task]
    C[comm_task]

    DS[device_state]

    MQTT[MQTT Broker]

    A -->|update measurements| DS
    B -->|update noise level| DS

    A -->|task notification| S
    B -->|task notification| S

    S -->|publish request| C

    C -->|read snapshot| DS

    C --> MQTT
```

## Task Stack Sizes

| Task | Stack Size |
|-----|-----|
| acquisition_task | 4096 – 8192 bytes |
| audio_task | 8192 – 16384 bytes |
| system_task | 4096 bytes |
| comm_task | 8192 – 12288 bytes |

At the beginning of the project, relatively large stack sizes are used to avoid stack overflows.  
Later, the actual stack usage will be evaluated using:

`uxTaskGetStackHighWaterMark(NULL)`

Based on these measurements, the stack sizes can be reduced to optimize memory usage.

---

## Task Priorities

| Task | Priority |
|-----|-----|
| acquisition_task | 2 |
| audio_task | 2 |
| system_task | 4 |
| comm_task | 3 |

The default priorities for the **main task** and the **idle task** will remain unchanged.

## Recap: Printing in esp-idf

| Macro      | Meaning  | When to use                    |
|------------|----------|-------------------------------|
| ESP_LOGE   | Error    | Errors or problems            |
| ESP_LOGW   | Warning  | Warnings or recoverable issues|
| ESP_LOGI   | Info     | Normal informational output   |
| ESP_LOGD   | Debug    | Debugging information         |
| ESP_LOGV   | Verbose  | Highly detailed trace output  |

## Stack tuning
I implemented a function to periodically log the stack usage of the current task.
```c
void logTaskStackUsage(uint32_t *counter, const char *TAG, UBaseType_t task_stack_size)
{
    if (++(*counter) % 10 == 0)
    {
        UBaseType_t stack_remaining = uxTaskGetStackHighWaterMark(NULL);
        UBaseType_t stack_used = task_stack_size - stack_remaining;

        ESP_LOGI(TAG, "Stack used: %u words | remaining: %u words",
                 stack_used, stack_remaining);
    }
}
```
The `counter` parameter is used to control how often the stack usage is printed.