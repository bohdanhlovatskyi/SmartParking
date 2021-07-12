

 nrf docs: https://maniacbug.github.io/RF24/classRF24.html#a4cd4c198a47704db20b6b5cf0731cd58

 - без антени: до 30 метрів в приміщенні, та 100 ззовні
 - один прийомник на 6 передатчиків
 - За один сеанс можно отправить в буфер 32 байта.
 - consumes 26 µA in standby mode and 900 nA at power down mode
 - 12 mA during transmission at 0 dBm, which is even lower than a single LED.
 - a network of 125 independently working modems in one place (2.4 - 2.525 GHz)
 - Each node can have up to 5 children, and this can go 5 levels deep, which means we can create a network of total 3125 nodes. Each node must be defined with a 15-bit address, which precisely describes the position of the node within the tree.
 -  
