# Lanterne magique
for Hideyuki ISHIBASHI, le Fresnoy 2017

-----------------------
## Description

2 x [ FraiseVersa + 3 PWMdriven-leds 12V/0.7A]  
3 x [ Fraise8X2A + DC-motor + encoder + multi-turn-potentiometer ]  
host: raspberryPi3 with LCD displaying Pof "start" touch-button

network remote Pd patch ("client"):

- set leds/motors   
- run/stop/skip-steps/edit text sequences

## Installation
### Machine:

Install Pure Data (v >= 0.47-1)  
Install Fraise  
Install Pd externals:

- maxlib
- ggee
- pof

Download project zipfile (from github's page), extract.  
Open with Pd : 0main.pd

Configure the rPi: (optionnal)

- autostart "pd 0main.pd"
- read-only file-system
- WIFI access point:
	- network name: AP-Lanterne
	- password: raspberry
- static eth0 IP: 192.168.1.71

### Remote client:

Install Pure Data (v >= 0.47-1)

Download project zipfile (from github's page), extract.

Connect to the raspberryPi either through ethernet or to WIFI access point:

- network name: AP-Lanterne
- password: raspberry

Open with Pd : 0client.pd  

#### If the client doesn't connect to the machine:
  
In 0client.pd, open sub-patch named `pd guts`; locate the object:  
`PdClient CLIENT CLIENT raspberrypi.local`

In edit mode, change `raspberrypi.local` to the IP address you can ping the machine host. Try `192.168.3.1` if WIFI connected, or `192.168.1.71` if ethernet. 
Connection should be established almost instantly. Save the patch when working.

## Credits

Copyright (c) Antoine Rousseau <antoine@metalu.net> 2017  
GNU GENERAL PUBLIC LICENSE  
For information on usage and redistribution, and for a DISCLAIMER OF ALL WARRANTIES, see the file "LICENSE.txt" in this distribution.


