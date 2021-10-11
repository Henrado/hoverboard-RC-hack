// *******************************************************************
//  Arduino Nano 5V example code
//  for   https://github.com/EmanuelFeru/hoverboard-firmware-hack-FOC
//
//  Copyright (C) 2019-2020 Emanuel FERU <aerdronix@gmail.com>
//
// *******************************************************************
// INFO:
// • This sketch uses the the Serial Software interface to communicate and send commands to the hoverboard
// • The built-in (HW) Serial interface is used for debugging and visualization. In case the debugging is not needed,
//   it is recommended to use the built-in Serial interface for full speed perfomace.
// • The data packaging includes a Start Frame, checksum, and re-syncronization capability for reliable communication
// 
// CONFIGURATION on the hoverboard side in config.h:
// • Option 1: Serial on Right Sensor cable (short wired cable) - recommended, since the USART3 pins are 5V tolerant.
   #define CONTROL_SERIAL_USART3
   #define FEEDBACK_SERIAL_USART3
//   // #define DEBUG_SERIAL_USART3
// • Option 2: Serial on Left Sensor cable (long wired cable) - use only with 3.3V devices! The USART2 pins are not 5V tolerant!
//   #define CONTROL_SERIAL_USART2
//   #define FEEDBACK_SERIAL_USART2
//   // #define DEBUG_SERIAL_USART2
// *******************************************************************

//#include <SoftwareSerial.h>
#include <IBusBM.h>

// ########################## DEFINES ##########################
#define HOVER_SERIAL_BAUD   115200      // [-] Baud rate for HoverSerial (used to communicate with the hoverboard)
#define SERIAL_BAUD         115200      // [-] Baud rate for built-in Serial (used for the Serial Monitor)
#define START_FRAME         0xABCD     	// [-] Start frme definition for reliable serial communication
#define TIME_SEND           50         // [ms] Sending time interval
#define SPEED_MAX_TEST      200         // [-] Maximum speed for testing
// #define DEBUG_RX                        // [-] Debug received data. Prints all bytes to serial (comment-out to disable)

HardwareSerial& HoverSerial = Serial2;        // RX, TX
IBusBM ibusRc;
HardwareSerial& ibusRcSerial = Serial1;

// Global variables
uint8_t idx = 0;                        // Index for new data pointer
uint16_t bufStartFrame;                 // Buffer Start Frame
byte *p;                                // Pointer declaration for the new received data
byte incomingByte;
byte incomingBytePrev;

int toleranse = 5;

int trimSpeed = 20;
int trimStear = 20;

struct dataSpeedStear{
  float speed;
  float stear;
};

// ########################## SETUP ##########################
void setup() 
{
  //Serial.begin(SERIAL_BAUD);
  //Serial.println("Hoverboard Serial v1.0");
  HoverSerial.begin(HOVER_SERIAL_BAUD);
  ibusRc.begin(ibusRcSerial);
  Send(0, 0);
}


// ########################## LOOP ##########################
unsigned long iTimeSend = 0;
int iTestMax = SPEED_MAX_TEST;
int iTest = 0;

void loop(void){
  int speed = readChannel(1, -300, 300, 0);
  int stear = readChannel(0, -300, 300, 0);
  unsigned long timeNow = millis();
  //Receive();
  if (iTimeSend > timeNow) return;
  iTimeSend = timeNow + TIME_SEND;
  dataSpeedStear regning = calcSpeedStear(speed, stear, toleranse);
  Send(regning.stear, regning.speed);
  //Serial.println(regning.speed);
}
// ########################## END ##########################
