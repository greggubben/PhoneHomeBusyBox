#include "HardwareSerial.h"
/*!
 * Phone Home Briefcase Game common definitions
 *
 * Common file for definitions to be used by all modules in the game.
 */

#ifndef PHONEHOME_COMMAND_HANDLING_H
#define PHONEHOME_COMMAND_HANDLING_H

#include <Arduino.h>
#include "PhoneHome_Definitions.h"
#include <PJONSoftwareBitBang.h>



// PJON Bus IDs
#define CONTROL_ID   1
#define FLIPBITS_ID  2
#define SLIDER_ID    3
#define WIRES_ID     4
#define SPINDIGIT_ID 5
#define PHONE_ID     6
#define TEST_ID      10

#define MAX_ID 6
#define ID_ARRAY_LENGTH MAX_ID+1

//const static char* moduleShortNames[] ={"", "Control", "Flip", "Slider", "Wires", "Spin", "Phone"}; 

// Commands
#define COMMAND_WAKE   'W'  // Wake the device to check that it is there
#define COMMAND_ACK    'A'  // Acknowledge it is awake and ready to play
#define COMMAND_START  'S'  // Start playing the puzzle - numbers following are the target digits 
#define COMMAND_INIT   'I'  // Waiting for puzzle to be placed in initial considition - control should display instructions
#define COMMAND_PLAY   'P'  // Playing the game - control should display instructions
#define COMMAND_DONE   'D'  // Game has been solved and puzzle is done
#define COMMAND_LINE   'L'  // Line to display as instructions
#define COMMAND_NEXT   'N'  // Ready for Next Line to display as instructions
#define COMMAND_TUNE   'T'  // Tune settings or Test Values - Remaining commands will be localized

#define DIFFICULTY_EASY   'E'
#define DIFFICULTY_MEDIUM 'M'
#define DIFFICULTY_HARD   'H'
#define MAX_DIFFICULTIES  3

// Default duration for flashing the displays
#define FLASH_DISPLAYS_DURATION 1000

// PJON Communication
PJONSoftwareBitBang *bus;

#define MAX_PJON_COMMAND_LENGTH 25


// Command variables
char command = ' ';           // String of the command
char commandArgument[MAX_PJON_COMMAND_LENGTH] = "";  // String of the command's argument (optional)
bool commandReady = false;    // Indicates the command is ready for processing

// Serial input for debugging interface
char serialInputString[MAX_PJON_COMMAND_LENGTH]  = "";         // a String to hold incoming data from the Serial line
uint8_t serialInputLength = 0;
//bool serialStringComplete = false;      // whether the string is complete
//const int serialInputStringMaxLen = 10; // Maximum length the input string can be


// Receive data via PJON
void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  /* Make use of the payload before sending something, the buffer where payload points to is
     overwritten when a new message is dispatched */

  command = payload[0];
  commandArgument[0] = 0;
  if (length > 1) {
    strncpy(commandArgument, &payload[1], length-1);
    //uint8_t argLen = 0;
    //for (uint8_t c=1; c<length && c<MAX_PJON_COMMAND_LENGTH-1; c++) {
    //  commandArgument[argLen++] = payload[c];
    //}
    commandArgument[length-1] = 0;
  }
  commandReady = true;

  Serial.print(F("PJON Command: "));
  Serial.print(command);
  if (commandArgument[0] != 0) {
    Serial.print(F(" with argument: "));
    Serial.print(commandArgument);
  }
  Serial.println();
};


void setupPJON(uint8_t deviceId) {
  bus = new PJONSoftwareBitBang(deviceId);
  //bus.set_id(deviceId);
  bus->strategy.set_pin(A1);
  bus->set_receiver(receiver_function);
  bus->begin();

}

void loopPJON () {
  bus->receive(1000);
  bus->update();
}


#ifndef PHONEHOME_CONTROLLER

// Print the serial command help
void printHelpInstructions() {
  Serial.println(F("Commands:"));
  Serial.println(F("'W'     - Wake up"));
  Serial.println(F("'SD###' - Start"));
  Serial.println(F("  ^ ^"));
  Serial.println(F("  | +---- - Target Number [digits for puzzle]"));
  Serial.println(F("  +------ - Difficulty [E=Easy; M=Medium; H=Hard]"));
  Serial.println();
  Serial.println(F("Debugging:"));
  Serial.println(F("'P' - Play"));
  Serial.println(F("'D' - Done"));
  Serial.println(F("'T' - Tune/Test"));
}

#endif

// After a command has been processed clear the command so it is not processed again
void clearCommand() {
  commandReady = false;
  command = ' ';
  commandArgument[0] = 0;
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    serialInputString[serialInputLength++] = inChar;
    serialInputString[serialInputLength] = 0;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      serialInputLength--;
      serialInputString[serialInputLength] = 0;

#ifndef PHONEHOME_CONTROLLER

      if (serialInputString[0] == '?') {
        printHelpInstructions();
        serialInputString[0] = 0;
        serialInputLength = 0;
        return;
      }

#endif

      command = serialInputString[0];
      if (serialInputLength > 1) {
        uint8_t ca = 0;
        for (uint8_t c=1; c<serialInputLength && c<MAX_PJON_COMMAND_LENGTH-1; c++) {
          commandArgument[ca++] = serialInputString[c];
        }
        commandArgument[ca] = 0;
      }
      else {
        commandArgument[0] = 0;
      }
      commandReady = true;
      Serial.print(F("Serial Command: "));
      Serial.print(command);
      if (commandArgument[0] != 0) {
        Serial.print(F(" with argument: "));
        Serial.print(commandArgument);
      }
      Serial.println();
      serialInputString[0] = 0;
      serialInputLength = 0;
    }
  }
}

// Send a command
void sendCommand(uint8_t deviceID, char commandByte, char* commandArg, uint8_t argLen) {

  char commandString[MAX_PJON_COMMAND_LENGTH];
  uint8_t commandLength = 0;
  commandString[commandLength++] = commandByte;

  if (argLen > 0) {
    uint8_t argumentLength = argLen;
    if (argumentLength > MAX_PJON_COMMAND_LENGTH - commandLength) {
      argumentLength = MAX_PJON_COMMAND_LENGTH - commandLength;
    }
    for (int c=0; c<argumentLength; c++) {
      commandString[commandLength++] = commandArg[c];
    }
  }
  commandString[commandLength] = 0;
  Serial.print(F("Send '"));
  Serial.print(commandString);
  Serial.print(F("' to "));
  Serial.println(deviceID);
  //Serial.println(commandString);
  bus->send(deviceID,commandString,commandLength);
}


// Send a Wake to Puzzle
void sendWake(int puzzleID) {
  //Serial.print(F("Send Wake to "));
  //Serial.println(puzzleID);
  //char commandString[MAX_PJON_COMMAND_LENGTH];
  //uint8_t commandLength = 0;
  //commandString[commandLength++] = COMMAND_WAKE;
  //commandString[commandLength] = 0;
  //Serial.println(commandString);
  //bus->send(puzzleID,commandString,commandLength);
  sendCommand(puzzleID, COMMAND_WAKE, "", 0);
}

// Send an Acknowledgement to Control
void sendAck(byte numLen, char* shortName) {
  //Serial.println(F("Send ACK to Control"));
  char argument[MAX_PJON_COMMAND_LENGTH];
  argument[0] = (numLen == 'A') ? 'A' : '0' + numLen;
  strcpy(&argument[1], shortName);
  //Serial.println(MAX_PJON_COMMAND_LENGTH);
  //uint8_t commandLength = 0;
  //commandString[commandLength++] = COMMAND_ACK;

  //int argumentLength = strlen(argument);
  //if (argumentLength > MAX_PJON_COMMAND_LENGTH - commandLength) {
  //  argumentLength = MAX_PJON_COMMAND_LENGTH - commandLength;
  //}
  //for (int c=0; c<argumentLength; c++) {
  //  commandString[commandLength++] = argument[c];
  //}
  
  //commandString[commandLength] = 0;
  //Serial.println(commandString);
  //bus->send(CONTROL_ID,commandString,commandLength);
  sendCommand(CONTROL_ID, COMMAND_ACK, argument, strlen(argument));
}

// Send the Start to Puzzle
void sendStart(int puzzleID, char difficulty, char* numChars, int numLength) {
  //Serial.print(F("Send Start to "));
  //Serial.println(puzzleID);
  char argString[MAX_PJON_COMMAND_LENGTH];
  uint8_t argLen = 0;
  //commandString[len++] = COMMAND_START;
  argString[argLen++] = difficulty;
  for (int n=0; n<numLength; n++) {
    argString[argLen++] = numChars[n];
  }
  argString[argLen] = 0;
  //Serial.println(commandString);
  //bus->send(puzzleID,commandString,len);
  sendCommand(puzzleID, COMMAND_START, argString, argLen);
}

// Send Initialize to Control so instructions can be displayed
void sendInitialize(char* argument) {
  //Serial.println(F("Send Initialize to Control"));
  //char commandString[MAX_PJON_COMMAND_LENGTH];
  //uint8_t commandLength = 0;
  //commandString[commandLength++] = COMMAND_INIT;

  //int argumentLength = strlen(argument);
  //if (argumentLength > MAX_PJON_COMMAND_LENGTH - commandLength) {
  //  argumentLength = MAX_PJON_COMMAND_LENGTH - commandLength;
  //}
  //for (int c=0; c<argumentLength; c++) {
  //  commandString[commandLength++] = argument[c];
  //}
  
  //commandString[commandLength] = 0;
  //Serial.println(commandString);
  //bus->send(CONTROL_ID,commandString,commandLength);
  sendCommand(CONTROL_ID, COMMAND_INIT, argument, strlen(argument));
}

// Send Playing to Control so instructions can be displayed
void sendPlay(char* argument) {
  //Serial.println(F("Send Playing to Control"));
  //char commandString[MAX_PJON_COMMAND_LENGTH];
  //uint8_t commandLength = 0;
  //commandString[commandLength++] = COMMAND_PLAY;

  //uint8_t argumentLength = strlen(argument);
  //if (argumentLength > MAX_PJON_COMMAND_LENGTH - commandLength) {
  //  argumentLength = MAX_PJON_COMMAND_LENGTH - commandLength;
  //}
  //for (int c=0; c<argumentLength; c++) {
  //  commandString[commandLength++] = argument[c];
  //}
  
  //commandString[commandLength] = 0;
  //Serial.println(commandString);
  //bus->send(CONTROL_ID,commandString,commandLength);
  sendCommand(CONTROL_ID, COMMAND_PLAY, argument, strlen(argument));
}

// Send a Line to Control
void sendLine(char* lineText) {
  //Serial.println(F("Send Line to Control"));
  //char commandString[MAX_PJON_COMMAND_LENGTH];
  //uint8_t commandLength = 0;
  //commandString[commandLength++] = COMMAND_LINE;
  //uint8_t lineLength = strlen(lineText);
  //if (lineLength > MAX_PJON_COMMAND_LENGTH - commandLength) {
  //  lineLength = MAX_PJON_COMMAND_LENGTH - commandLength;
  //}
  //for (int c=0; c<lineLength; c++) {
  //  commandString[commandLength++] = lineText[c];
  //}
  //commandString[commandLength] = 0;
  //Serial.println(commandString);
  //bus->send(CONTROL_ID,commandString,commandLength);
  sendCommand(CONTROL_ID, COMMAND_LINE, lineText, strlen(lineText));
}

// Send a Next line request to Puzzle
void sendNext(int puzzleID) {
  //Serial.print(F("Send Next to "));
  //Serial.println(puzzleID);
  //char commandString[MAX_PJON_COMMAND_LENGTH];
  //uint8_t commandLength = 0;
  //commandString[commandLength++] = COMMAND_NEXT;
  //commandString[commandLength] = 0;
  //Serial.println(commandString);
  //bus->send(puzzleID,commandString,commandLength);
  sendCommand(puzzleID, COMMAND_NEXT, "", 0);
}

// Send Solved to Control so next puzzle can be played
void sendSolved(bool doneInstructions = false) {
  Serial.println(F("Send Solved to Control"));
  //char commandString[MAX_PJON_COMMAND_LENGTH];
  //uint8_t commandLength = 0;
  //commandString[commandLength++] = COMMAND_DONE;
  uint8_t argLen = 0;
  if (doneInstructions) {
    //commandString[commandLength++] = 'Y';
    argLen = 1;
  }
  //commandString[commandLength] = 0;
  //Serial.println(commandString);
  //bus->send(CONTROL_ID,commandString,commandLength);
  sendCommand(CONTROL_ID, COMMAND_DONE, "Y", argLen);
}



#endif // PHONEHOME_COMMAND_HANDLING_H
