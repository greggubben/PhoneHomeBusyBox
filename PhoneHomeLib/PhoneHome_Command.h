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


// PJON Bus IDs
#define COMMAND_ID  1
#define FLIPBITS_ID 2
#define SLIDER_ID   3
#define WIRES_ID    4
#define DIALNUM_ID  5
#define PHONE_ID    6
#define TEST_ID     10

// Commands
const char COMMAND_WAKE   = 'W';  // Wake the device to check that it is there
const char COMMAND_ACK    = 'A';  // Acknowledge it is awake and ready to play
const char COMMAND_START  = 'S';  // Start playing the puzzle - numbers following are the target digits 
const char COMMAND_INIT   = 'I';  // Waiting for puzzle to be placed in initial considition - control should display instructions
const char COMMAND_PLAY   = 'P';  // Playing the game - control should display instructions
const char COMMAND_DONE   = 'D';  // Game has been solved and puzzle is done

const char DIFFICULTY_EASY   = 'E';
const char DIFFICULTY_MEDIUM = 'M';
const char DIFFICULTY_HARD   = 'H';

// Command variables
char command = " ";          // String of the command
String commandArgument = "";  // String of the command's argument (optional)
bool commandReady = false;    // Indicates the command is ready for processing

// Serial input for debugging interface
String serialInputString  = "";         // a String to hold incoming data from the Serial line
//bool serialStringComplete = false;      // whether the string is complete
//const int serialInputStringMaxLen = 10; // Maximum length the input string can be


// Send an Acknowledgement to Command
void sendAck(char* deviceName) {
  Serial.println(F("Sending ACK to Command"));
}

// Send Initialize to Command so instructions can be displayed
void sendInitialize() {
  Serial.println(F("Sending Initialize to Command"));

}

// Send Playing to Command so instructions can be displayed
void sendPlay() {
  Serial.println(F("Sending Playing to Command"));

}

// Send Solved to Command so next puzzle can be played
void sendSolved() {
  Serial.println(F("Sending Solved to Command"));

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


#endif // PHONEHOME_COMMAND_HANDLING_H
