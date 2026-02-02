import sys
import threading
import serial
import serial.tools.list_ports

BAUD = 9600

def pick_port():
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        print("No serial ports found. Is the Arduino plugged in?")
        sys.exit(1)

    print("Available ports:")
    for i, p in enumerate(ports, 1):
        print(f"  {i}) {p.device}  -  {p.description}")

    # If only one port, auto-pick
    if len(ports) == 1:
        return ports[0].device

    while True:
        choice = input("Select port number: ").strip()
        if choice.isdigit() and 1 <= int(choice) <= len(ports):
            return ports[int(choice) - 1].device

def reader(ser: serial.Serial):
    """Continuously read lines from Arduino and print them."""
    try:
        while ser.is_open:
            line = ser.readline()  # reads until '\n' or timeout
            if line:
                # Print exactly what Arduino sent
                print(line.decode(errors="replace").rstrip())
    except Exception as e:
        print(f"\n[reader stopped] {e}")

def main():
    port = pick_port()
    print(f"Opening {port} @ {BAUD} ...")

    # timeout lets readline return occasionally even if no data
    ser = serial.Serial(port, BAUD, timeout=1)

    # Many Arduinos reset on serial open; wait a moment for "READY"
    print("Connected. Type commands and press Enter. Ctrl+C to quit.\n")

    t = threading.Thread(target=reader, args=(ser,), daemon=True)
    t.start()

    try:
        while True:
            cmd = input("> ")
            # Send line with newline so Arduino sees end-of-command
            ser.write((cmd + "\n").encode())
    except (KeyboardInterrupt, EOFError):
        print("\nExiting...")
    finally:
        try:
            ser.close()
        except Exception:
            pass

if __name__ == "__main__":
    main()
