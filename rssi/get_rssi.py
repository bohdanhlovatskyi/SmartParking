import asyncio
from bleak import BleakScanner
from time import gmtime, strftime

DETECTED_DEVICES = dict()
FILE_TO_SAVE = "two_devices_one_meter.csv"

def detection_callback(device, _):
    if device.name is not None and device.name.startswith("SmartParking"):
        # print(device.name, "RSSI:", device.rssi)
        cur_time = strftime("%H:%M:%S", gmtime())
        DETECTED_DEVICES[cur_time] = (device.name, device.rssi)
        with open(FILE_TO_SAVE, "a") as file:
            file.write(f"{cur_time};{device.name};{device.rssi}\n")



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
    with open(FILE_TO_SAVE, "a") as file:
        file.write("TIME;DEVICE;RSSI\n")
    loop = asyncio.get_event_loop()
    for i in range(50):
        print(f"...{i}") 
        loop.run_until_complete(run())
