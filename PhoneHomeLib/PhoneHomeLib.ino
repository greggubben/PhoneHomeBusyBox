/*
 * Control
 * This code is a part of the Phone Home Briefcase Game.
 * The Control module will:
 *    - start the game
 *    - divide up the phone number
 *    - delegate to each puzzle for solving
 *    - display instructions and hints
 *    - celebrate completion
 * 
 * The Control module manages the overall state of the game with the state of
 * each puzzle will be managed locally by the puzzle. Common definitions will be
 * defined by an include header file maintained by the Control module.
 */

#include "PhoneHome_Definitions.h"
#include "PhoneHome_Command.h"
#include "PhoneHome_PuzzleState.h"

// Puzzle definitions
const int   PuzzleId  = TEST_ID;
const char* PuzzleName = "Test";
bool puzzleInitialized = false;
char puzzleDifficulty = DIFFICULTY_EASY;

// Target number
int targetNumber = 50;


// Turn on all visible lights and indicators for a little while as a test
void flashDisplays () {
  Serial.println(F("Flashing Displays"));
  // Turn on Lights and other indicators
  delay(500);
  // Turn off Lights and other indicators
}


// Check if the puzzle is in a ready state
bool puzzleReady() {
  // Check for the puzzle to be placed in ready state
  // returning false for testing
  return true;
}


// Perform actions when a Wake command is received
void performWake() {
  flashDisplays();
  sendAck(PuzzleName);
  setPuzzleState(PuzzleStates::Ready);
  clearCommand();

}

// Perform actions when a Start command is received
void performStart(String commandArgument) {
  puzzleInitialized = false;
  setPuzzleState(PuzzleStates::Intialize);
  clearCommand();

  puzzleDifficulty = (commandArgument.length() > 0) ? commandArgument[0] : DIFFICULTY_EASY;
  targetNumber = (commandArgument.length() > 1) ? commandArgument.substring(1).toInt() : 50;
  targetNumber = constrain(targetNumber, 0, 255);
}

// Perform the Initialization steps including any randomizations
void performInitialize() {
  if (!puzzleInitialized) {
    puzzleInitialized = true;
    if (!puzzleReady()) {
      // Need to let Command know we will be in this state for a while
      sendInitialize();
    }
  }
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

}

void loop() {

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
