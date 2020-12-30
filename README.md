# wordklok

This is the program for infamous letter clock. Design is rougly based on this project: http://www.highonsolder.com/electronics/arduino-word-clock/

I used simpler electrical design. There are three 74HC595 shift registers driving each segment. Each segment is driven by one BC547 transistor. 

ESP8266 chip provides captive portal configuration. It will use your default gateway as NTP server. This works great, beacuse my router has NTP service. You may hard code NTP pool server instead if needed.

OTAP is also supported.
