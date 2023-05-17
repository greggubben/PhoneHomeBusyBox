// Connect Wires
// Receive Pins (top row of pins) and bit lights should always match
// The driver pins will be randomized

#include <Arduino.h>
#include <PhoneHome_Definitions.h>
#include <PhoneHome_Command.h>
#include <PhoneHome_PuzzleState.h>


// Puzzle definitions
const int   PuzzleId  = WIRES_ID;
const char* PuzzleName = "Wires";
bool puzzleInitialized = false;
char puzzleDifficulty = DIFFICULTY_EASY;


// Pin definitions
int driverPins[]  = {5,4,3,2};
int receivePins[] = {9,8,7,6};
int bitLightPins[]  = {A2, A3, A4, A5};
int puzzleDriverPins[4];

byte targetNumber = 9;


// Turn on all visible lights and indicators for a little while as a test
void flashDisplays () {
  Serial.println(F("Flashing Displays"));
  // Turn on Lights and other indicators
  for (int p = 0; p < 4; p++) {
    digitalWrite(bitLightPins[p], HIGH);
  }
  delay(500);
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
  sendAck(PuzzleName);
  setPuzzleState(PuzzleStates::Connected);
  clearCommand();

}

// Perform actions when a Start command is received
void performStart(String commandArgument) {
  puzzleInitialized = false;
  setPuzzleState(PuzzleStates::Intialize);
  clearCommand();

  puzzleDifficulty = (commandArgument.length() > 0) ? commandArgument[0] : DIFFICULTY_EASY;
  targetNumber = (commandArgument.length() > 1) ? commandArgument.substring(1).toInt() : 9;
  targetNumber = constrain(targetNumber, 0, 15);
}

// Perform the Initialization steps including any randomizations
void performInitialize() {

  // Perform first time tasks such as randomization of drivers
  if (!puzzleInitialized) {
    puzzleInitialized = true;

    switch (puzzleDifficulty) {
      case DIFFICULTY_EASY:
      default:
        for (int p=0; p<4; p++) {
          puzzleDriverPins[p] = driverPins[p];
        }
        break;
      case DIFFICULTY_HARD:
      case DIFFICULTY_MEDIUM:
        // Randomize the driverPins
        int remainingPins[4];
        for (int p=0; p<4; p++) {
          remainingPins[p] = driverPins[p];
        }
        for (int p=0; p<4; p++) {
          int leftPins = 4-p;
          int rndPos = random(leftPins);
          puzzleDriverPins[p] = remainingPins[rndPos];
          remainingPins[rndPos] = remainingPins[leftPins-1];
        }
        for (int p=0; p<4; p++) {
          Serial.print(F("Driver: "));
          Serial.print(driverPins[p]);
          Serial.print(F(" --> "));
          Serial.print(puzzleDriverPins[p]);
          Serial.print(F(" to Receiver: "));
          Serial.print(receivePins[p]);
          Serial.print(F(" for Light: "));
          Serial.print(bitLightPins[p]);
          Serial.println();
        }
        break;
    }

    if (!puzzleReady()) {
      // Need to let Command know we will be in this state for a while
      sendInitialize();
    }
  }

  // Wait until the puzzle has been reset by player
  if (puzzleReady()) {
    // Good to go - Let's play
    // Inform Command we are playing
    sendPlay();
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
    byte r = digitalRead(receivePins[p]);
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
    sendSolved();
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
  Serial.println(PuzzleName);

  // Initialize the status pixel and set initial puzzle state
  setupPuzzleStatus();

  // Set the mode for each of the sets of pins
  for (int p=0; p<4; p++) {
    pinMode(driverPins[p], OUTPUT);
    digitalWrite(driverPins[p], HIGH);      // Inverted
    pinMode(receivePins[p], INPUT_PULLUP);
    pinMode(bitLightPins[p], OUTPUT);
    digitalWrite(bitLightPins[p], LOW);
  }

}

void loop() {
  switch (puzzleState) {
    case PuzzleStates::Starting:
      if (commandReady && command == COMMAND_WAKE) {
        performWake();
      }
      break;
    case PuzzleStates::Connected:
      if (commandReady && command == COMMAND_START) {
        performStart(commandArgument);
      }
      break;
    case PuzzleStates::Intialize:
      performInitialize();
      // The following if should be removed - it is only here for testing
      if (commandReady && command == COMMAND_PLAY) {
        setPuzzleState(PuzzleStates::Playing);
        clearCommand();
      }
      break;
    case PuzzleStates::Playing:
      performPlaying();
      // The following if should be removed - it is only here for testing
      if (commandReady && command == COMMAND_DONE) {
        setPuzzleState(PuzzleStates::Solved);
        clearCommand();
      }
      break;
    case PuzzleStates::Solved:
      if (commandReady && command == COMMAND_WAKE) {
        performWake();
      }
      break;
    default:
      Serial.print(F("Invalid State: "));
      Serial.println(puzzleState);
      break;
  }
  if (commandReady) {
    Serial.print(F("Command '"));
    Serial.print(command);
    Serial.print(F("' "));
    if (commandArgument.length() > 0) {
      Serial.print(F("with args "));
      Serial.print(commandArgument);
    }
    Serial.print(F(" was not processed in state: "));
    Serial.println(puzzleStatesString[puzzleState]);
    clearCommand();
  }

}


