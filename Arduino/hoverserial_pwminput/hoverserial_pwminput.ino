//  Arduino Nano 5V example code
//  for   https://github.com/EmanuelFeru/hoverboard-firmware-hack-FOC
//  Copyright (C) 2019-2020 Emanuel FERU <aerdronix@gmail.com>
// INFO:
// • The built-in (HW) Serial interface is used for debugging and visualization. In case the debugging is not needed,
//   it is recommended to use the built-in Serial interface for full speed perfomace.
// • The data packaging includes a Start Frame, checksum, and re-syncronization capability for reliable communication
// 
// CONFIGURATION on the hoverboard side in config.h:
   #define CONTROL_SERIAL_USART3
   #define FEEDBACK_SERIAL_USART3
//   // #define DEBUG_SERIAL_USART3
// *******************************************************************


//#include <SoftwareSerial.h>


// ########################## DEFINES til hoverbord ##########################
#define HOVER_SERIAL_BAUD   115200      // [-] Baud rate for HoverSerial (used to communicate with the hoverboard)
#define SERIAL_BAUD         115200      // [-] Baud rate for built-in Serial (used for the Serial Monitor)
#define START_FRAME         0xABCD     	// [-] Start frme definition for reliable serial communication
#define TIME_SEND           50         // [ms] Sending time interval
#define SPEED_MAX_TEST      200         // [-] Maximum speed for testing
// #define DEBUG_RX                        // [-] Debug received data. Prints all bytes to serial (comment-out to disable)
uint8_t idx = 0;                        // Index for new data pointer
uint16_t bufStartFrame;                 // Buffer Start Frame
byte *p;                                // Pointer declaration for the new received data
byte incomingByte;
byte incomingBytePrev;

#define CH1 3
#define CH2 5


HardwareSerial& HoverSerial = Serial2;        // RX, TX

int toleranse = 5;
int maxInt = 100;

struct dataSpeedStear{
  float speed;
  float stear;
};

struct dataLeftRight{
  float left;
  float right;
};



// ########################## SETUP ##########################
void setup() 
{
  Serial.begin(SERIAL_BAUD);
  Serial.println("Hoverboard Serial v1.0");
  HoverSerial.begin(HOVER_SERIAL_BAUD);
  pinMode(CH1, INPUT);
  pinMode(CH2, INPUT);
  Send(0, 0);
}


// ########################## LOOP ##########################
unsigned long iTimeSend = 0;
int iTestMax = SPEED_MAX_TEST;
int iTest = 0;

void loop(void){
  // Henter joystick posisjoner 
  int y = readChannel(1, -maxInt, maxInt, 0);
  int x = readChannel(0, -maxInt, maxInt, 0);

  // For å regne ut når man skal sende kommandoene til hoverbordet 
  unsigned long timeNow = millis();
  //Receive();
  if (iTimeSend > timeNow) return;
  else iTimeSend = timeNow + TIME_SEND;
 
  // Regner ut left og right styrke til hjulene 
  dataLeftRight regning = calcLeftRight(x, y, maxInt, 3, 3);
  Serial.print(x);
  Serial.print(" ");
  Serial.print(y);
  Serial.print(" ");
  Serial.print(regning.left);
  Serial.print(" ");
  Serial.println(regning.right);
  Send(regning.left, regning.right);
}
// ########################## END ##########################
