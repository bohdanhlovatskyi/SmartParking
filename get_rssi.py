import asyncio
from bleak import BleakScanner
from time import gmtime, strftime

DETECTED_DEVICES = dict()
MEASURED_POWER = -42
RSSI_COEFFICIENT = 4 # from 2 to 4

def detection_callback(device, _):
    if device.name is not None and device.name.startswith("SmartParking"):
        # print(device.name, "RSSI:", device.rssi)
        cur_time = strftime("%H:%M:%S", gmtime())
        DETECTED_DEVICES[cur_time] = (device.name, device.rssi)
        with open("test.csv", "a") as file:
            file.write(f"{cur_time};{device.name};{device.rssi}\n")

def get_distance(rssi: int):
    "TODO"

async def run():
    scanner = BleakScanner()
    scanner.register_detection_callback(detection_callback)
    await scanner.start()
    await asyncio.sleep(5.0)
    await scanner.stop()

    for key in DETECTED_DEVICES:
        # print(key, DETECTED_DEVICES[key])
        pass


if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    for _ in range(50):
        print("...") 
        loop.run_until_complete(run())
