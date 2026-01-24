# Inital program to run entire watering system

""" 
Functional Requirements
    - Serial Communication with Mega
        -- Commands to Mega 
            --- Synchronize time
            --- Configure pumps (daily liters, add/ remove pumps)
        -- Telemetry from Mega
            --- {Date: DD/MM/YYYY, Time: 00:00, Zone: Pots, Nr_Pumps: 1, Nr_Soil_Hum: 0, 
                    {
                        Pumps_ID: 1, 
                        daily_liters: 0.5, // Maximum daily liters
                        current_liters: 0.1,
                        limit_reached: false,
                        soil_humidity: 0.5, // Percentage of humidity
                    }
                    {
                        Pumps_ID: 2,
                        daily_liters: 0.7,
                        current_liters: 0.1,
                        limit_reached: false,
                        soil_humidity: 0.5, // Percentage of humidity
                    }    
                }
    - Website
        -- Check current telemetry
            --- Over time
            --- Graphs 
        -- Add/ Remove pumps/ sensors
        -- Modify daily liters
    - Database 
        -- Access from anywhere?
        -- Logging of everything
"""


import sys
import subprocess
import time
import os
import json
import fcntl
from pprint import pformat
from .helpers.Utils import read_from_file_lock_safe, write_to_file_lock_safe, split_and_parse_data
from .helpers.Logger import setup_listener, setup_worker_logging

from pathlib import Path

        
def main():
    """Main program loop for the Mola Interface Program."""
    # Init Logging 
    log_queue = multiprocessing.Queue()
    listener = setup_listener(log_queue)
    setup_worker_logging(log_queue)
    logging.info("Starting Mola Interface Program...")

    interface = Interface()
    process_manager = ProcessManager(log_queue)
    
    # Example: Starting initial processes for system communications
    process_manager.start_process(
        "Serial", 
        ["python", "-m", "src.comms.Serial"]
    )

    # process_manager.start_process("WiFi", "python Comms/WiFi.py")
    # process_manager.start_process(
    #     "Client", 
    #     ["python", "-m", "src.comms.Client"]
    # )

    last_master_command_time = time.time()

    while True:
        # Read command for master and perform command
        elapsed_telem_time = time.time() - last_master_command_time
        master_command_parsed = {}
        if elapsed_telem_time > interface.MASTER_COMMAND_INTER:
            last_master_command_time = time.time()
            try:
                logging.debug(f"Perform read from master_command.txt file")
                interface.read_master_and_perform_command()
            except Exception as e:
                logging.debug(f"ERROR: performing master command: {e}")

if __name__ == "__main__":
    main()


