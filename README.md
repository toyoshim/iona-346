# いおな三四郎 JVS - Sega Saturn Gamepad I/O Bridge Board

This program makes an Arduino Nano work as a JVS I/O Board that allows you
to connect a Sega Saturn Gamepad to NAOMI. Twin-Stick also works for Virtual-On.

Default is Twin-Stick mode. It maps as it works for Virtual-On as is.

  JVS SWs     Twin-Stick
 1P Start  - Start
 1P Up     - Left Stick Up
 1P Down   - Left Stick Down
 1P Left   - Left Stick Left
 1P Right  - Left Stick Right
 1P Push 1 - Left Trigger
 1P Push 2 - Left Button
 2P Up     - Right Stick Up
 2P Down   - Right Stick Down
 2P Left   - Right Stick Left
 2P Right  - Right Stick Right
 2P Push 1 - Right Trigger
 2P Push 2 - Right Button

Once a combination of X and Z, or B and Y is detected, the mode will be changed
to normal Gamepad mode. You can not make any of these combinations by Twin-Stick
as it's right stick left and right, or up and down.

  JVS SWs     Twin-Stick
 1P Start  - Start
 1P Up     - Pad Up
 1P Down   - Pad Down
 1P Left   - Pad Left
 1P Right  - Pad Right
 1P Push 1 - A
 1P Push 2 - B
 1P Push 3 - C
 1P Push 4 - X
 1P Push 5 - Y
 1P Push 6 - Z
 1P Push 7 - L
 1P Push 8 - R

## Schematic
```

Arduino Nano

                         o TX1  VIN o--------------o 5V
JVS Data+ o--------------o RX0  GND o
                         o RST  RST o
                         o GND   5V o
JVS Data- o--------------o D2    A7 o
JVS Sense o---o---VVVV---o D3    A6 o
              |  100 Ohm
       100 nF =
              |
            -----
            /////
  Coin SW o--------------o D4    A5 o--------------o Saturn d3
          o--------------o D5    A4 o--------------o Saturn d2
          o--------------o D6    A3 o--------------o Saturn d1
          o--------------o D7    A2 o--------------o Saturn d0
          o--------------o D8    A1 o--------------o Saturn s1
          o--------------o D9    A0 o--------------o Saturn s0
          o--------------o D10 AREF o--------------o
          o--------------o D11  3V3 o
          o--------------o D12  D13 o--------------o LED on Nano


JVS - Series B connector (same with USB Type B)
  ________
 /        \
 | 1#  #2 |  1 Sense
 |  +--+  |  2 Data-
 |  +--+  |  3 Data+
 | 4#  #3 |  4 GND
 +--------+


Sega Saturn Gamepad IO (female - host side)
 _____________________________
/                             \ 1 GND  4 5V   7 d0
| 9# 8# 7# 6# 5# 4# 3# 2# 1#  | 2 d2   5 s1   8 d1
| =========================== | 3 d3   6 s0   9 5V
+-----------------------------+
```
