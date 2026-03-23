# Electrical Outlet IoT

# Work in progress ... (not yet functional)

An embedded environmental monitoring platform designed to fit inside a **standard electrical outlet enclosure**.

The device focuses on **indoor air quality and comfort monitoring**, rather than electrical power measurement.  
The objective is to integrate multiple environmental sensors into a compact embedded system capable of collecting meaningful indoor data.

## Quick Start (PlatformIO)

```bash
pio run
```

Optional commands:

```bash
pio run -t upload
pio device monitor
```

## Repository Structure

```text
src/       Main firmware tasks and application entry points
include/   Public headers for application modules
lib/       Reusable modules (device_state, mqtt, drivers)
```

## README Navigation

- Hardware overview: `Hardware Platform`
- Firmware architecture: `Firmware Architecture`
- Shared state model: `Device State`
- Supervision logic: `System Task`
- MQTT model: `Communication Task`

---

## Project Objectives

- Monitor key **indoor air quality parameters**
- Track **comfort indicators** in living environments
- Maintain a **modular hardware architecture**
- Provide a foundation for **future data logging and automation**

---

## Planned Measurements

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
- LED status (RGB LED)

---

## Hardware Platform

The prototype is built around an **ESP32-S3 microcontroller**, combined with several specialized environmental sensors.

### Core Controller

- `ESP32-S3` — main microcontroller and communication interface

### Sensor Stack


| Measurement              | Sensor                      |
| ------------------------ | --------------------------- |
| CO₂                      | **Sensirion SCD40**         |
| VOC / NOx                | **Sensirion SGP41**         |
| PM particles             | **Plantower PMS7003**       |
| Temperature / Humidity   | **Sensirion SHT41**         |
| Atmospheric pressure     | **Bosch BMP280**            |
| Light spectrum           | **AMS AS7341**              |
| Motion detection         | **AS312 PIR sensor**        |
| Noise level              | **INMP441 MEMS microphone** |
| Combustible gases        | **MiCS-5524 gas sensor**    |
| RGB LED status indicator | **APHF1608SEEQBDZGKC**      |


### Audio Acquisition Options

Two possible microphone configurations are considered:

- `ICS-40730 + PCM1809` (high quality analog path)
- `INMP441` digital MEMS microphone (current prototype)

---

### First Prototype Layout

This diagram shows the first prototype layout.

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



## System Architecture

The system is structured around a sensor node based on an ESP32-S3 microcontroller.

All environmental sensors are connected directly to the microcontroller through I2C, UART, GPIO, ADC and I2S interfaces.

The firmware is implemented using **FreeRTOS**, where sensors are grouped into a small set of coordinated tasks.  
Measurements are validated and aggregated before being transmitted through Wi-Fi using the **MQTT protocol**.

The MQTT broker acts as the central data hub for future integrations such as:

- home automation platforms
- data logging systems
- monitoring dashboards

## Development Status

The current stage of the project focuses on:

- sensor integration
- hardware validation
- initial firmware testing

The firmware architecture and data infrastructure will evolve as the hardware platform stabilizes.

## Project Progress Report

This section summarizes the current progress based on the repository contents.

---

### Swiss T13 Outlet Plate Dimensions

- Front plate: ~86 mm x 86 mm
- Wall recess hole: ~55-60 mm diameter
- Box depth: ~45-60 mm

## Firmware Architecture

The firmware is built on **FreeRTOS** and follows a **minimal task architecture** designed for reliability, maintainability, and predictable timing behavior. Instead of dedicating a separate task to each sensor, the system groups responsibilities into a small number of well-defined tasks. This approach reduces context switching, memory usage, and synchronization complexity, which is important for a production-grade embedded system.

The firmware is structured around **four main tasks**.

---

## Task Overview


| Task               | Responsibility                                            |
| ------------------ | --------------------------------------------------------- |
| `acquisition_task` | Polls environmental sensors and updates system state      |
| `audio_task`       | Handles I2S audio acquisition and noise level estimation  |
| `system_task`      | Supervises system health, alarms, watchdog and LED status |
| `comm_task`        | Manages Wi‑Fi connectivity and MQTT communication         |


This architecture provides clear separation between **sensor acquisition**, **signal processing**, **system supervision**, and **network communication**.

---

## System State Model

Sensor measurements and device status are stored in a centralized structure called `device_state`.

This structure represents the **current state of the device** and is shared between tasks. It acts as the single source of truth for the system.

Typical stored values include:

- CO₂ concentration
- VOC / NOx index
- particulate matter
- temperature and humidity
- atmospheric pressure
- light spectrum
- motion detection
- noise level
- combustible gas level
- network connectivity state
- system fault flags

The acquisition and audio tasks update this structure, while the communication task reads it to publish telemetry.

Access to `device_state` is protected using a **mutex** to guarantee thread‑safe access with minimal locking time.

---

## Inter‑Task Communication

The firmware uses a minimal set of synchronization primitives to keep the system deterministic and lightweight.

### Task Notifications

Direct **task notifications** are used for fast signaling between tasks.

For example, the acquisition task can notify the `system_task` when:

- a threshold is exceeded
- a sensor failure occurs
- a motion event is detected

Task notifications are preferred for one‑to‑one signaling because they are faster and require less RAM than queues.

---

### Publish Queue

`publish_queue`

A small queue is used to request outgoing telemetry or alarm publication.

Typical messages include:

- periodic telemetry publication
- alarm events
- device status updates

The `comm_task` reads this queue and publishes messages over MQTT.

---

## Task Responsibilities

### Acquisition Task

`acquisition_task`

Responsible for managing the majority of environmental sensors.

Sensors handled by this task include:

- SCD40 (CO₂)
- SGP41 (VOC / NOx)
- SHT41 (temperature / humidity)
- BMP280 (pressure)
- AS7341 (light spectrum)
- PMS7003 (particulate matter)
- AS312 (motion detection)
- MiCS‑5524 (combustible gases)

Instead of running independent threads, sensors are polled using an **internal scheduler** based on time intervals.

Typical polling intervals:

  Sensor                   Interval

---

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

### Audio Task (I2S)

`audio_task`

Handles audio acquisition from the **I2S MEMS microphone**.

Responsibilities include:

- configuring the I2S peripheral
- capturing audio samples using DMA
- computing noise level metrics (RMS / dB)
- updating the system noise measurement

This task is separated from the main acquisition loop because I2S audio streaming has different timing and buffering constraints.

Current prototype microphone:

INMP441 digital MEMS microphone

---

### System Supervision Task

`system_task`

Acts as the central control logic of the device.

Responsibilities include:

- alarm detection
- system fault monitoring
- watchdog supervision
- degraded mode handling
- LED status control

The task reacts to notifications from acquisition and audio tasks and decides whether telemetry or alarms must be published.

---

### Communication Task

`comm_task`

Responsible for all network communication.

Responsibilities include:

- Wi‑Fi connection management
- MQTT broker connection and reconnection
- telemetry publishing
- alarm message publishing
- device status reporting

This task is the **single owner of the MQTT client**, avoiding concurrency issues.

When a publish request is received, the task reads the latest measurements from `device_state` and transmits them to the MQTT broker.

---

## Synchronization Mechanisms

The firmware relies on a minimal set of synchronization primitives.

### Mutex

`g_device_state_mutex`

Protects access to the shared `device_state` structure.

---

### Event Groups

Event groups are used to represent system state flags such as:

- Wi‑Fi connected
- MQTT connected
- system ready
- degraded mode
- alarm active

These flags allow tasks to quickly react to changes in system status.

---

## Task Interaction Diagram

```mermaid
flowchart LR

    A[acquisition_task]
    B[audio_task]
    S[system_task]
    C[comm_task]

    DS[device_state]

    MQTT[MQTT Broker]

    A -->|update measurements| DS
    B -->|update noise level| DS

    S -->|monitor system state| DS

    C -->|read snapshot| DS

    C -->|publish telemetry| MQTT
```



### Task Stack Sizes


| Task             | Stack Size         |
| ---------------- | ------------------ |
| acquisition_task | 4096 – 8192 words  |
| audio_task       | 8192 – 16384 words |
| system_task      | 4096 words         |
| comm_task        | 8192 – 12288 words |


At the beginning of the project, relatively large stack sizes are used to avoid stack overflows.  
Later, the actual stack usage will be evaluated using:

`uxTaskGetStackHighWaterMark(NULL)`

Based on these measurements, the stack sizes can be reduced to optimize memory usage.

---

### Task Priorities


| Task             | Priority |
| ---------------- | -------- |
| acquisition_task | 2        |
| audio_task       | 2        |
| system_task      | 4        |
| comm_task        | 3        |


The default priorities for the **main task** and the **idle task** will remain unchanged.

### Logging in ESP-IDF


| Macro    | Meaning | When to use                    |
| -------- | ------- | ------------------------------ |
| ESP_LOGE | Error   | Errors or problems             |
| ESP_LOGW | Warning | Warnings or recoverable issues |
| ESP_LOGI | Info    | Normal informational output    |
| ESP_LOGD | Debug   | Debugging information          |
| ESP_LOGV | Verbose | Highly detailed trace output   |


### Stack tuning

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

The `counter` parameter is used to control how often the stack usage is printed. `uxTaskGetStackHighWaterMark()` returns the minimum amount of stack that has remained unused since the task started running. This allows estimating the real stack usage of the task.

### Task Creation

The basic task structure and stack monitoring are in place.  
Task internals are developed incrementally and validated through integration tests.

### Device State

The **device state** represents the current internal state of the device. It contains the latest measurements collected from the sensors as well as important system status flags.

All tasks in the firmware share this structure. Instead of storing measurements in multiple global variables, all data is centralized inside a single structure called `device_state_t`. This approach improves code readability, simplifies data management, and provides a single source of truth for the system.

The device state is implemented in the `device_state.h` and `device_state.c` files.

```c
//Number of spectral channels measured by the AS7341
#define AS7341_CHANNELS 8

//AS7341 spectral data structure
typedef struct{
    float channels[AS7341_CHANNELS];
} as7341_data_t;

typedef struct{
    float co2_ppm;
    float temperature_c;
    float humidity_percent;
    float pressure_hpa;

    float voc_index;
    float nox_index;

    float pm1_0_ug_m3;
    float pm2_5_ug_m3;
    float pm10_ug_m3;

    float noise_db;

    as7341_data_t light;

    float gas_level_raw;

    bool motion_detected;

    bool wifi_connected;
    bool mqtt_connected;
    bool alarm_active;
    bool degraded_mode;

} device_state_t;

extern device_state_t g_device_state;
extern SemaphoreHandle_t g_device_state_mutex;
void device_state_init(void);
```

#### Global Device State

A global instance of the structure is created:

> `g_device_state`

This variable contains the latest known values of all measurements and system flags. All tasks read from or update this structure when interacting with the system state.

#### Thread Safety

Since multiple FreeRTOS tasks access the shared device state concurrently, access must be protected using a mutex:

> `g_device_state_mutex`

The mutex ensures that only one task at a time can read or modify the shared structure, preventing race conditions and inconsistent updates.

In this architecture, the mutex is **not kept during the whole sensor acquisition phase**.  
The `acquisition_task` first updates a **local persistent structure** called `acquisition_local_state_t`, and only after that it briefly locks the mutex to copy the final values into `g_device_state`.

Typical pattern:

```text
read sensors into local_state
take mutex
copy local_state into g_device_state
release mutex
```

This approach keeps the critical section short and avoids blocking other tasks while sensors are being read.

#### Initialization

The function `device_state_init()` is responsible for initializing the module:

- sets the initial state values
- creates the mutex used for synchronization

This function must be called during system startup before any task accesses the shared device state.

#### Usage

The device state is initialized in `main.c`.
After initialization, the code verifies that the mutex was successfully created.

All tasks that need to read or update the device state must access it through the mutex to guarantee thread-safe access.

In this acquisition model, sensor values are first stored in a task-local structure and then committed to the shared state:

```c
// Copy the complete local state into the shared device state
if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
{
    g_device_state.motion_detected   = local_state.motion_detected;

    g_device_state.gas_level_raw     = local_state.gas_level_raw;

    g_device_state.voc_index         = local_state.voc_index;
    g_device_state.nox_index         = local_state.nox_index;

    g_device_state.temperature_c     = local_state.temperature_c;
    g_device_state.humidity_percent  = local_state.humidity_percent;

    g_device_state.pressure_hpa      = local_state.pressure_hpa;

    g_device_state.co2_ppm           = local_state.co2_ppm;
    g_device_state.temperature_scd40 = local_state.temperature_scd40;
    g_device_state.humidity_scd40    = local_state.humidity_scd40;

    g_device_state.pm1_0_ug_m3       = local_state.pm1_0_ug_m3;
    g_device_state.pm2_5_ug_m3       = local_state.pm2_5_ug_m3;
    g_device_state.pm10_ug_m3        = local_state.pm10_ug_m3;

    for (int i = 0; i < AS7341_CHANNELS; i++)
    {
        g_device_state.light.channels[i] = local_state.light.channels[i];
    }
    xSemaphoreGive(g_device_state_mutex);
}
```

So the shared structure stays centralized, but the actual acquisition work happens outside the critical section.

#### Scheduler Inside the Acquisition Task

The `acquisition_task` uses a simple internal time-based scheduler to poll sensors at different intervals.

Instead of creating one FreeRTOS task for each sensor, the firmware stores one timestamp for each sensor group and checks at every loop iteration whether its polling interval has expired.

The task wakes up every 100 ms using `vTaskDelayUntil()`.
At each iteration:

- it updates the current tick
- it checks whether each sensor interval has elapsed
- if yes, it updates the corresponding field in local_state
- at the end of the loop, it copies the complete local_state into g_device_state under mutex

This keeps the architecture simple, scalable, and memory-efficient.

Example based on the current code:

```c
void acquisition_task(void *pvParameters)
{
    (void)pvParameters;

    uint32_t counter = 0;
    uint32_t alive_counter = 0;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t now = xLastWakeTime;

    acquisition_local_state_t local_state = {
        .motion_detected = false,
    };

    TickType_t last_as312    = now - pdMS_TO_TICKS(AS312_INTERVAL_MS);

    for (;;)
    {
        alive_counter++;
        if (alive_counter >= 20)
        {
            ESP_LOGI(TAG, "Acquisition task alive");
            alive_counter = 0;
        }

        now = xTaskGetTickCount();

        if ((now - last_as312) >= pdMS_TO_TICKS(AS312_INTERVAL_MS))
        {
            last_as312 = now;
            local_state.motion_detected = true;
        }

        if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            g_device_state.motion_detected  = local_state.motion_detected;
            xSemaphoreGive(g_device_state_mutex);
        }

        logTaskStackUsage(&counter, TAG, STACK_ACQUISITION_WORDS);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    }
}
```

#### Device State Update

The following diagram shows the current logic:

```mermaid
flowchart TD

START[Task start]
ALIVE[Update alive counter]
NOW[Get current tick]

CHECK{AS312 interval elapsed?}
UPDATE_TS[Update timestamp<br/>last_as312 = now]
READ[Update local_state.motion_detected]

LOCK[Take device_state mutex]
COPY[Copy local_state.motion_detected<br/>into g_device_state]
UNLOCK[Release mutex]

STACK[Log stack usage]
DELAY[vTaskDelayUntil 100 ms]

START --> ALIVE
ALIVE --> NOW
NOW --> CHECK

CHECK -->|yes| UPDATE_TS
UPDATE_TS --> READ
READ --> LOCK

CHECK -->|no| LOCK

LOCK --> COPY
COPY --> UNLOCK
UNLOCK --> STACK
STACK --> DELAY
DELAY --> ALIVE
```



#### Interaction of `acquisition_task` with the Shared Device State

This diagram summarizes the architecture pattern:
the task works first on `local_state`, then performs a short synchronized copy into the shared state.

```mermaid
flowchart LR

A[acquisition_task]
L[acquisition_local_state_t]
M[g_device_state_mutex]
S[g_device_state]

A -->|read/update sensor values| L
A -->|take mutex| M
L -->|copy final values| S
A -->|release mutex| M
```



This is more accurate than saying that each sensor directly locks the mutex during measurement.
In the updated implementation, the mutex is only used during the final commit phase.

### Device State Structure

To support system supervision and communication tasks, the device_state structure also contains additional metadata:

- timestamps (last update time)
- valid flags (data validity)
- fault flags (sensor errors)

Example:

Typical extension fields in `device_state` include:

```c
    // -----------------------------
    // Last update timestamps
    // -----------------------------
    TickType_t motion_last_update;

    // -----------------------------
    // Validity flags
    // -----------------------------
    bool motion_valid;

    // -----------------------------
    // Fault flags
    // -----------------------------
    bool motion_fault;

```

These fields allow other tasks (for example system_task or comm_task) to detect:

- stale measurements
- sensor failures
- degraded operating modes

## System Task

The `system_task` is responsible for supervising the health of the device.

Unlike the acquisition_task, it does not read sensors directly. Instead, it reads the shared `device_state`, checks whether the data is still reliable, and updates the global system flags. Typical responsibilities include:

- checking sensor validity
- checking sensor fault flags
- checking data freshness
- updating `degraded_mode`, `alarm_active`, and `system_ok`

### Supervision Model

The task follows this simple sequence:

```text
read shared state
check sensor status
compute system flags
write flags back
```

To keep access thread-safe, the shared state is protected by the same mutex used by the other tasks.

#### Example: Motion Sensor Supervision

For clarity, the README uses only the motion sensor as an example. The system_task checks three conditions:

1. Validity

If the motion data is not valid, the system enters degraded mode.

```c
if (!state_copy.motion_valid)
{
    degraded_mode = true;
}
```

1. Fault

If the driver reports a motion sensor fault, the system enters degraded mode.

```c
if (state_copy.motion_fault)
{
    degraded_mode = true;
}
```

1. Freshness

If the motion sensor has not been updated for too long, the system enters degraded mode.

```c
if ((now - state_copy.motion_last_update) > pdMS_TO_TICKS(MOTION_TIMEOUT_MS))
{
    degraded_mode = true;
}
```

### Important Note

The absence of motion is not a fault condition. For example:

```text
motion_detected = false;
motion_valid = true;
```

means that no movement was detected, but the sensor is still working correctly. So `degraded_mode` depends on the health of the sensor, not on the measured value itself.

### System Flags Update

After evaluating the motion sensor state, the task updates the global system flags. Example:

```c
system_ok = !degraded_mode;
```

Then the result is written back to the shared state:

```c
if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
{
    g_device_state.degraded_mode = degraded_mode;
    g_device_state.alarm_active = alarm_active;
    g_device_state.system_ok = system_ok;

    xSemaphoreGive(g_device_state_mutex);
}
```

### Flow Example

The following diagram shows the logic of the system_task using only the motion sensor.

```mermaid
flowchart TD

START[Task start]
NOW[Get current tick]

LOCK1[Take device_state mutex]
COPY[Copy g_device_state into state_copy]
UNLOCK1[Release mutex]

VALID{motion_valid?}
FAULT{motion_fault?}
FRESH{motion_last_update too old?}

DEGRADED[Set degraded_mode = true]
OK[Set system_ok = true]
NOT_OK[Set system_ok = false]

ALARM[Set alarm_active = false]

LOCK2[Take device_state mutex]
WRITE[Write degraded_mode alarm_active and system_ok to g_device_state]
UNLOCK2[Release mutex]

DELAY[vTaskDelayUntil 1000 ms]

START --> NOW
NOW --> LOCK1
LOCK1 --> COPY
COPY --> UNLOCK1
UNLOCK1 --> VALID

VALID -->|no| DEGRADED
VALID -->|yes| FAULT

FAULT -->|yes| DEGRADED
FAULT -->|no| FRESH

FRESH -->|yes| DEGRADED
FRESH -->|no| OK

DEGRADED --> NOT_OK
OK --> ALARM
NOT_OK --> ALARM

ALARM --> LOCK2
LOCK2 --> WRITE
WRITE --> UNLOCK2
UNLOCK2 --> DELAY
DELAY --> NOW
```



### Interaction With the Firmware

```mermaid
flowchart LR

A[acquisition_task]
DS[device_state]
S[system_task]
C[comm_task]

A -->|update measurements| DS
S -->|read and update system flags| DS
DS -->|snapshot| C
```



- `acquisition_task` updates sensor values
- `system_task` supervises sensor health
- `comm_task` publishes the device state

### Future Extension

The same supervision pattern can be applied to all sensors:

- validity
- fault
- freshness

This keeps the architecture modular, easy to maintain, and scalable.

## Communication Task

The `comm_task` is responsible for managing the network connection and publishing the device state to the MQTT broker.

The task periodically reads a snapshot of the shared `device_state`, builds the appropriate MQTT payloads and publishes them to the configured MQTT broker.

To avoid blocking other tasks, the communication task only performs:

- network supervision
- MQTT connection management
- payload generation
- message publishing
All sensor acquisition and system supervision are handled by other tasks.

### MQTT Payload

The MQTT payloads are built using the mqtt_payload module.

This module converts the internal device_state_t structure into JSON payloads suitable for transmission over MQTT.

Files:

```bash
mqtt_payload.c
mqtt_payload.h
```

Example functions:

```bash
mqtt_payload_build_state()
mqtt_payload_build_system()
mqtt_payload_build_environment()
mqtt_payload_build_audio()
mqtt_payload_build_faults()
mqtt_payload_build_validity()
mqtt_payload_build_last_update()
mqtt_payload_build_alarm()
mqtt_payload_build_availability()
```

This design keeps the communication task simple and separates data formatting from networking logic.

### MQTT Topic Structure

The device publishes data using the following topic hierarchy:

```bash
devices/<device_id>/state
devices/<device_id>/telemetry/environment
devices/<device_id>/telemetry/audio
devices/<device_id>/status/system
devices/<device_id>/event/alarm
devices/<device_id>/availability
devices/<device_id>/command
devices/<device_id>/command/reply
```

This structure allows easy integration with:

- IoT dashboards
- Home automation systems
- monitoring platforms
- data collection services

### MQTT Module Folder Structure

```bash
mqtt/
├── mqtt_topic.h
├── mqtt_topic.c
├── mqtt_payload.h
├── mqtt_payload.c
├── mqtt_publish.h
├── mqtt_publish.c
├── mqtt_defs.h
```

#### Module responsibilities


| File           | Responsibility                          |
| -------------- | --------------------------------------- |
| `mqtt_topic`   | Build MQTT topic strings                |
| `mqtt_payload` | Convert device_state into JSON payloads |
| `mqtt_publish` | Publish messages to the MQTT broker     |
| `mqtt_defs`    | Global MQTT configuration and constants |


### Debugging MQTT & ESP32

#### Viewing MQTT Topics on Raspberry Pi

To view all messages published by the ESP32 to the MQTT broker, use:

```bash
mosquitto_sub -h localhost -t "#" -v
```

This command subscribes to all topics (`#`) and displays both the topic and the payload (`-v`).

#### Monitoring the ESP32

To see logs from the ESP32 (WiFi, MQTT, debug output), run:

```bash
pio device monitor -b 115200   
```

