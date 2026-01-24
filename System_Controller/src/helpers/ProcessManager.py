import subprocess
import threading
import logging
import os

class ProcessManager:
    def __init__(self, log_queue=None):
        self.processes = {}
        self.log_queue = log_queue  # (not needed for subprocess)
    
    def _log_subprocess_output(self, name, stream, level=logging.INFO):
        for line in iter(stream.readline, b''):
            logging.log(level, f"[{name}] {line.decode().rstrip()}")
        stream.close()

    def start_process(self, name, command):
        if name in self.processes:
            logging.debug(f"Process {name} is already running.")
            return

        try:
            env = os.environ.copy()

            logging.debug(f"Starting process {name} with command: {command}")
            process = subprocess.Popen(
                command,          # LIST — OK
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=env
            )
            self.processes[name] = process
            logging.info(f"Started process {name} (PID: {process.pid})")

            threading.Thread(target=self._log_subprocess_output, args=(name, process.stdout, logging.INFO), daemon=True).start()
            threading.Thread(target=self._log_subprocess_output, args=(name, process.stderr, logging.ERROR), daemon=True).start()

        except Exception as e:
            logging.exception(f"Failed to start process {name}")

    def stop_process(self, name):
        if name not in self.processes:
            logging.debug(f"Process {name} not found.")
            return

        process = self.processes[name]
        process.terminate()
        try:
            process.wait(timeout=5)
            logging.info(f"Stopped process {name}")
        except subprocess.TimeoutExpired:
            process.kill()
            logging.info(f"Force killed process {name} after timeout.")
        finally:
            del self.processes[name]