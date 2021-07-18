- Esp code is written in the depricated library version, so it would be nice to rewrite (meh...)
- later on: do the mesh system (relay node that collects the data from 5 nodes via meshnetwork library into allocated buffer and sends it to te gateway (nodemcu that will post it as json (sketch already contains the exaple of json arrays sending)), meh...)


BLE BASED:
- Measured power (?)
- How does the power of RSSI change from sensor to sensor, how does it change with the accumulator charge?
- Reread the papers on the RSSI to distance
- Take more sensors and try to collect some kind of dataset, so to be able to analyse it while being in Kiev
- !!! Check power consumption and get the optimal advertising interval
- How to measure coincell battery power that isr remaining?
- conduct several little experiments: RSSI at 1 meter with different conditions (different sensors, batteries, adv etc)
- Why there is more scanning data than transmitted one?
