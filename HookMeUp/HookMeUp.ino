// Connect Wires
// Receive Pins (top row of pins) and bit lights should always match
// The driver pins will be randomized

#include <Arduino.h>
#include <PhoneHome_Definitions.h>
#include <PhoneHome_Command.h>
#include <PhoneHome_PuzzleState.h>


// Puzzle definitions
const int   PuzzleId  = WIRES_ID;
const char* PuzzleShortName = "Wires";
const char* PuzzleLongName = "Hook Me Up";
bool puzzleInitialized = false;
char puzzleDifficulty = " ";
#define MAX_NUMBER_LENGTH 1


// Pin definitions
int driverPins[]  = {9,8,7,6};
int receivePins[] = {5,4,3,2};
int bitLightPins[]  = {A2, A3, A4, A5};
int puzzleDriverPins[4];
int puzzleReceivePins[4];

byte targetNumber = 9;

// Instuctions
int instructionLine = 0;

//
// Instrutions for initializing the puzzle
//
// Limit to 20 chars  "                    "
const char Init_1[] = "Disconnect All Wires";

const char *const Init[] =
{   
  Init_1
};

const int initLines = 1;

//
// Instructions for playing puzzle
//
// Limit to 20 chars  "                    "
const char Play_1[] = "Connect Wires across";
const char Play_2[] = "Top & Bottom rows";
const char Play_3[] = "Turn On Lights Off";
const char Play_4[] = "Turn Off Lights On";

const char *const Play[] =
{   
  Play_1,
  Play_2,
  Play_3,
  Play_4
};

const int playLines = 4;

//
// Instructions for determining number
//
// Limit to 20 chars  "                    "
const char Done_1[] = "Use Hexadaisy to";
const char Done_2[] = "convert lights in";
const char Done_3[] = "Binary to Decimal.";

const char *const Done[] =
{   
  Done_1,
  Done_2,
  Done_3
};

const int doneLines = 3;


// Turn on all visible lights and indicators for a little while as a test
void flashDisplays () {
  Serial.println(F("Flashing Displays"));
  // Turn on Lights and other indicators
  for (int p = 0; p < 4; p++) {
    digitalWrite(bitLightPins[p], HIGH);
  }
  delay(FLASH_DISPLAYS_DURATION);
  // Turn off Lights and other indicators
  for (int p = 0; p < 4; p++) {
    digitalWrite(bitLightPins[p], LOW);
  }
}


// Check if the puzzle is in a ready state
bool puzzleReady() {
  bool puzzReady = true;
  // Check for the puzzle to be placed in ready state
  // Make sure all 
  for (int drivePos=0; drivePos<4 && puzzReady; drivePos++) {
    digitalWrite(driverPins[drivePos], LOW);
    for (int receivePos=0; receivePos<4 && puzzReady; receivePos++) {
      if (!digitalRead(receivePins[receivePos])) {
        puzzReady = false;
      }
    }
    digitalWrite(driverPins[drivePos], HIGH);
  }
  // returning false for testing
  return puzzReady;
}


// Perform actions when a Wake command is received
void performWake() {
  flashDisplays();
  sendAck(MAX_NUMBER_LENGTH, PuzzleShortName);
  setPuzzleState(PuzzleStates::Ready);
  clearCommand();

}

// Perform actions when a Start command is received
void performStart(String commandArgument) {
  puzzleInitialized = false;
  setPuzzleState(PuzzleStates::Intialize);
  clearCommand();

  puzzleDifficulty = (commandArgument.length() > 0) ? commandArgument[0] : DIFFICULTY_EASY;
  targetNumber = (commandArgument.length() > 1) ? commandArgument.substring(1,2).toInt() : targetNumber;
  targetNumber = constrain(targetNumber, 0, 15);
  Serial.print(F("Difficulty = "));
  Serial.println(puzzleDifficulty);
  Serial.print(F("Target Num = "));
  Serial.println(targetNumber);
}

// Perform the Initialization steps including any randomizations
void performInitialize() {

  // Perform first time tasks such as randomization of drivers
  if (!puzzleInitialized) {
    puzzleInitialized = true;

    int remainingPins[4];

    // Start with no randomizing
    for (int p=0; p<4; p++) {
      puzzleDriverPins[p] = driverPins[p];
      puzzleReceivePins[p] = receivePins[p];
    }
    switch (puzzleDifficulty) {

      case DIFFICULTY_EASY:
      default:
        // No randomizing
        break;
      case DIFFICULTY_HARD:
        // Randomize the receivePins
        for (int p=0; p<4; p++) {
          remainingPins[p] = receivePins[p];
        }
        for (int p=0; p<4; p++) {
          int leftPins = 4-p;
          int rndPos = random(leftPins);
          puzzleReceivePins[p] = remainingPins[rndPos];
          remainingPins[rndPos] = remainingPins[leftPins-1];
        }

        // Conitnue on to randomize the driverPins like Medium difficulty
      case DIFFICULTY_MEDIUM:
        // Randomize the driverPins
        for (int p=0; p<4; p++) {
          remainingPins[p] = driverPins[p];
        }
        for (int p=0; p<4; p++) {
          int leftPins = 4-p;
          int rndPos = random(leftPins);
          puzzleDriverPins[p] = remainingPins[rndPos];
          remainingPins[rndPos] = remainingPins[leftPins-1];
        }
        break;
    }
    for (int p=0; p<4; p++) {
      Serial.print(F("Driver: "));
      Serial.print(driverPins[p]);
      Serial.print(F(" --> "));
      Serial.print(puzzleDriverPins[p]);
      Serial.print(F(" to Receiver: "));
      Serial.print(receivePins[p]);
      Serial.print(F(" --> "));
      Serial.print(puzzleReceivePins[p]);
      Serial.print(F(" for Light: "));
      Serial.print(bitLightPins[p]);
      Serial.println();
    }

    if (!puzzleReady()) {
      // Need to let Command know we will be in this state for a while
      sendInitialize(PuzzleLongName);
      instructionLine = 0;
    }
  }

  // Wait until the puzzle has been reset by player
  if (puzzleReady()) {
    // Good to go - Let's play
    // Inform Command we are playing
    sendPlay(PuzzleLongName);
    instructionLine = 0;
    setPuzzleState(PuzzleStates::Playing);
  }
}

// Handle playing the puzzle
void performPlaying() {
  bool puzzleSolved = false;

  byte targetComplement = 0xFF ^ targetNumber;
  byte puzzleNumber = 0;
  byte displayNumber = 0;
  uint8_t wiresConnected = 0;

  // Check wire connections and update lights
  for (int p=0; p<4; p++) {
    byte bitMask = 1<<p;
    byte targetBit = targetComplement & bitMask;

    // Read the wires
    digitalWrite(puzzleDriverPins[p], LOW);
    byte r = digitalRead(puzzleReceivePins[p]);
    digitalWrite(puzzleDriverPins[p], HIGH);
    if (r == 0) {
      wiresConnected++;
    }
    r ^= 0x01;  // Flip the bit because it is inverted
    r = r << p; // Shift into position

    if ((targetBit ^ r) == 0) {
      digitalWrite(bitLightPins[p], LOW);
    }
    else {
      displayNumber |= bitMask;
      digitalWrite(bitLightPins[p], HIGH);
    }

    // Check if puzzle has been solved
    //if (wiresConnected == 4 && puzzleNumber == targetNumber) {
    if (wiresConnected == 4 && displayNumber == targetNumber) {
      puzzleSolved = true;
    }
  }

  if (puzzleSolved) {
    // Inform Command the puzzle has been solved
    sendSolved(true);
    instructionLine = 0;
    setPuzzleState(PuzzleStates::Solved);
  }

}



void setup() {
  Serial.begin(9600);
  Serial.println(__FILE__);
  Serial.println(F("Compiled: " __DATE__ ", " __TIME__));
  Serial.print(F("Puzzle   ID: "));
  Serial.println(PuzzleId);
  Serial.print(F("Puzzle Name: "));
  Serial.println(PuzzleShortName);


  // Set the mode for each of the sets of pins
  for (int p=0; p<4; p++) {
    pinMode(driverPins[p], OUTPUT);
    digitalWrite(driverPins[p], HIGH);      // Inverted
    pinMode(receivePins[p], INPUT_PULLUP);
    pinMode(bitLightPins[p], OUTPUT);
    digitalWrite(bitLightPins[p], LOW);
  }

  setupPJON(PuzzleId);

  // Initialize the status pixel and set initial puzzle state
  setupPuzzleStatus();

}

void loop() {

  loopPJON();

  switch (puzzleState) {
    case PuzzleStates::Starting:
      if (commandReady && command == COMMAND_WAKE) {
        performWake();
      }
      break;
    case PuzzleStates::Ready:
      if (commandReady && command == COMMAND_START) {
        performStart(commandArgument);
      }
      break;
    case PuzzleStates::Intialize:
      if (commandReady && command == COMMAND_NEXT) {
        if (instructionLine < initLines) {
          sendLine(Init[instructionLine++]);
        }
        clearCommand();
      }
      performInitialize();
      // The following if should be removed - it is only here for testing
      if (commandReady && command == COMMAND_PLAY) {
        setPuzzleState(PuzzleStates::Playing);
        clearCommand();
      }
      break;
    case PuzzleStates::Playing:
      if (commandReady && command == COMMAND_NEXT) {
        if (instructionLine < playLines) {
          sendLine(Play[instructionLine++]);
        }
        clearCommand();
      }
      performPlaying();
      // The following if should be removed - it is only here for testing
      if (commandReady && command == COMMAND_DONE) {
        setPuzzleState(PuzzleStates::Solved);
        clearCommand();
      }
      break;
    case PuzzleStates::Solved:
      if (commandReady && command == COMMAND_NEXT) {
        if (instructionLine < doneLines) {
          sendLine(Done[instructionLine++]);
        }
        clearCommand();
      }
      break;
    default:
      Serial.print(F("Invalid State: "));
      Serial.println(puzzleState);
      break;
  }
  if (commandReady) {
    if (commandReady && command == COMMAND_WAKE) {
      performWake();
    }
    else {
      Serial.print(F("Command '"));
      Serial.print(command);
      Serial.print(F("' "));
      if (commandArgument[0] != 0) {
        Serial.print(F("with args "));
        Serial.print(commandArgument);
      }
      Serial.print(F(" was not processed in state: "));
      Serial.println(puzzleStatesString[puzzleState]);
    }
    clearCommand();
  }

}


