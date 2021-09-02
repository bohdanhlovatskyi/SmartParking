import asyncio
import math
import scipy.stats as stats
import pandas as pd
import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt
from bleak import BleakScanner
from time import gmtime, strftime

DETECTED_DEVICES = dict()
FILE_TO_SAVE = "parking_other_lot_closer_to_the_source.csv"

def detection_callback(device, _):
    if device.name is not None and device.name.startswith("SmartParking"):
        # print(device.name, "RSSI:", device.rssi)
        cur_time = strftime("%H:%M:%S", gmtime())
        try:
            DETECTED_DEVICES[cur_time]
        except KeyError:
            DETECTED_DEVICES[cur_time] = (device.name, device.rssi)
            with open(FILE_TO_SAVE, "a") as file:
                file.write(f"{cur_time};{device.name};{device.rssi}\n")


async def run():
    scanner = BleakScanner()
    scanner.register_detection_callback(detection_callback)
    await scanner.start()
    await asyncio.sleep(2.0)
    await scanner.stop()

    for key in DETECTED_DEVICES:
        # print(key, DETECTED_DEVICES[key])
        pass

def get_path_loss_index(tx_benchmark_power: int, RSSI: int,
                        benchmark_distance: float, RSSI_measured_distance: float):
    '''
    determines n - path loss index which depends on the
    propagation environment
    '''

    return (tx_benchmark_power - RSSI) / (10 * math.log(RSSI_measured_distance/benchmark_distance))


def get_distance(RSSI: int, tx_power: int, n: float): 
    '''
    Uses Log-distance path loss model
    '''
    
    return 10**((tx_power -  RSSI) / (10 * n))

def gets_stats(path_to_file: str, device: str):
    df = pd.read_csv(path_to_file, sep=";")
    # print(df.head())
    
    sens_rssi = df[df["DEVICE"] == device]["RSSI"].values.tolist()
    sns.displot(sens_rssi)
    plt.show()
    
    MEAN_POWER = np.array(sens_rssi).mean()
    MODE_POWER = stats.mode(sens_rssi)[0][0]
    # print(f"{MEAN_POWER=}")
    # print(f"{MODE_POWER=}")
    
    return MEAN_POWER


if __name__ == "__main__":
    with open(FILE_TO_SAVE, "a") as file:
        file.write("TIME;DEVICE;RSSI\n")
    loop = asyncio.get_event_loop()
    for i in range(500):
        print(f"...{i}") 
        loop.run_until_complete(run())
    # gets_stats(FILE_TO_SAVE, "SmartParking1")
