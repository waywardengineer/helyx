#include "Wire.h"
#include "BlinkM_funcs.h"
#include <ctype.h>
#include <EEPROM.h>

extern "C" { 
#include "utility/twi.h"  // from Wire library, so we can do bus scanning
}


// !bbc.llrrggbb.l2rrggbb!b2c.llrrggbb
#define BOARD_ADDR_BITS 5
#define HIGHESTADDRESS 12
#define CMDBUFFERLEN 120
const uint8_t iAddrPins[BOARD_ADDR_BITS] = {3, 4, 5, 6, 7};
const uint8_t oKillPowerOut = 9;
const uint8_t oLed = 10;
uint8_t ledAddresses[HIGHESTADDRESS];
uint8_t ledColorIndexes[HIGHESTADDRESS];
int ledAddressCount = 0;
int i;
int j;

uint8_t boardAddress;
uint8_t cmdBuffer[ CMDBUFFERLEN ];  // array that will hold the serial input string
uint8_t asciiMode = true;
int cmdLength;
uint8_t gettingCommand = false; // true if we are in the process of recieving a command for us
int cmdIndex = 0;
uint8_t c1;
uint8_t c2;
uint8_t c3;
uint8_t mode = 1;
unsigned long eventTimer = 0;
uint8_t defaultColors[3][3];
uint8_t rnd;
uint8_t bufferIndex = 0; // true if we are in the process of recieving a command for us
uint8_t buffer[3];




void setup()
{
  Wire.begin();
  delay(100); // wait a bit for things to stabilize
    BlinkM_off(0);  // turn everyone off

  Serial.begin(9600);
  for (i=0; i < BOARD_ADDR_BITS; i++){
    pinMode(iAddrPins[i], INPUT);
    digitalWrite(iAddrPins[i], HIGH);
  }
  boardAddress = 1;//getMyAddress();
  populateLedAddresses();
  loadDefaultColors();
  for (i=0; i < ledAddressCount; i++){
    ledColorIndexes[i] = random(3);
  }
    
}
void loop() {
  cmdLength = checkSerial();
  if (cmdLength > 0){
    if (cmdBuffer[0] == 'c' || cmdBuffer[0] == 'h' || cmdBuffer[0] == 'C' || cmdBuffer[0] == 'H'){
     mode = 0;
      Serial.println("LEDcommand");
      sendLedColorCmd(cmdLength);
    }
    if (cmdBuffer[0] == 'b'){
      changeBaudRate();
    }
    if (cmdBuffer[0] == 'Q'){
      asciiMode = true;
      Serial.println("Ascii mode");
    }
    if (cmdBuffer[0] == 'q'){
      asciiMode = false;
      Serial.println("Binary mode");
    }
    if (cmdBuffer[0] == 'T'){
      saveDefaultColors();
    }
    if (cmdBuffer[0] == 't'){
      uint8_t index = 1;
      for (i=0; i < 3; i++){
        for (j=0; j < 3; j++){
          defaultColors[i][j] = cmdBuffer[index++];

        }
      }
    }
    
  }
  switch(mode){  
    case 1:
      if (eventTimer < millis()){
        for (i = 0; i < ledAddressCount; i++){
          if (random(100) < 6){
            ledColorIndexes[i] = random(3);
          }
          c1 = randomize(defaultColors[ledColorIndexes[i]][0], 7);
          c2 = randomize(defaultColors[ledColorIndexes[i]][1], 60);
          c3 = randomize(defaultColors[ledColorIndexes[i]][2], 60);
          BlinkM_fadeToHSB(ledAddresses[i], c1, c2, c3);
        }
        eventTimer = millis() + 300;
     }
      break;
    }

}
uint8_t randomize (uint8_t num, uint8_t range){
  uint8_t change = random(range);
  if (change % 2){
    num = (num  >  change)?(num - change) : 0;
  }
  else{
    num = ((255 - num)  >  change)?(num + change) : 255;
  }


  return num;
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



void sendLedColorCmd(int cmdLength){
  uint8_t subCmdIndex = 0;
  int subCmdLength = 5;
  int startIndex = 1;
  int nextStartIndex = 0;
  while (startIndex < cmdLength){
    nextStartIndex = startIndex + subCmdLength;
    Serial.println(cmdBuffer[startIndex]);
    if (cmdBuffer[startIndex] == 210){
        Serial.println("beginning");
      if (nextStartIndex >= cmdLength  || cmdBuffer[nextStartIndex] == 200){// subcommand is right length;
        Serial.println("sendingCmd");
        Wire.beginTransmission(cmdBuffer[++startIndex]);
        Wire.write(cmdBuffer[0]);
        Wire.write(cmdBuffer[++startIndex]);
        Wire.write(cmdBuffer[++startIndex]);
        Wire.write(cmdBuffer[++startIndex]);
        Wire.endTransmission();
      }
      else {
        startIndex ++;
      }
    }
    else {
      startIndex ++;
    }
  }
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

void loadDefaultColors(){
  uint8_t addr = 0;
  for (i=0; i<3; i++){
    for (uint8_t j = 0; j < 3; j++){
      defaultColors [i][j] = EEPROM.read (addr++);
    }
  }
}
void saveDefaultColors(){
  uint8_t addr = 0;
  for (i=0; i<3; i++){
    for (uint8_t j = 0; j < 3; j++){
      EEPROM.write (addr++, defaultColors [i][j]);
    }
  }
}
 
void populateLedAddresses() {
  byte rc;
  byte data = 0; // not used, just an address to feed to twi_writeTo()
  for( byte addr=1; addr < HIGHESTADDRESS; addr++ ) {
    rc = twi_writeTo(addr, &data, 0, 1, 0);
    if( rc == 0 ) {
      ledAddresses[ledAddressCount++] = addr;
    }
  }
}
