
so the thing with nRF24l01 is:
- as much as we want addresses (addresses will be included in the payload)
! they are hardware written (meh)

- master can listen on 6 reading pipes at the same time (collision avoidance?)

- all the slaves send data on the master's listening pipe address
- then it can send back acknowledge to the node with specific address (all the others will be ignored I assume)

So, generally it seems pretty ok to me
--------------------------------------
Motivation on external rtc: setup initial tick time at the square wave output via clock (but basically I did not really worked with that, as the external rtc was not set up, therefore the clock pulses are not synchronised within one second, so I can easilly drop it)

--------------------------------------
What remains?
only main clock that will fire an interrupt when it is time to conduct a transmission


GREAT! 
--------------------------------------
Different IOT RF Modules

- LoRaWAN
   


receiver sensistivity is a nice value to compare when we are lookig for radio devices

rf line of sight = fresnel Zone: optimal antenna height for range of 100 meters is 2.94m for 868 mhx radios and 1.77 for 2.4 ghz radios 

868 Mhx radio modules are those benefits: more range at same power output, better wall penetrarion, less crowded frequency


nRF24l01
- Enhanced ShockBurst packet structur: 1-32 bytes payload, providec each sent packet with packet ID (retransmission check), acknowledgement support 
nRF24L01 is a single chip radio transceiver for the world wide 2.4 - 2.5 GHz ISM band. The transceiver consists of a fully integrated frequency synthesizer, a power amplifier, a crystal oscillator, a demodulator, modulator and Enhanced ShockBurstTM protocol engine.

Current consumption is very low, only 9.0mA at an output power of -6dBm and 12.3mA in RX mode

Sensitivity: -85dBm (at 1000 kbps)
- Power Down mode support (900 nA supply current )

nRF24L01 will use the data pipe address when acknowledging a received packet. This means that nRF24L01 will transmit ACK with the same address as it receives payload at. In the PTX device data pipe 0 is used to received the acknowledgement, and therefore the receive address for data pipe 0 has to be equal to the transmit address to be able to receive the acknowledgement.
 data transfer rate can be one of 250kbps, 1Mbps and 2Mbps.
Programmable output power: 0, -6, -12 or -18dBm
- 1 - 2 dollars per module

The default channel for the RF24 library is 76.

pipe 0  is the only writing pipe

However each individual transmission is very short (a few millisecs) so that it is unlikely that the transmission from your TX will overlap with something else.

The dynamic payload mode is automatically applied when you choose the ackPayload feature

When using dynamic payloads you must ensure that you read all the bytes that are received or the communication will break down.


 nrf docs: https://maniacbug.github.io/RF24/classRF24.html#a4cd4c198a47704db20b6b5cf0731cd58

 - без антени: до 30 метрів в приміщенні, та 100 ззовні
 - один прийомник на 6 передатчиків
 - За один сеанс можно отправить в буфер 32 байта.
 - consumes 26 µA in standby mode and 900 nA at power down mode
 - 12 mA during transmission at 0 dBm, which is even lower than a single LED.
 - a network of 125 independently working modems in one place (2.4 - 2.525 GHz)
 - Each node can have up to 5 children, and this can go 5 levels deep, which means we can create a network of total 3125 nodes. Each node must be defined with a 15-bit address, which precisely describes the position of the node within the tree.


----------------------------
LoRaWAN
900/915 frequency
Sensitivity: -136 dBm
tx power: +14dBm
chipset consumption: 39 mA
RangeL >15km

uses LoRa on the physicall layer and LoRaWAN on the canal(?) level
5 kb / s

стоимость внедрения, стоимость обслуживания и выгода. 

Lora Shield that is compatable with Arduino: 17euro one module








