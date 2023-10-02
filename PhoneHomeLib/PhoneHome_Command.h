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

// Commands
const char COMMAND_WAKE   = 'W';  // Wake the device to check that it is there
const char COMMAND_ACK    = 'A';  // Acknowledge it is awake and ready to play
const char COMMAND_START  = 'S';  // Start playing the puzzle - numbers following are the target digits 
const char COMMAND_INIT   = 'I';  // Waiting for puzzle to be placed in initial considition - control should display instructions
const char COMMAND_PLAY   = 'P';  // Playing the game - control should display instructions
const char COMMAND_DONE   = 'D';  // Game has been solved and puzzle is done
const char COMMAND_TUNE   = 'T';  // Tune settings or Test Values - Remaining commands will be localized

const char DIFFICULTY_EASY   = 'E';
const char DIFFICULTY_MEDIUM = 'M';
const char DIFFICULTY_HARD   = 'H';
#define MAX_DIFFICULTIES  3

// Default duration for flashing the displays
#define FLASH_DISPLAYS_DURATION 1000

// PJON Communication
PJONSoftwareBitBang *bus;

#define MAX_PJON_COMMAND_LENGTH 11


// Command variables
char command = " ";          // String of the command
String commandArgument = "";  // String of the command's argument (optional)
bool commandReady = false;    // Indicates the command is ready for processing

// Serial input for debugging interface
String serialInputString  = "";         // a String to hold incoming data from the Serial line
//bool serialStringComplete = false;      // whether the string is complete
//const int serialInputStringMaxLen = 10; // Maximum length the input string can be


// Receive data via PJON
void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  /* Make use of the payload before sending something, the buffer where payload points to is
     overwritten when a new message is dispatched */

  Serial.print("PJON Length = ");
  Serial.print(length);
  Serial.print(" '");
  for (int c=0; c<length; c++) {
    Serial.print((char)payload[c]);
  }
  Serial.println("'");

  command = payload[0];
  commandArgument = "";
  if (length > 1) {
    for (int c=1; c<length; c++) {
      commandArgument += (char)payload[c];
    }
  }
  commandReady = true;

  Serial.print(F("PJON Command: "));
  Serial.print(command);
  if (commandArgument.length() > 0) {
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



// Print the serial command help
void printHelpInstructions() {
  Serial.println(F("Commands for simulating Command:"));
  Serial.println(F("'W'     - Wake up"));
  Serial.println(F("'SD###' - Start"));
  Serial.println(F("  ^ ^"));
  Serial.println(F("  | +---- - Target Number [1-3 digits for puzzle; 7 digits for phone dialer]"));
  Serial.println(F("  +------ - Difficulty [E=Easy; M=Medium; H=Hard]"));
  Serial.println();
  Serial.println(F("Commands for debugging:"));
  Serial.println(F("'P' - Play"));
  Serial.println(F("'D' - Done"));
}

// After a command has been processed clear the command so it is not processed again
void clearCommand() {
  commandReady = false;
  command = ' ';
  commandArgument = "";
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
    serialInputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      serialInputString.trim();
      if (serialInputString[0] == '?') {
        printHelpInstructions();
        serialInputString = "";
        return;
      }
      command = serialInputString[0];
      if (serialInputString.length() > 1) {
        commandArgument = serialInputString.substring(1);
      }
      else {
        commandArgument = "";
      }
      commandReady = true;
      Serial.print(F("Serial Command: "));
      Serial.print(command);
      if (commandArgument.length() > 0) {
        Serial.print(F(" with argument: "));
        Serial.print(commandArgument);
      }
      Serial.println();
      serialInputString = "";
    }
  }
}


// Send a Wake to Puzzle
void sendWake(int puzzleID) {
  Serial.print(F("Sending Wake to "));
  Serial.println(puzzleID);
  char commandString[MAX_PJON_COMMAND_LENGTH];
  int commandLength = 0;
  commandString[commandLength++] = COMMAND_WAKE;
  commandString[commandLength] = 0;
  Serial.println(commandString);
  bus->send(puzzleID,commandString,commandLength);
}

// Send an Acknowledgement to Control
void sendAck(char* deviceName) {
  Serial.println(F("Sending ACK to Control"));
  char commandString[MAX_PJON_COMMAND_LENGTH];
  int commandLength = 0;
  commandString[commandLength++] = COMMAND_ACK;
  commandString[commandLength] = 0;
  Serial.println(commandString);
  bus->send(CONTROL_ID,commandString,commandLength);
}

// Send the Start to Puzzle
void sendStart(int puzzleID, char difficulty, char* numChars, int numLength) {
  Serial.print(F("Sending Start to "));
  Serial.println(puzzleID);
  char commandString[MAX_PJON_COMMAND_LENGTH];
  int len = 0;
  commandString[len++] = COMMAND_START;
  commandString[len++] = difficulty;
  for (int n=0; n<numLength; n++) {
    commandString[len++] = numChars[n];
  }
  commandString[len] = 0;
  Serial.println(commandString);
  bus->send(puzzleID,commandString,len);
}

// Send Initialize to Control so instructions can be displayed
void sendInitialize() {
  Serial.println(F("Sending Initialize to Control"));
  char commandString[MAX_PJON_COMMAND_LENGTH];
  int commandLength = 0;
  commandString[commandLength++] = COMMAND_INIT;
  commandString[commandLength] = 0;
  Serial.println(commandString);
  bus->send(CONTROL_ID,commandString,commandLength);
}

// Send Playing to Control so instructions can be displayed
void sendPlay() {
  Serial.println(F("Sending Playing to Control"));
  char commandString[MAX_PJON_COMMAND_LENGTH];
  int commandLength = 0;
  commandString[commandLength++] = COMMAND_PLAY;
  commandString[commandLength] = 0;
  Serial.println(commandString);
  bus->send(CONTROL_ID,commandString,commandLength);
}

// Send Solved to Control so next puzzle can be played
void sendSolved() {
  Serial.println(F("Sending Solved to Control"));
  char commandString[MAX_PJON_COMMAND_LENGTH];
  int commandLength = 0;
  commandString[commandLength++] = COMMAND_DONE;
  commandString[commandLength] = 0;
  Serial.println(commandString);
  bus->send(CONTROL_ID,commandString,commandLength);
}


#endif // PHONEHOME_COMMAND_HANDLING_H
