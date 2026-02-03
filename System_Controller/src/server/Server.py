from __future__ import annotations

import os
import time
import threading
import subprocess
from collections import deque
from typing import Deque, Dict, Any
from pathlib import Path
import logging
import multiprocessing
import json
import re

from flask import Flask, jsonify, render_template, request
from ..helpers.Logger import setup_worker_logging, setup_listener
from ..helpers.Utils import read_from_file_lock_safe, write_to_file_lock_safe, split_and_parse_data

APP_DIR = os.path.dirname(os.path.abspath(__file__))
STATIC_DIR = os.path.join(APP_DIR, "static")
LATEST_JPG = os.path.join(STATIC_DIR, "latest.jpg")

# ---- Camera settings ----
CAM_DEVICE = "/dev/video0"
CAPTURE_INTERVAL_SEC = 5.0
CAPTURE_RESOLUTION = "1280x720"  # e.g. "640x480"
FSWEBCAM_TIMEOUT_SEC = CAPTURE_INTERVAL_SEC + 3

# ---- Simple in-memory state ----
logs: Deque[str] = deque(maxlen=300)
metrics: Deque[Dict[str, Any]] = deque(maxlen=300)
start_time = time.time()

# Telem data path
ROOT = Path(__file__).parent.parent
DATA_BUS_PATH = ROOT / "data_bus"

app = Flask(__name__)


def write_to_web_log(msg: str) -> None:
    ts = time.strftime("%H:%M:%S")
    logs.appendleft(f"[{ts}] {msg}\n")

def configure_camera(dev):
    # Make exposure manual so exposure_time_absolute is not "inactive"
    subprocess.run(["v4l2-ctl", "-d", dev, "--set-ctrl=auto_exposure=1"],
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    # Pick a starting exposure (tune this)
    subprocess.run(["v4l2-ctl", "-d", dev, "--set-ctrl=exposure_time_absolute=10"],
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    # Optional: small brightness lift
    subprocess.run(["v4l2-ctl", "-d", dev, "--set-ctrl=brightness=0"],
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    # Optional: if backlit scenes
    subprocess.run(["v4l2-ctl", "-d", dev, "--set-ctrl=backlight_compensation=0"],
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

def capture_loop() -> None:
    """
    Periodically capture a JPEG from the USB webcam using fswebcam.
    Writes to static/latest.jpg (atomic-ish replace).
    """
    configure_camera(CAM_DEVICE)

    os.makedirs(STATIC_DIR, exist_ok=True)
    tmp_path = os.path.join(STATIC_DIR, "latest.tmp.jpg")

    logging.info("Camera capture thread started.")
    while True:
        try:
            # fswebcam notes:
            # -d selects device, -r resolution, --jpeg quality, --no-banner removes overlay
            cmd = [
                "fswebcam",
                "-d", CAM_DEVICE,
                "-r", CAPTURE_RESOLUTION,
                # "--no-banner",
                "--jpeg", "85",
                "--delay", "0.5",
                "-S", "5",
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
            min_bytes = 20_000  # minimal valid JPEG size
            if os.path.exists(tmp_path) and os.path.getsize(tmp_path) >= min_bytes:
                os.replace(tmp_path, LATEST_JPG)
            else:
                logging.error("Captured image is too small, skipping update.")

        except subprocess.TimeoutExpired:
            logging.error("Camera capture timed out (fswebcam).")
        except FileNotFoundError:
            logging.error("fswebcam not found. Install it: sudo apt install fswebcam")
        except Exception as e:
            logging.error(f"Camera capture error: {e!r}")

        time.sleep(CAPTURE_INTERVAL_SEC)


def metrics_loop() -> None:
    """
    Generates example metrics for the chart.
    Replace this with your real sensor readings / values.
    """
    logging.info("Metrics thread started.")

    while True:
        t = time.time() - start_time
        telem_data = read_from_file_lock_safe(DATA_BUS_PATH / "slave_telem.txt")
        if telem_data:
            write_to_web_log(telem_data)
            parsed_data = split_and_parse_data(telem_data)

            value = parsed_data.get("moisture_raw_54") 
            metrics.append({"t": time.time(), "value": value})
        time.sleep(10.0)


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/api/action", methods=["POST"])
def action():
    data = request.get_json(silent=True) or {}
    name = str(data.get("name", "unknown"))
    logging.info(f"Button pressed: {name}")
    write_to_file_lock_safe(DATA_BUS_PATH / "slave_command.txt", name)
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

def main(log_queue=None):

    if log_queue is None:
        log_queue = multiprocessing.Queue(-1)
        setup_listener(log_queue)

    setup_worker_logging(log_queue)
    logging.info("Running Server app...")

    start_background_threads()
    app.run(host="0.0.0.0", port=5000, debug=False)



if __name__ == "__main__":
    main()