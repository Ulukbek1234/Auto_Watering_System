import serial
import time
import json
import logging
import multiprocessing 
from ..helpers.Utils import read_from_file_lock_safe, write_to_file_lock_safe, split_and_parse_data
from ..helpers.Logger import setup_listener, setup_worker_logging
from pathlib import Path



telem_dict_data = {}

class SerialComms:
    # For synchronization TODO better solution
    MOLA_DELAY = 0.5  # seconds

    def __init__(self):
        # Establish serial connection with Slave
        # USB = ttyACM0
        # GPIO = serial0 
        self.ser = serial.Serial(port='/dev/ttyACM0', baudrate=9600, timeout=1) 
        time.sleep(2)  # wait for the serial connection to initialize
        self.ser.write(b"SYNCH\n")

        # For data bus
        ROOT = Path(__file__).parent.parent
        DATA_BUS_PATH = ROOT / "data_bus"
        logging.debug(f"DATA_BUS_PATH: {DATA_BUS_PATH}")

        # For telemetry data
        self.TELEM_INTER = 15 # telemetry data gets requested every 30 seconds //TODO change back to 30 after debugging
        self.TELEM_PATH = DATA_BUS_PATH / "slave_telem.txt"

        # For boat commands
        self.SLAVE_COMMAND_INTER = 1
        self.SLAVE_COMMAND_PATH = DATA_BUS_PATH / "slave_command.txt"


    def check_response(self, line, command):
        if command in line:
            return True
        else:
            return False

    def request_telemetry(self):
        logging.info("Getting telem data from Mega")
        
        # Send request for telemetry data
        # ser.write(json.dumps({"Command": 250}).encode())  # 250 == GET_TELEM_DATA
        self.ser.write(b"CMD: TELEM,\n")
        time.sleep(self.MOLA_DELAY)  # Wait for the device to respond
        
        # Check if data is available in the serial buffer
        if self.ser.in_waiting > 0:
        # while ser.in_waiting > 0: # TODO: only for debugging, remove later
            line = self.ser.readline().decode('utf-8', errors='replace').rstrip()
            logging.debug(f"Telem data from Mega: {line}") 
            logging.debug(split_and_parse_data(line))
            return split_and_parse_data(line)
        else:
            logging.info("No telemetry data received.")


    def readAndSendCommandToBoat(self):
        logging.info("Getting boat commands and sending")
        boat_command = read_from_file_lock_safe(self.SLAVE_COMMAND_PATH)
        logging.debug(f"Boat command read: {boat_command}")
        
        # If the command file is empty, no need to send anything
        if len(boat_command.strip()) == 0:
            logging.debug("No boat command to send.")
            return
        
        # Clear output buffer before sending new command
        self.ser.flushOutput()

        # Send a command to the boat
        self.ser.write(boat_command.encode() + b'\n')
        time.sleep(self.MOLA_DELAY)

        # Check if a response is available from the boat
        if self.ser.in_waiting > 0:
            line = self.ser.readline().decode('utf-8').rstrip()
            logging.debug(f"Response from boat: {line}")
            # Need to check if its correct response
        else:
            logging.debug("No response from the boat.")

# TODO need to check if response is correct (synchronization issue)
def main(log_queue=None):
    if log_queue is None:
        log_queue = multiprocessing.Queue(-1)
        setup_listener(log_queue)

    setup_worker_logging(log_queue)

    logging.info("Running Serial connection...")
    serial_comms = SerialComms()

    last_telem_time = time.time()
    last_boat_command_time = time.time()
    while True:
        # Request telemetry data from the boat
        elapsed_telem_time = time.time() - last_telem_time
        if elapsed_telem_time > serial_comms.TELEM_INTER:
            last_telem_time = time.time()
            try:
                telem_dict_data = serial_comms.request_telemetry()
                if telem_dict_data != None:
                    write_to_file_lock_safe(serial_comms.TELEM_PATH, str(telem_dict_data))
            except Exception as e:
                logging.info(f"ERROR: request telemetry: {e}")

        # Send commands to the boat
        elapsed_boat_command_time = time.time() - last_boat_command_time
        if elapsed_boat_command_time > serial_comms.SLAVE_COMMAND_INTER:
            last_boat_command_time = time.time()
            try:
                serial_comms.readAndSendCommandToBoat()
            except Exception as e:
                logging.info(f"ERROR: read and send command to boat: {e}")


if __name__ == "__main__":
    main()