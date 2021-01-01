# wordklok

This is the brains for the infamous letter clock. Design is rougly based on this project: http://www.highonsolder.com/electronics/arduino-word-clock/

My project ueses simpler electrical design. There are three 74HC595 shift registers driving all 23 segments. Each segment is switched on by saturating one BC547 transistor.

ESP8266 chip provides captive portal configuration. It will use your default gateway as NTP server. This works great for me, beacuse my router has NTP service. If your router does not do NTP you may hard code NTP pool server instead if ne.

OTAP is supported as well. You are free to brick your device remotely.
