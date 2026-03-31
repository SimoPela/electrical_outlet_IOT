const socket = io();
const MAX_POINTS = 10;
const LIGHT_WAVELENGTHS_NM = [415, 445, 480, 515, 555, 590, 630, 680];
const AXIS_LIMITS = {
  co2: { min: 0, max: 1550 },
  temp: { min: -20, max: 45 },
  humidity: { min: 0, max: 100 },
  gas: { min: 0, max: 10 },
  light: { min: 0, max: 2500 },
};

const statusPillEl = document.getElementById("status-pill");
const systemAlarmEl = document.getElementById("system-alarm");
const envSnapshotEl = document.getElementById("env-snapshot");
const metaEl = document.getElementById("meta");

const history = {
  labels: [],
  co2: [],
  temp: [],
  humidity: [],
  gas: [],
};
const trendDebugBuffer = [];
let lastLightSignature = null;

function kvRows(rows) {
  return rows
    .map(([key, value, className = "value"]) => `<div class="key">${key}</div><div class="${className}">${value}</div>`)
    .join("");
}

function formatNumber(value, digits = 2) {
  if (typeof value !== "number" || Number.isNaN(value)) return "-";
  return value.toFixed(digits);
}

function toNumberOrNull(value) {
  const num = Number(value);
  return Number.isFinite(num) ? num : null;
}

function toNumberOrZero(value) {
  const num = Number(value);
  return Number.isFinite(num) ? num : 0;
}

function enqueueQueue(queue, value, max = MAX_POINTS) {
  queue.push(value);
  if (queue.length > max) queue.shift();
}

function lastOrDefault(queue, fallback = 0) {
  if (!Array.isArray(queue) || queue.length === 0) return fallback;
  const last = queue[queue.length - 1];
  return Number.isFinite(last) ? last : fallback;
}

function normalizeCo2Level(level) {
  return String(level || "unknown")
    .toLowerCase()
    .replace(/_/g, " ")
    .trim();
}

function co2LevelClass(level) {
  const normalized = normalizeCo2Level(level);
  if (normalized === "optimal") return "value co2-level-optimal";
  if (normalized === "good") return "value co2-level-good";
  if (normalized === "moderate") return "value co2-level-moderate";
  if (normalized === "poor") return "value co2-level-poor";
  if (normalized === "very poor") return "value co2-level-very-poor";
  if (normalized === "critical") return "value co2-level-critical";
  return "value";
}

function getLightChannels(env) {
  if (Array.isArray(env?.light)) {
    return env.light;
  }
  if (Array.isArray(env?.light?.channels)) {
    return env.light.channels;
  }
  if (env?.light && typeof env.light === "object") {
    const objectValues = [
      env.light.f1,
      env.light.f2,
      env.light.f3,
      env.light.f4,
      env.light.f5,
      env.light.f6,
      env.light.f7,
      env.light.f8,
    ];
    if (objectValues.some((v) => v !== undefined)) {
      return objectValues;
    }

    const channelValues = Array.from({ length: 8 }, (_, i) => env.light[`ch${i}`]);
    if (channelValues.some((v) => v !== undefined)) {
      return channelValues;
    }
  }
  if (typeof env?.light === "string") {
    try {
      const parsed = JSON.parse(env.light);
      if (Array.isArray(parsed)) return parsed;
    } catch (_) {
      return env.light.split(",").map((v) => Number(v.trim()));
    }
  }
  return new Array(8).fill(0);
}

function pushSeries(label, env) {
  const nextCo2 = Number.isFinite(Number(env.co2_ppm)) ? Number(env.co2_ppm) : lastOrDefault(history.co2, 0);
  const nextTempRaw = env.temperature_c ?? env.bmp280_temperature_c ?? env.temperature_scd40;
  const nextTemp = Number.isFinite(Number(nextTempRaw)) ? Number(nextTempRaw) : lastOrDefault(history.temp, 0);
  const nextHumRaw = env.humidity_percent ?? env.humidity_scd40;
  const nextHumidity = Number.isFinite(Number(nextHumRaw)) ? Number(nextHumRaw) : lastOrDefault(history.humidity, 0);
  const nextGas = Number.isFinite(Number(env.gas_ppm)) ? Number(env.gas_ppm) : lastOrDefault(history.gas, 0);

  enqueueQueue(history.labels, label);
  enqueueQueue(history.co2, nextCo2);
  enqueueQueue(history.temp, nextTemp);
  enqueueQueue(history.humidity, nextHumidity);
  enqueueQueue(history.gas, nextGas);

  const debugEntry = `co2:${nextCo2.toFixed(1)} temp:${nextTemp.toFixed(1)} hum:${nextHumidity.toFixed(1)} gas:${nextGas.toFixed(2)}`;
  enqueueQueue(trendDebugBuffer, debugEntry);
  console.log("[TREND_QUEUE]", debugEntry);
}

function safeUpdateTrendChart() {
  try {
    const validCo2 = history.co2.filter((v) => Number.isFinite(v));
    if (validCo2.length > 0) {
      const minCo2 = Math.min(...validCo2);
      const maxCo2 = Math.max(...validCo2);
      const dynamicMin = Math.max(0, Math.floor(minCo2 - 100));
      const dynamicMax = Math.ceil(maxCo2 + 100);
      trendChart.options.scales.yCO2.min = dynamicMin;
      trendChart.options.scales.yCO2.max = Math.max(dynamicMax, dynamicMin + 200);
    }

    trendChart.data.labels = history.labels.slice();
    trendChart.data.datasets[0].data = history.co2.slice();
    trendChart.data.datasets[1].data = history.temp.slice();
    trendChart.data.datasets[2].data = history.humidity.slice();
    trendChart.data.datasets[3].data = history.gas.slice();
    trendChart.update("none");
  } catch (error) {
    console.error("Trend chart update error:", error);
  }
}

const trendCtx = document.getElementById("trend-chart");
const trendChart = new Chart(trendCtx, {
  type: "line",
  data: {
    labels: history.labels,
    datasets: [
      { label: "CO2 (ppm)", data: history.co2, borderColor: "#5ca8ff", backgroundColor: "#5ca8ff", tension: 0.25, pointRadius: 0, yAxisID: "yCO2" },
      { label: "Temp (C)", data: history.temp, borderColor: "#ffb14a", backgroundColor: "#ffb14a", tension: 0.25, pointRadius: 0, yAxisID: "yTemp" },
      { label: "Humidity (%)", data: history.humidity, borderColor: "#51d7b5", backgroundColor: "#51d7b5", tension: 0.25, pointRadius: 0, yAxisID: "yHumidity" },
      { label: "Gas (ppm)", data: history.gas, borderColor: "#e98cff", backgroundColor: "#e98cff", tension: 0.25, pointRadius: 0, yAxisID: "yGas" },
    ],
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,
    plugins: { legend: { labels: { color: "#d7e4ff" } } },
    scales: {
      x: { ticks: { color: "#a9bbde", maxTicksLimit: 8 }, grid: { color: "rgba(140,160,200,0.12)" } },
      yCO2: {
        type: "linear",
        position: "left",
        min: AXIS_LIMITS.co2.min,
        max: AXIS_LIMITS.co2.max,
        ticks: { color: "#a9bbde", stepSize: 500 },
        grid: { color: "rgba(140,160,200,0.12)" },
      },
      yTemp: {
        type: "linear",
        position: "right",
        min: AXIS_LIMITS.temp.min,
        max: AXIS_LIMITS.temp.max,
        ticks: { color: "#ffb14a", stepSize: 20 },
        grid: { drawOnChartArea: false },
      },
      yHumidity: {
        type: "linear",
        position: "right",
        min: AXIS_LIMITS.humidity.min,
        max: AXIS_LIMITS.humidity.max,
        ticks: { color: "#51d7b5", stepSize: 20 },
        grid: { drawOnChartArea: false },
      },
      yGas: {
        type: "linear",
        position: "right",
        min: AXIS_LIMITS.gas.min,
        max: AXIS_LIMITS.gas.max,
        ticks: { color: "#e98cff", stepSize: 1 },
        grid: { drawOnChartArea: false },
      },
    },
  },
});

const spectrumCtx = document.getElementById("spectrum-chart");
const spectrumChart = new Chart(spectrumCtx, {
  type: "bar",
  data: {
    labels: LIGHT_WAVELENGTHS_NM.map((w) => `${w}nm`),
    datasets: [
      {
        label: "AS7341 Channels",
        data: new Array(8).fill(0),
        backgroundColor: ["#5f6cff", "#4a86ff", "#36a2ff", "#38d9a9", "#9be15d", "#ffd166", "#ff9f43", "#ff6b6b"],
        borderWidth: 0,
      },
    ],
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,
    plugins: { legend: { labels: { color: "#d7e4ff" } } },
    scales: {
      x: { ticks: { color: "#a9bbde" }, grid: { color: "rgba(140,160,200,0.1)" } },
      y: {
        min: AXIS_LIMITS.light.min,
        max: AXIS_LIMITS.light.max,
        ticks: { color: "#a9bbde", stepSize: 250 },
        grid: { color: "rgba(140,160,200,0.12)" },
      },
    },
  },
});

function selectActiveDevice(devices) {
  const ids = Object.keys(devices || {});
  if (ids.length === 0) return null;
  ids.sort((a, b) => {
    const ta = new Date(devices[a].meta?.last_update || 0).getTime();
    const tb = new Date(devices[b].meta?.last_update || 0).getTime();
    return tb - ta;
  });
  return { id: ids[0], data: devices[ids[0]] };
}

function render(payload) {
  const selected = selectActiveDevice(payload?.devices || {});
  if (!selected) {
    statusPillEl.textContent = "Waiting for data";
    systemAlarmEl.innerHTML = kvRows([["Connection", "-", "value"]]);
    envSnapshotEl.innerHTML = kvRows([["Environment", "-", "value"]]);
    metaEl.innerHTML = kvRows([["MQTT", "No messages received yet", "value"]]);
    return;
  }

  const { id, data } = selected;
  const env = data.environment || {};
  const sys = data.system || {};
  const alarm = data.alarm || {};
  const meta = data.meta || {};

  statusPillEl.textContent = `${id} - ${sys.status || "unknown"}`;
  statusPillEl.style.color = sys.status === "online" ? "#4cd97b" : "#ff5f73";

  systemAlarmEl.innerHTML = kvRows([
    ["System OK", String(!!sys.system_ok), sys.system_ok ? "value ok" : "value bad"],
    ["Degraded Mode", String(!!sys.degraded_mode), sys.degraded_mode ? "value bad" : "value ok"],
    ["WiFi Connected", String(!!sys.wifi_connected), sys.wifi_connected ? "value ok" : "value bad"],
    ["MQTT Connected", String(!!sys.mqtt_connected), sys.mqtt_connected ? "value ok" : "value bad"],
    ["Motion Alarm (AS312)", String(!!alarm.as312_alarm), alarm.as312_alarm ? "value bad" : "value ok"],
    ["Gas Alarm (MICS5524)", String(!!alarm.mics5524_alarm), alarm.mics5524_alarm ? "value bad" : "value ok"],
    ["CO2 Alarm Level", normalizeCo2Level(alarm.co2_alarm_level), co2LevelClass(alarm.co2_alarm_level)],
  ]);

  envSnapshotEl.innerHTML = kvRows([
    ["CO2 (ppm)", formatNumber(env.co2_ppm)],
    ["Temperature Main (C)", formatNumber(env.temperature_c)],
    ["Temperature SCD40 (C)", formatNumber(env.temperature_scd40)],
    ["Temperature BMP280 (C)", formatNumber(env.bmp280_temperature_c)],
    ["Humidity Main (%)", formatNumber(env.humidity_percent)],
    ["Humidity SCD40 (%)", formatNumber(env.humidity_scd40)],
    ["Pressure (hPa)", formatNumber(env.bmp280_pressure_hpa)],
    ["VOC Index", formatNumber(env.voc_index)],
    ["NOx Index", formatNumber(env.nox_index)],
    ["PM1.0 (ug/m3)", formatNumber(env.pm1_0_ug_m3)],
    ["PM2.5 (ug/m3)", formatNumber(env.pm2_5_ug_m3)],
    ["PM10 (ug/m3)", formatNumber(env.pm10_ug_m3)],
    ["Gas (ppm)", formatNumber(env.gas_ppm)],
    ["Noise (dB)", formatNumber(env.noise_db)],
  ]);

  metaEl.innerHTML = kvRows([
    ["Last Topic", meta.last_topic || "-"],
    ["Last Update", meta.last_update || "-"],
    ["Parse Error", meta.parse_error || "none"],
    ["Light Channels", getLightChannels(env).map((v) => formatNumber(Number(v), 0)).join(", ")],
    ["Trend Queue (last 10)", trendDebugBuffer.join(" | ") || "-"],
  ]);

  const isEnvironmentUpdate = String(meta.last_topic || "").endsWith("/telemetry/environment");
  if (isEnvironmentUpdate) {
    pushSeries("", env);
    safeUpdateTrendChart();
  }

  const light = getLightChannels(env);
  const nextLightData = LIGHT_WAVELENGTHS_NM.map((_, i) => {
    const rawValue = Number(light[i] || 0);
    return Number.isFinite(rawValue) ? rawValue : 0;
  });
  const nextLightSignature = nextLightData.join("|");
  if (nextLightSignature !== lastLightSignature) {
    lastLightSignature = nextLightSignature;
    const maxLight = Math.max(...nextLightData, 0);
    spectrumChart.options.scales.y.min = 0;
    spectrumChart.options.scales.y.max = Math.max(100, Math.ceil(maxLight + 100));
    spectrumChart.data.datasets[0].data = nextLightData;
    spectrumChart.update("none");
  }
}

socket.on("device_update", (payload) => render(payload));
socket.on("connect", () => {
  statusPillEl.textContent = "Connected";
});
socket.on("disconnect", () => {
  statusPillEl.textContent = "Disconnected - reconnecting...";
  statusPillEl.style.color = "#ff5f73";
});
