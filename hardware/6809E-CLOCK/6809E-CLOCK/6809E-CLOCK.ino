///////////////////////////////////////////
// two phased clock generator for 6809E
//
//  Copyright (c) 2021 by Y.Kuwata
///////////////////////////////////////////

// ATtiny202 with 16MHz internal Clock (MegaTinyCore)
// 
// PIN ASSIGN
//
//           +--U--+
//   Vcc 1  -|     |-  8  GND
//   E   2  -|     |-  7  Q
// RESET 3  -|     |-  6  (UPDI)
// SEL1  4  -|     |-  5  SEL2
//           +-----+
//
//   CLOCK SELECTION TABLE
//
//       SEL1   SEL2
// 1MHz   H      H
// 2MHz   L      H
// 3MHz   H      L
// FLOAT  L      L

#include <Logic.h>
#include <util/delay.h>

void setupLogic() {
  Logic0.enable = true;                 // Enable logic block 0
  Logic0.input0 = in::masked;           // mask input0 (NOT Use TCA0 WO0 as input 0)
  Logic0.input1 = in::tca0;             // Use TCA0 WO1 as input 1
  Logic0.input2 = in::tca0;             // Use TCA0 WO2 as input 2
  Logic0.output = out::enable;          // Enable logic block 0 output pin at PA6 (PIN2)
  Logic0.filter = filter::disable;      // delay 2 clock
  Logic0.truth = 0x3c;                  // Set truth table - ex-or 
  Logic0.init();   // Initialize logic block 0
}

#define ENABLE_LOGIC Logic::start()
#define DISABLE_LOGIC Logic::stop()

void setup1M() {
  pinMode(PIN_PA3, OUTPUT);           // PIN 7 for clock (WO0)
  TCA0.SINGLE.PER = 15;  // f = 16 / (1 + 15) = 1MHz 
  TCA0.SINGLE.CMP0 = 7;  // 50% duty 
  TCA0.SINGLE.CMP1 = 3;  // 25% duty 
  TCA0.SINGLE.CMP2 = 11;  // 75% duty   
  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm; //enable the timer with no prescaler
  ENABLE_LOGIC;
}

void setup2M() {
  pinMode(PIN_PA3, OUTPUT);           // PIN 7 for clock (WO0)
  TCA0.SINGLE.PER =  7;  // f = 16 / (1 + 7) = 2MHz 
  TCA0.SINGLE.CMP0 = 3;  // 50% duty 
  TCA0.SINGLE.CMP1 = 1;  // 25% duty 
  TCA0.SINGLE.CMP2 = 5;  // 75% duty   
  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm; //enable the timer with no prescaler
  ENABLE_LOGIC;
}

void setup3M() {
  pinMode(PIN_PA3, OUTPUT);           // PIN 7 for clock (WO0)
  TCA0.SINGLE.PER =  4;  // f = 16 / (1 + 4) =~ 3.2MHz 
  TCA0.SINGLE.CMP0 = 2;  // 50% duty 
  TCA0.SINGLE.CMP1 = 1;  // 25% duty 
  TCA0.SINGLE.CMP2 = 3;  // 75% duty   
  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm; //enable the timer with no prescaler
  ENABLE_LOGIC;
}

void setupFloat() {
  DISABLE_LOGIC;  // stop logic
  pinMode(PIN_PA3, INPUT);
  pinMode(PIN_PA6, INPUT);
}

void setup() {

  pinMode(PIN_PA1, INPUT_PULLUP);     // PIN 4 for input
  pinMode(PIN_PA2, INPUT_PULLUP);     // PIN 5 for input
  pinMode(PIN_PA7, OUTPUT);           // PIN 3 for RESET
  digitalWrite(PIN_PA7, LOW);         // /RESET
  PORTMUX.CTRLC = PORTMUX_TCA00_DEFAULT_gc; //turn off PORTMUX, returning WO0 to PA3

  // set single mode
  TCA0.SPLIT.CTRLA = 0; //disable TCA0 and set divider to 1
  TCA0.SPLIT.CTRLESET = TCA_SPLIT_CMD_RESET_gc | 0x03; //set CMD to RESET, and enable on both pins.
  TCA0.SPLIT.CTRLD = 0; //Split mode now off, CMPn = 0, CNT = 0, PER = 255

  // use single slope pwm mode
  TCA0.SINGLE.CTRLB = (TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_CMP1EN_bm |TCA_SINGLE_CMP2EN_bm |TCA_SINGLE_WGMODE_SINGLESLOPE_gc);

  setupLogic();
  setup1M();
  _delay_ms(600);
  digitalWrite(PIN_PA7, HIGH);   // release RESET

}

static uint8_t mode = 3;
void loop() {
  uint8_t x = digitalRead(PIN_PA2) << 1 | digitalRead(PIN_PA1);
  if (x != mode) {
    switch (x) {
      case 0:
        setupFloat();
        break;
      case 1:
        setup3M();
        break;
      case 2:
        setup2M();
        break;
      case 3:
        setup1M();
        break;
    }
    mode = x;
  }
  _delay_ms(10);
}
