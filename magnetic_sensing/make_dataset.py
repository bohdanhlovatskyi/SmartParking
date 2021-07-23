import serial
import time
import tqdm.notebook as tqdm

PORTS = ["/dev/cu.usbserial-14220", "/dev/cu.usbserial-14240",
         "/dev/cu.usbserial-14120", "/dev/cu.usbserial-14140"]
serialport = None


def get_mag_data():
    global serialport
    if not serialport:
        # open serial port
        for port in PORTS:
            try:
                serialport = serial.Serial(port, 9600)
                break
            except Exception:
                continue
        if (not serialport):
            raise ValueError("Could not open the port")

        # check which port was really used
        print("Opened", serialport.name)

        # Flush input
        time.sleep(3)
        serialport.readline()

    # Poll the serial port

    # I assume, that there is some line that is not needed (empty one)
    serialport.readline()
    line = str(serialport.readline(), 'utf-8')
    if not line:
        raise ValueError('Line was not openned')
    vals = line.strip().split(';')[:-1]

    if len(vals) != 3:
        raise ValueError('Lenght of values is not enough')
    try:
        vals = [float(i) for i in vals]
    except ValueError:
        raise ValueError('Values cannot be converted')

    return vals

def log_mag(file_name: str, iters: int = 500):
    with open(file_name, "w") as file:
        for _ in tqdm.tqdm(range(iters)):
            x, y, z = get_mag_data()
            print(f'{x=}, {y=}, {z=}')
            file.write(f'{x};{y};{z}\n')


if __name__ == "__main__":
    import sys
    if len(sys.argv) != 3:
        raise ValueError("Usage: python3 make_dataset.py file_name num_of_sample")
    log_mag(sys.argv[-2], int(sys.argv[-1]))

