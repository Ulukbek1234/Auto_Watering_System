import logging
import logging.handlers
from multiprocessing import Queue

def setup_worker_logging(log_queue):
    """
    Call this at the start of each worker process.
    """
    handler = logging.handlers.QueueHandler(log_queue)
    root = logging.getLogger()
    root.setLevel(logging.DEBUG)
    root.addHandler(handler)

def setup_listener(log_queue, log_file="master.log"):
    """
    Call this in the main process to start the logging listener.
    """
    # Console handler
    console_handler = logging.StreamHandler()
    console_handler.setFormatter(logging.Formatter(
        "[%(asctime)s] [%(levelname)s] [%(processName)s/%(threadName)s] %(message)s"
    ))

    # File handler
    file_handler = logging.FileHandler(log_file)
    file_handler.setFormatter(logging.Formatter(
        "[%(asctime)s] [%(levelname)s] [%(processName)s/%(threadName)s] %(message)s"
    ))

    listener = logging.handlers.QueueListener(
        log_queue, console_handler, file_handler
    )
    listener.start()
    return listener
