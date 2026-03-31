const path = require("path");
const express = require("express");
const http = require("http");
const mqtt = require("mqtt");
const { Server } = require("socket.io");

require("dotenv").config();

const MQTT_BROKER_URL = process.env.MQTT_BROKER_URL;
const MQTT_USERNAME = process.env.MQTT_USERNAME;
const MQTT_PASSWORD = process.env.MQTT_PASSWORD;
const HTTP_PORT = Number(process.env.PORT || 3000);

const MQTT_TOPICS = [
  "devices/+/status/system",
  "devices/+/telemetry/environment",
  "devices/+/event/alarm",
];

const app = express();
const server = http.createServer(app);
const io = new Server(server);

app.use(express.static(__dirname));

const devices = {};
const alarmResetTimers = {};

function createInitialDeviceState() {
  return {
    system: {
      system_ok: false,
      degraded_mode: false,
      wifi_connected: false,
      mqtt_connected: false,
      status: "offline",
    },
    environment: {},
    alarm: {
      as312_alarm: false,
      mics5524_alarm: false,
      co2_alarm_level: "unknown",
    },
    meta: {
      last_topic: null,
      last_update: null,
      last_raw_payload: null,
      parse_error: null,
    },
  };
}

function parseDeviceId(topic) {
  const parts = topic.split("/");
  return parts.length >= 2 && parts[0] === "devices" ? parts[1] : "unknown";
}

function sanitizeJson(raw) {
  return raw
    .replace(/,\s*}/g, "}")
    .replace(/,\s*]/g, "]")
    .replace(/"status"\s*:\s*(online|offline)\b/g, '"status":"$1"')
    .replace(/"co2_alarm_level"\s*:\s*([^,}]+)/g, (_, rawLevel) => {
      const cleanLevel = String(rawLevel).trim().replace(/^"(.*)"$/, "$1");
      return `"co2_alarm_level":"${cleanLevel}"`;
    });
}

function parsePayload(raw) {
  try {
    return { ok: true, data: JSON.parse(raw) };
  } catch (_) {
    const repaired = sanitizeJson(raw);
    try {
      return { ok: true, data: JSON.parse(repaired) };
    } catch (error) {
      return { ok: false, error: error.message };
    }
  }
}

function buildBroadcastState() {
  return { devices };
}

function getDeviceTimers(deviceId) {
  if (!alarmResetTimers[deviceId]) {
    alarmResetTimers[deviceId] = {
      as312_alarm: null,
      mics5524_alarm: null,
    };
  }
  return alarmResetTimers[deviceId];
}

function setTimedAlarm(deviceId, alarmKey, shouldActivate) {
  const current = devices[deviceId];
  const deviceTimers = getDeviceTimers(deviceId);

  if (!Object.prototype.hasOwnProperty.call(current.alarm, alarmKey)) {
    return;
  }

  if (deviceTimers[alarmKey]) {
    clearTimeout(deviceTimers[alarmKey]);
    deviceTimers[alarmKey] = null;
  }

  if (!shouldActivate) {
    current.alarm[alarmKey] = false;
    return;
  }

  current.alarm[alarmKey] = true;
  deviceTimers[alarmKey] = setTimeout(() => {
    if (!devices[deviceId]) {
      return;
    }
    devices[deviceId].alarm[alarmKey] = false;
    io.emit("device_update", buildBroadcastState());
    deviceTimers[alarmKey] = null;
  }, ALARM_ON_MS);
}

const mqttClient = mqtt.connect(MQTT_BROKER_URL, {
  username: MQTT_USERNAME,
  password: MQTT_PASSWORD,
  reconnectPeriod: 2000,
});

mqttClient.on("connect", () => {
  console.log("Connected to MQTT broker");
  mqttClient.subscribe(MQTT_TOPICS, (err) => {
    if (err) {
      console.error("Subscribe error:", err.message);
      return;
    }
    console.log("Subscribed to MQTT topics:", MQTT_TOPICS.join(", "));
  });
});

mqttClient.on("reconnect", () => {
  console.log("Reconnecting to MQTT...");
});

mqttClient.on("error", (err) => {
  console.error("MQTT error:", err.message);
});

mqttClient.on("message", (topic, messageBuffer) => {
  const raw = messageBuffer.toString();
  const deviceId = parseDeviceId(topic);

  if (!devices[deviceId]) {
    devices[deviceId] = createInitialDeviceState();
  }

  const current = devices[deviceId];
  current.meta.last_topic = topic;
  current.meta.last_update = new Date().toISOString();
  current.meta.last_raw_payload = raw;
  current.meta.parse_error = null;

  const parsed = parsePayload(raw);
  if (!parsed.ok) {
    current.meta.parse_error = parsed.error;
    io.emit("device_update", buildBroadcastState());
    return;
  }

  if (topic.endsWith("/status/system")) {
    current.system = parsed.data;
  } else if (topic.endsWith("/telemetry/environment")) {
    current.environment = parsed.data;
  } else if (topic.endsWith("/event/alarm")) {
    const as312Active = parsed.data?.as312_alarm === true;
    const micsActive = parsed.data?.mics5524_alarm === true;
    if (typeof parsed.data?.co2_alarm_level === "string") {
      current.alarm.co2_alarm_level = parsed.data.co2_alarm_level;
    }
    setTimedAlarm(deviceId, "as312_alarm", as312Active);
    setTimedAlarm(deviceId, "mics5524_alarm", micsActive);
  }

  io.emit("device_update", buildBroadcastState());
});

io.on("connection", (socket) => {
  console.log("Browser connected");
  socket.emit("device_update", buildBroadcastState());
});

server.listen(HTTP_PORT, () => {
  console.log(`Dashboard available on http://localhost:${HTTP_PORT}`);
});