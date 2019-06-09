// Copyright 2018 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jvsio/NanoClient.cpp"
#include "jvsio/JVSIO.cpp"

// PIO pins used by JVSIO
//  RX0 - JVS Data+
//  D2  - JVS Data-
//  D3  - JVS Sense
//  D13 - LED

// PIO pins used by 346
//  A0  - SS s0 [out]
//  A1  - SS s1 [out]
//  A2  - SS d0 [in]
//  A3  - SS d1 [in]
//  A4  - SS d2 [in]
//  A5  - SS d3 [in]
//  D4  - Coin  [in]

NanoDataClient data;
NanoSenseClient sense;
NanoLedClient led;
JVSIO io(&data, &sense, &led);

static const char gamepad_id[] = "SEGA ENTERPRISES,LTD.compat;IONA-346-NANO;ver1.00;Gamepad Mode";
static const char twinstick_id[] = "SEGA ENTERPRISES,LTD.compat;IONA-346-NANO;Ver1.00;Twin-Stick Mode";

uint8_t coin_count = 0;
uint8_t coin = 0;
uint16_t pad = 0xffff;

bool twinstick_mode = true;

void strobe(int s) {
  digitalWrite(A0, (s & 1) ? HIGH : LOW);
  digitalWrite(A1, (s & 2) ? HIGH : LOW);
}

int peek() {
  return (
    (digitalRead(A2) << 0) |
    (digitalRead(A3) << 1) |
    (digitalRead(A4) << 2) |
    (digitalRead(A5) << 3));
}

void updateState() {
  // Update coin.
  uint8_t new_coin = digitalRead(4);
  if (coin && !new_coin)
    coin_count++;
  coin = new_coin;

  // Update pad state.
  //    F  E  D  C  B  A  9  8  7  6  5  4  3  2  1  0  bit/mode
  //  [ R  X  Y  Z  S  A  C  B  >  <  v  ^  L  1  0  0] gamepad
  //  [LB R< R^ R>  S RT RB Rv L> L< Lv L^ LT  1  0  0] twinstick
  strobe(0);
  pad = peek() << 12;
  strobe(1);
  pad |= peek() << 8;
  strobe(2);
  pad |= peek() << 4;
  strobe(3);
  pad |= peek();

  // Update mode.
  // - disconnected, reset to default
  if ((pad & 0x0007) != 4)
    twinstick_mode = true;
  // - impossible stick combination results in gamepad mode
  if (((pad & 0x5000) == 0) || ((pad & 0x2100) == 0))
    twinstick_mode = false;
}

uint8_t report(size_t player, size_t line) {
  if (twinstick_mode) {
    //       |  B7 |  B6 |  B5 |  B4 |  B3 |  B2 |  B1 |  B0 |
    // ------+-----+-----+-----+-----+-----+-----+-----+-----+
    // P0-L1 |Start|  -  | L U | L D | L L | L R |L sht|L trb|
    // P0-L2 |  -  |  -  |  -  |  -  |  -  |  -  |  -  |  -  |
    // P1-L1 |  -  |  -  | R U | R D | R L | R R |R sht|R trb|
    // P1-L2 |  -  |  -  |  -  |  -  |  -  |  -  |  -  |  -  |
    if (player > 1 || line != 1)
      return 0x00;
    if (player == 0) {
      return ~(0x40 |
          ((pad & 0x0800) >>  4) |
          ((pad & 0x0010) <<  1) |
          ((pad & 0x0020) >>  1) |
          ((pad & 0x0040) >>  3) |
          ((pad & 0x0080) >>  5) |
          ((pad & 0x0008) >>  2) |
          ((pad & 0x8000) >> 15));
    } else {
      return ~(0xc0 |
          ((pad & 0x2000) >>  8) |
          ((pad & 0x0100) >>  4) |
          ((pad & 0x4000) >> 11) |
          ((pad & 0x1000) >> 10) |
          ((pad & 0x0400) >>  9) |
          ((pad & 0x0200) >>  9));
    }
  } else {
    //       |  B7 |  B6 |  B5 |  B4 |  B3 |  B2 |  B1 |  B0 |
    // ------+-----+-----+-----+-----+-----+-----+-----+-----+
    // P0-L1 |Start|  -  | Up  |Down |Left |Right|  A  |  B  |
    // P0-L2 |  C  |  X  |  Y  |  Z  |  L  |  R  |  -  |  -  |
    // P1-L1 |  -  |  -  |  -  |  -  |  -  |  -  |  -  |  -  |
    // P1-L2 |  -  |  -  |  -  |  -  |  -  |  -  |  -  |  -  |
    if (player != 0 || line > 2)
      return 0x00;
    if (line == 1) {
      return ~(0x40 |
          ((pad & 0x0800) >>  4) |
          ((pad & 0x0010) <<  1) |
          ((pad & 0x0020) >>  1) |
          ((pad & 0x0040) >>  3) |
          ((pad & 0x0080) >>  5) |
          ((pad & 0x0400) >>  9) |
          ((pad & 0x0100) >>  8));
    } else {
      return ~(0x03 |
          ((pad & 0x0200) >>  2) |
          ((pad & 0x4000) >>  8) |
          ((pad & 0x2000) >>  8) |
          ((pad & 0x1000) >>  8) |
          ((pad & 0x0008) >>  0) |
          ((pad & 0x8000) >> 13));
    }
  }
}

void setup() {
  io.begin();

  // Initialize output pins.
  {
    const int out_pins[] = { A0, A1 };
    for (size_t i = 0; i < sizeof(out_pins); ++i)
      pinMode(out_pins[i], OUTPUT);
  }
  strobe(0);

  // Initialize input pins.
  {
    const int in_pins[] = { 4, A2, A3, A4, A5 };
    for (size_t i = 0; i < sizeof(in_pins); ++i)
      pinMode(in_pins[i], INPUT_PULLUP);
  }
}

void loop() {
  uint8_t len;
  uint8_t* data = io.getNextCommand(&len);
  if (!data) {
    updateState();
    return;
  }
  switch (*data) {
   case JVSIO::kCmdIoId:
    io.pushReport(JVSIO::kReportOk);
    {
      const char* id = twinstick_mode ? twinstick_id : gamepad_id;
      for (size_t i = 0; id[i]; ++i)
        io.pushReport(id[i]);
    }
    io.pushReport(0);

    // Initialize.
    coin_count = 0;
    break;
   case JVSIO::kCmdFunctionCheck:
    io.pushReport(JVSIO::kReportOk);

    io.pushReport(0x01);  // sw
    io.pushReport(0x02);  // players
    io.pushReport(0x0C);  // buttons
    io.pushReport(0x00);

    io.pushReport(0x03);  // analog inputs
    io.pushReport(0x08);  // channels
    io.pushReport(0x00);  // bits
    io.pushReport(0x00);

    io.pushReport(0x12);  // general purpose driver
    io.pushReport(0x08);  // slots
    io.pushReport(0x00);
    io.pushReport(0x00);

    io.pushReport(0x02);  // coin
    io.pushReport(0x02);  // slots
    io.pushReport(0x00);
    io.pushReport(0x00);

    io.pushReport(0x00);
    break;
   case JVSIO::kCmdSwInput:
    io.pushReport(JVSIO::kReportOk);
    io.pushReport(0x00);  // TEST, TILT1-3, and undefined x4.
    for (size_t player = 0; player < data[1]; ++player) {
      for (size_t line = 1; line <= data[2]; ++line)
        io.pushReport(report(player, line));
    }
    break;
   case JVSIO::kCmdCoinInput:
    io.pushReport(JVSIO::kReportOk);
    for (size_t slot = 0; slot < data[1]; ++slot) {
      io.pushReport((0 << 6) | 0);
      io.pushReport(slot ? 0x00 : coin_count);
    }
    break;
   case JVSIO::kCmdAnalogInput:
    io.pushReport(JVSIO::kReportOk);
    for (size_t channel = 0; channel < data[1]; ++channel) {
      io.pushReport(0x80);
      io.pushReport(0x00);
    }
    break;
   case JVSIO::kCmdCoinSub:
    if (!data[1])
      coin_count -= data[3];
    io.pushReport(JVSIO::kReportOk);
    break;
   case JVSIO::kCmdDriverOutput:
    io.pushReport(JVSIO::kReportOk);
    break;
  }
}
