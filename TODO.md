- Flask server should support two ways of data receiving - threading one with serial and the wifi one with esp
- Esp code is written in the depricated library version, so it would be nice to rewrite
- NODEMCU does not start the nRF module (I guess that the problem is with the wiring)
- finifsh the `/postjson` function -> should fill the general queue that the `/update` then displays
- later on: do the mesh system (relay node that collects the data from 5 nodes via meshnetwork library into allocated buffer and sends it to te gateway (nodemcu that will post it as json (sketch already contains the exaple of json arrays sending)))


BLE BASED:
- Measured power (?)
- How does the power of RSSI change from sensor to sensor, how does it change with the accumulator charge?
- Watch one more time all the Cypress' videos on BLE
- Reread the papers on the RSSI to distance
- Take more sensors and try to collect some kind of dataset, so to be able to analyse it while being in Kiev
