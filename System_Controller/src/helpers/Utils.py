import fcntl
import logging

@staticmethod
def read_from_file_lock_safe(file_path):
        try:
            with open(file_path, 'r+') as f:
                # Acquire an exclusive lock on the file
                fcntl.flock(f, fcntl.LOCK_EX)

                # Read data from the file
                data = f.read()

                # Clear the file after reading
                f.truncate(0)

                # Unlock the file
                fcntl.flock(f, fcntl.LOCK_UN)

                return data
        except Exception as e:
            logging.info(f"ERROR: read from file: {e}")

@staticmethod
def write_to_file_lock_safe(file_path, data):
    try:
        with open(file_path, 'w') as f:
            # Acquire an exclusive lock on the file
            fcntl.flock(f, fcntl.LOCK_EX)

            # Write data to the file
            f.write(data)

            # Flush the file buffer to ensure data is written to disk
            f.flush()

            # Unlock the file
            fcntl.flock(f, fcntl.LOCK_UN)

    except Exception as e:
        logging.info(f"ERROR: write to file: {e}")


@staticmethod
def split_and_parse_data(line):
    line = line.replace("{", "")
    line = line.replace("}", "")

    print(line)
    output = {}
    split_data = line.split(", ")
    for item in split_data:
        key, value = item.split(": ")
        key = key.strip()
        value = value.strip()
        value = value.replace(",", "")
        if value.isnumeric():
            value = int(value)
        elif "." in value:
            value = float(value)
        output[key] = value
    return output