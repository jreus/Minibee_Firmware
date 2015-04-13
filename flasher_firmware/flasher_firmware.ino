/**
 * 2015 Jonathan Reus
 * original code Copyright (c) Marije Baalman. All rights reserved
 *
 *  Custom minibee firmware for general-purpose switching applications such as flashing a LED and variable frequency PWM. 
 *
 *
 */
 
 /*
 Needs to be programmed quickly, before it goes out of bootloader mode.
 
 */

/*
Pseudo code. C - coordinator, M - minibee.

Setup:
M set timer interval to something very fast as a maximum quantization of pulse widths.
Say.. 10Khz ... or every 100us.



Operation:
C send(flashing_rate, burst_duration, curve) // burst_duration=0 for continuous flashing
M receive data
M set new flashing rate, start duration timer
M on timer clock tick
- alternate switch on/off
- count time elapsed
- if time elapsed > burst_duration stop duration timer

M send duration elapsed to coordinator



*/

/// in the header file of the MiniBee you can disable some options to save
/// space on the MiniBee. If you don't the board may not work as it runs
/// out of RAM.

/// Wire needs to be included if TWI is enaleds[2]

#include <Wire.h>

#include <LIS302DL.h>
#include <ADXL345.h>
#include <TMP102.h>
#include <BMP085.h>
#include <HMC5843.h>

// Use MiniBee_APIn.h ---- the other libraries (MiniBee.h and MiniBeev2.h 
// are older versions that are no longer maintained
#include <XBee.h>
#include <MiniBee_APIn.h>

#include <TimerOne.h>

#define TIMER_PERIOD 100 // timer period in us - interrupt every 100us
#define LED_PIN 3

MiniBee_API Bee = MiniBee_API();

volatile boolean hi = false;

struct ledstruct {
  boolean on;
  uint16_t duration; // how many timer periods before switching to alternate state
  uint16_t time; // counter for specific led
  int8_t pin;
} leds[5];


void callback()
{
  clock_led(0);
  clock_led(1);
  clock_led(2);
  clock_led(3);
  clock_led(4);
}

void clock_led(int lednum) {
  leds[lednum].time++;
  if(leds[lednum].time > leds[lednum].duration) {
    digitalWrite(leds[lednum].pin, (leds[lednum].on) ? HIGH : LOW);
    leds[lednum].on = !leds[lednum].on;
    leds[lednum].time=0;
  }  
}

/// this will be our parser for the custom messages we will send:
/// msg[0] and msg[1] will be msg type ('E') and message ID
/// the remainder are the actual contents of the message
/// set the leds duration via a 16 bit integer separated into 2 bytes
/// msg[2]=byte0, msg[3]=byte1
void customMsgParser( uint8_t * msg, uint8_t size, uint16_t source ){
    leds[0].duration = (msg[3] << 8) + msg[2];
    leds[1].duration = (msg[5] << 8) + msg[4];
    leds[2].duration = (msg[7] << 8) + msg[6];
    leds[3].duration = (msg[9] << 8) + msg[8];
    leds[4].duration = (msg[11] << 8) + msg[10];
}


void setup() {  
  Bee.setup(57600, 'D' ); // arguments are the baudrate, and the board revision  

  // Set defaults for led rates
  leds[0].pin = 3;
  leds[0].on=false;
  leds[0].duration=73; // 100us x 3000 = change state every 0.3 seconds
  leds[0].time=0;

  leds[1].pin = 5;
  leds[1].on=false;
  leds[1].duration=100;
  leds[1].time=0;
  
  leds[2].pin = 6;
  leds[2].on=false;
  leds[2].duration=2000; 
  leds[2].time=0;
  
  leds[3].pin = 7;
  leds[3].on=false;
  leds[3].duration=70; 
  leds[3].time=0;
  
  leds[4].pin = 8;
  leds[4].on=false;
  leds[4].duration=220;
  leds[4].time=0;
  
  pinMode(leds[0].pin, OUTPUT); 
  pinMode(leds[1].pin, OUTPUT); 
  pinMode(leds[2].pin, OUTPUT); 
  pinMode(leds[3].pin, OUTPUT); 
  pinMode(leds[4].pin, OUTPUT);
  
  Bee.setCustomPin(leds[0].pin, 0);
  Bee.setCustomPin(leds[1].pin, 0);
  Bee.setCustomPin(leds[2].pin, 0);
  Bee.setCustomPin(leds[3].pin, 0);
  Bee.setCustomPin(leds[4].pin, 0);
  Bee.setCustomCall( &customMsgParser );
  
  noInterrupts();
   Timer1.initialize(TIMER_PERIOD);   // initialize timer1
   Timer1.attachInterrupt(callback);  // attaches callback() as a timer overflow interrupt
   interrupts(); 
}

void loop() { 
  Bee.loopStep();
}
