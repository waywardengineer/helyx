#include "Wire.h"
#include <Adafruit_PWMServoDriver.h>
#include <ctype.h>
#include <EEPROM.h>



// !bbc.llrrggbb.l2rrggbb!b2c.llrrggbb
#define BOARD_ADDR_BITS 5
#define HIGHESTADDRESS 12
#define CMDBUFFERLEN 120
const uint8_t iAddrPins[BOARD_ADDR_BITS] = {3, 4, 5, 6, 7};
const uint8_t oKillPowerOut = 9;
const uint8_t oLed = 10;
int i;
int j;
uint8_t boardAddress;
uint8_t cmdBuffer[ CMDBUFFERLEN ];  // array that will hold the serial input string
uint8_t asciiMode = true;
int cmdLength;
uint8_t gettingCommand = false; // true if we are in the process of recieving a command for us
int cmdIndex = 0;
unsigned long eventTimer = 0;
uint8_t bufferIndex = 0; // true if we are in the process of recieving a command for us
uint8_t buffer[3];
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  pwm.begin();
  pwm.setPWMFreq(1200); 

  for (i=0; i < BOARD_ADDR_BITS; i++){
    pinMode(iAddrPins[i], INPUT);
    digitalWrite(iAddrPins[i], HIGH);
  }
  boardAddress = 1;//getMyAddress();
    
}
void loop() {
  cmdLength = checkSerial();
  if (cmdLength > 0){
    if (cmdBuffer[0] == 'V'){
       Serial.println("PWM set");
       Serial.println(cmdBuffer[1], HEX);
       Serial.println(cmdBuffer[2], HEX);
       pwm.setPWM(cmdBuffer[1], 0, (cmdBuffer[2]*16) );
    }
    if (cmdBuffer[0] == 'b'){
      changeBaudRate();
    }
    

  }

}
  
void changeBaudRate(){
  char baudRateStr[cmdLength - 1];
  i = 0;
  while (i < cmdLength){
    baudRateStr[i] = cmdBuffer[++i];
    Serial.println(cmdBuffer[i]);
  }
  long baudRate = atol(baudRateStr);
  Serial.println("changeBaudRate");
  Serial.println(baudRateStr);
  Serial.println(baudRate);
  
  Serial.end();
  delay(50);
  Serial.begin(baudRate);
}





int checkSerial()
{
  if(!Serial.available()) {
    return 0;
  }
  delay(10);  // wait a little for serial data
  if (!gettingCommand){
    memset( cmdBuffer, 0, sizeof(cmdBuffer) ); // set it all to zero
    cmdIndex = 0;
    bufferIndex = 0;
  }
  uint8_t commandBrdAddress;
  uint8_t boardAddrStartByte = asciiMode? '!':200;
  uint8_t boardAddrBytes = asciiMode? 2:1;
  
  while(Serial.available() && !gettingCommand) {//process through the serial buffer and see if there's anything for us
    buffer[bufferIndex] = Serial.read();
    if (bufferIndex) {
      bufferIndex ++;
    }
    else if (buffer[bufferIndex] == boardAddrStartByte) { //start of new command;
      bufferIndex = 1;
    }
    if (bufferIndex > boardAddrBytes){
      commandBrdAddress = asciiMode?toHex(buffer[1], buffer[2]):buffer[1];
      if (commandBrdAddress == 0 || commandBrdAddress == boardAddress){ // it's for us, start saving the data
        gettingCommand = true;
      }
      bufferIndex = 0;
    }
  }
  
  while(Serial.available() && gettingCommand){// we've found something for us, decode if ascii and save to cmdbuffer
    buffer[bufferIndex] = Serial.read();
    if (buffer[bufferIndex] == boardAddrStartByte && cmdIndex > 0){
      gettingCommand = false;
    }
    
    if (gettingCommand && asciiMode && cmdIndex > 0 ){//cmdBuffer[0] is the one byte ascii letter command
      if (buffer[bufferIndex] == '.'){
        cmdBuffer[cmdIndex ++] = 210;        //start bit for led addres
      }
      else { // everything besides a board or led address start bit or commnd bit will be 2 digit hex
        if (bufferIndex == 0) {
          bufferIndex ++;
        }
        else {
          cmdBuffer[cmdIndex ++] = toHex(buffer[0], buffer[1]);
          bufferIndex = 0;          
        }
      }
    }
    else {
      cmdBuffer[cmdIndex ++] = buffer[bufferIndex];
      Serial.println(cmdBuffer[cmdIndex]);
      
    }
  }
  return gettingCommand ? 0 : cmdIndex;  // return number of chars read of command that applies to us
}




uint8_t getMyAddress(){
  uint8_t myAddress = 0;
  uint8_t thisBit = 0;
  for (i=0; i < BOARD_ADDR_BITS; i++){
    thisBit = digitalRead(iAddrPins[i]) ? 1 : 0;
    myAddress = myAddress || (thisBit << i);
  }
  return myAddress;
}


uint8_t toHex(char hi, char lo)
{
  uint8_t b;
  hi = toupper(hi);
  if( isxdigit(hi) ) {
    if( hi > '9' ) hi -= 7;      // software offset for A-F
    hi -= 0x30;                  // subtract ASCII offset
    b = hi<<4;
    lo = toupper(lo);
    if( isxdigit(lo) ) {
      if( lo > '9' ) lo -= 7;  // software offset for A-F
      lo -= 0x30;              // subtract ASCII offset
      b = b + lo;
      return b;
    } // else error
  }  // else error
  return 0;
}

 
