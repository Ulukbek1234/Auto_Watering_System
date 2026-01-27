from __future__ import annotations

import os
import time
import threading
import subprocess
from collections import deque
from typing import Deque, Dict, Any

from flask import Flask, jsonify, render_template, request

APP_DIR = os.path.dirname(os.path.abspath(__file__))
STATIC_DIR = os.path.join(APP_DIR, "static")
LATEST_JPG = os.path.join(STATIC_DIR, "latest.jpg")

# ---- Camera settings ----
CAM_DEVICE = "/dev/video0"
CAPTURE_INTERVAL_SEC = 1.0
CAPTURE_RESOLUTION = "1280x720"  # e.g. "640x480"
FSWEBCAM_TIMEOUT_SEC = 3

# ---- Simple in-memory state ----
logs: Deque[str] = deque(maxlen=300)
metrics: Deque[Dict[str, Any]] = deque(maxlen=300)
start_time = time.time()

app = Flask(__name__)


def log(msg: str) -> None:
    ts = time.strftime("%H:%M:%S")
    logs.appendleft(f"[{ts}] {msg}")

def setup_camera():
    cmds = [
        ["v4l2-ctl", "-d", CAM_DEVICE, "--set-ctrl=auto_exposure=1"],
        ["v4l2-ctl", "-d", CAM_DEVICE, "--set-ctrl=exposure_time_absolute=4000"],
        ["v4l2-ctl", "-d", CAM_DEVICE, "--set-ctrl=gain=60"],
    ]
    for cmd in cmds:
        try:
            subprocess.run(cmd, check=True)
        except Exception as e:
            log(f"Camera setup failed: {e}")


def capture_loop() -> None:
    """
    Periodically capture a JPEG from the USB webcam using fswebcam.
    Writes to static/latest.jpg (atomic-ish replace).
    """
    os.makedirs(STATIC_DIR, exist_ok=True)
    tmp_path = os.path.join(STATIC_DIR, "latest.tmp.jpg")

    log("Camera capture thread started.")
    while True:
        try:
            # fswebcam notes:
            # -d selects device, -r resolution, --jpeg quality, --no-banner removes overlay
            cmd = [
                "fswebcam",
                "-d", CAM_DEVICE,
                "-r", CAPTURE_RESOLUTION,
                "--no-banner",
                "--jpeg", "85",
                "--save", tmp_path,
            ]
            subprocess.run(
                cmd,
                check=True,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                timeout=FSWEBCAM_TIMEOUT_SEC,
            )
            # Replace latest.jpg
            os.replace(tmp_path, LATEST_JPG)
        except subprocess.TimeoutExpired:
            log("Camera capture timed out (fswebcam).")
        except FileNotFoundError:
            log("fswebcam not found. Install it: sudo apt install fswebcam")
        except Exception as e:
            log(f"Camera capture error: {e!r}")

        time.sleep(CAPTURE_INTERVAL_SEC)


def metrics_loop() -> None:
    """
    Generates example metrics for the chart.
    Replace this with your real sensor readings / values.
    """
    log("Metrics thread started.")
    while True:
        t = time.time() - start_time
        # Example signal (change this!)
        value = 50 + 30 * (0.5 + 0.5 * __import__("math").sin(t / 3.0))
        metrics.append({"t": time.time(), "value": round(value, 2)})
        time.sleep(1.0)


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/api/action", methods=["POST"])
def action():
    data = request.get_json(silent=True) or {}
    name = str(data.get("name", "unknown"))
    log(f"Button pressed: {name}")
    # TODO: put your hardware actions here (GPIO, scripts, etc.)
    return jsonify({"ok": True, "received": name})


@app.route("/api/logs")
def api_logs():
    # Return the newest N lines (already stored newest-first)
    n = int(request.args.get("n", 50))
    n = max(1, min(n, 300))
    return jsonify({"lines": list(logs)[:n]})


@app.route("/api/metrics")
def api_metrics():
    # Return last N metric points oldest->newest for plotting
    n = int(request.args.get("n", 60))
    n = max(5, min(n, 300))
    pts = list(metrics)[-n:]  # metrics deque is oldest->newest in append order
    return jsonify({"points": pts})


def start_background_threads() -> None:
    threading.Thread(target=capture_loop, daemon=True).start()
    threading.Thread(target=metrics_loop, daemon=True).start()


if __name__ == "__main__":
    log("Starting Flask app...")
    setup_camera()
    start_background_threads()
    # host=0.0.0.0 makes it accessible from other devices on your LAN
    app.run(host="0.0.0.0", port=5000, debug=True)
