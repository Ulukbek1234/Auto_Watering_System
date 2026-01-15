#!/usr/bin/env python3
import json
import os
import threading
import time
from collections import deque
from typing import Any, Dict, Optional

import serial
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from fastapi.responses import HTMLResponse

# -----------------------
# Configuration
# -----------------------
SERIAL_PORT = os.getenv("SERIAL_PORT", "/dev/ttyACM0")  # common for Arduino over USB
BAUD_RATE = int(os.getenv("BAUD_RATE", "9600"))
READ_TIMEOUT_S = float(os.getenv("READ_TIMEOUT_S", "1.0"))

# keep last N messages for /api/history
HISTORY_MAXLEN = int(os.getenv("HISTORY_MAXLEN", "2000"))

# optional "poor man's auth" for commands
# set: export API_TOKEN="something-long"
API_TOKEN = os.getenv("API_TOKEN", "")

# -----------------------
# Shared state
# -----------------------
latest_lock = threading.Lock()
latest_status: Dict[str, Any] = {
    "connected": False,
    "last_update_unix": None,
    "data": None,
    "error": None,
}

history_lock = threading.Lock()
history = deque(maxlen=HISTORY_MAXLEN)  # each item: {"ts":..., "data":...}

serial_lock = threading.Lock()
ser: Optional[serial.Serial] = None

# -----------------------
# Serial handling
# -----------------------
def open_serial() -> serial.Serial:
    s = serial.Serial(
        SERIAL_PORT,
        BAUD_RATE,
        timeout=READ_TIMEOUT_S,
    )
    # give Arduino time to reset after opening USB serial
    time.sleep(2.0)
    return s

def set_latest(connected: bool, data: Any = None, error: Optional[str] = None):
    with latest_lock:
        latest_status["connected"] = connected
        latest_status["last_update_unix"] = time.time() if data is not None else latest_status["last_update_unix"]
        latest_status["data"] = data if data is not None else latest_status["data"]
        latest_status["error"] = error

def serial_reader_loop():
    global ser
    backoff = 1.0

    while True:
        try:
            if ser is None or not ser.is_open:
                set_latest(False, error=f"Opening serial {SERIAL_PORT} @ {BAUD_RATE} ...")
                s = open_serial()
                with serial_lock:
                    ser = s
                set_latest(True, error=None)
                backoff = 1.0

            # Read one line (Arduino should send newline-terminated lines)
            line = ser.readline()
            if not line:
                # no data this cycle
                continue

            try:
                text = line.decode("utf-8", errors="replace").strip()
            except Exception:
                continue

            if not text:
                continue

            # Expect JSON per line. Example:
            # {"temp":23.4,"ph":6.8,"pump1":1}
            try:
                data = json.loads(text)
            except json.JSONDecodeError:
                # If you want, store raw lines too; for now just ignore noisy lines
                continue

            ts = time.time()
            set_latest(True, data=data, error=None)

            with history_lock:
                history.append({"ts": ts, "data": data})

        except Exception as e:
            # Serial unplugged or port wrong: close and retry with backoff
            set_latest(False, error=f"Serial error: {e}")
            try:
                with serial_lock:
                    if ser is not None:
                        ser.close()
            except Exception:
                pass
            ser = None
            time.sleep(backoff)
            backoff = min(backoff * 2.0, 30.0)

def send_serial_line(text: str) -> None:
    """
    Send a command line to Arduino, newline-terminated.
    """
    global ser
    with serial_lock:
        if ser is None or not ser.is_open:
            raise RuntimeError("Serial not connected")
        ser.write((text.strip() + "\n").encode("utf-8"))
        ser.flush()

# Start background reader thread
threading.Thread(target=serial_reader_loop, daemon=True).start()

# -----------------------
# FastAPI app
# -----------------------
app = FastAPI(title="Arduino Mega Gateway", version="1.0")

class CommandRequest(BaseModel):
    command: str
    token: Optional[str] = None

@app.get("/", response_class=HTMLResponse)
def index():
    # Tiny dashboard: polls /api/status every 1s
    return """
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>Arduino Mega Status</title>
  <style>
    body { font-family: system-ui, sans-serif; margin: 24px; }
    pre { background: #f6f6f6; padding: 16px; border-radius: 10px; overflow:auto; }
    .ok { color: #0a7; font-weight: 600; }
    .bad { color: #c33; font-weight: 600; }
  </style>
</head>
<body>
  <h1>Arduino Mega Status</h1>
  <div id="conn"></div>
  <pre id="out">Loading...</pre>

<script>
async function tick() {
  try {
    const r = await fetch('/api/status', { cache: 'no-store' });
    const j = await r.json();
    const conn = document.getElementById('conn');
    conn.innerHTML = j.connected
      ? '<span class="ok">Connected</span>'
      : '<span class="bad">Disconnected</span>';
    document.getElementById('out').textContent = JSON.stringify(j, null, 2);
  } catch (e) {
    document.getElementById('conn').innerHTML = '<span class="bad">Error</span>';
  }
}
tick();
setInterval(tick, 1000);
</script>
</body>
</html>
"""

@app.get("/api/status")
def api_status():
    with latest_lock:
        # Return a copy so we don't race
        return dict(latest_status)

@app.get("/api/history")
def api_history(seconds: int = 60):
    """
    Return history items from the last `seconds` seconds.
    """
    cutoff = time.time() - max(1, seconds)
    with history_lock:
        items = [h for h in history if h["ts"] >= cutoff]
    return {"seconds": seconds, "count": len(items), "items": items}

@app.post("/api/command")
def api_command(req: CommandRequest):
    """
    Send a command to Arduino.
    Example command strings:
      - "PUMP 1 ON"
      - "PUMP 1 OFF"
      - "SET RATE 120"
    Your Arduino firmware must parse these.
    """
    if API_TOKEN:
        if req.token != API_TOKEN:
            raise HTTPException(status_code=401, detail="Invalid token")

    if not req.command or len(req.command) > 200:
        raise HTTPException(status_code=400, detail="Bad command")

    try:
        send_serial_line(req.command)
    except Exception as e:
        raise HTTPException(status_code=503, detail=f"Serial send failed: {e}")

    return {"ok": True, "sent": req.command, "ts": time.time()}
