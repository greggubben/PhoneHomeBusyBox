/*
 * FlipBits
 *
 * Flip bits using toggle switches to find the right binary number on the display
 * Each toggle switch represents one bit between 2^0 (1) and 2^7 (128)
 * The 0 to 255 decimal number will be represented on the display in hex 00 to FF
 * The display will flash Lo if the number is lower than target and
 * the display will flash Hi if the number is higher than target.
 */

#include <Bounce2.h>
#include <Adafruit_MCP23X17.h>
#include <PhoneHome_Definitions.h>
#include <PhoneHome_Command.h>
#include <PhoneHome_PuzzleState.h>

// Puzzle definitions
const int   PuzzleId  = FLIPBITS_ID;
const char* PuzzleShortName = "Flip";
const char* PuzzleLongName = "Flip the Bits";
bool puzzleInitialized = false;
char puzzleDifficulty = DIFFICULTY_EASY;
#define MAX_NUMBER_LENGTH 2


// Target number
int targetNumber = 50;

// INPUT TOGGLE DEFINITIONS
const int togglePins[8] = {2,3,4,5,6,7,8,9};
Bounce bounceToggles[8] = {Bounce(), Bounce(), Bounce(), Bounce(), Bounce(), Bounce(), Bounce(), Bounce()};

// Blink display based on Hi or Lo
bool showNum = true;
uint16_t numPattern = 0x0000;
uint16_t dirPattern = 0x0000;
unsigned long blinkDuration = 0;
unsigned long startMillis = millis();
#define INIT_DURATION 1000
#define NUM_DURATION 3000
#define DIR_DURATION 1000


// 7 SEGMENT DISPLAY DEFINITIONS

// number to binary 7-segment is 0bpgfedcba
//
//  -a-
// |   |
// f   b
// |   |
//  -g-
// |   |
// e   c
// |   |
//  -d-   p
//
// byte layout: pgfedcba

const uint8_t num2seg[] = { 0b00111111, // 0
                            0b00000110, // 1
                            0b01011011, // 2
                            0b01001111, // 3
                            0b01100110, // 4
                            0b01101101, // 5
                            0b01111101, // 6
                            0b00000111, // 7
                            0b01111111, // 8
                            0b01100111, // 9
                            0b01110111, // A
                            0b01111100, // B
                            0b00111001, // C
                            0b01011110, // D
                            0b01111001, // E
                            0b01110001  // F
};

const uint16_t Lo2seg = 0b0011100001011100; // Lo
const uint16_t Hi2seg = 0b0111011000000100; // Hi

// 16 bit pia extender
Adafruit_MCP23X17 mcp;


uint16_t lastByte = 0x100;  // This is outside the range of uint8 so first time through will not match

// Instuctions
int instructionLine = 0;

//
// Instrutions for initializing the puzzle
//
// Limit to 20 chars  "                    "
const char Init_1[] = "Flip All Switches";
const char Init_2[] = "Down to Off";

const char *const Init[] =
{   
  Init_1,
  Init_2
};

const int initLines = 2;

//
// Instructions for playing puzzle
//
// Limit to 20 chars          "                    "
const char Play_1[] = "Flip Switches to add";
const char Play_2[] = "or remove value/bit";
const char Play_3[] = "Hi = number too High";
const char Play_4[] = "Lo = number too Low";

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
const char Done_1[] = "Swap HEX for Decimal";
const char Done_2[] = "with Hexadaisy inner";
const char Done_3[] = "ring of 'DEC.HX'.";

const char *const Done[] =
{   
  Done_1,
  Done_2,
  Done_3
};

const int doneLines = 3;



// 7-SEGMENT FUNCTIONS


// Convert a byte to two 7-segment patterns.
uint16_t byte2seg (uint8_t b) {
  uint8_t upperNibble = b >> 4;
  uint8_t lowerNibble = 0b00001111 & b;
  return (num2seg[upperNibble] << 8) | num2seg[lowerNibble];
}

// Update the display with segment pattern
void updateDisplay(uint16_t segmentPattern) {
  mcp.writeGPIOAB(0xFFFF ^ segmentPattern);
}

// display a Byte
void displayByte(uint8_t b) {
    updateDisplay(byte2seg(b));
}


// TOGGLE FUNCTIONS


// Read the value of the toggles
uint8_t readToggles() {
  uint8_t readByte = 0;
  for (int t=0; t<8; t++) {
    //Bounce bounceToggle = bounceToggles[t];
    bounceToggles[t].update();
    readByte = readByte << 1 | bounceToggles[t].read();
  }
  return readByte;
}

// Turn on all visible lights and indicators for a little while as a test
void flashDisplays () {
  Serial.println(F("Flashing Displays"));
  // Turn on Lights and other indicators
  updateDisplay(0xFFFF);
  delay(FLASH_DISPLAYS_DURATION);
  // Turn off Lights and other indicators
  updateDisplay(0x0000);
}


// Check if the puzzle is in a ready state
bool puzzleReady() {
  bool puzzReady = false;
  uint8_t toggleValue = readToggles();
  puzzReady = (toggleValue == 0);

  if (puzzReady) {
    updateDisplay(0x0000);    // Blank display
  }
  else {
    displayByte(toggleValue); // Show the number
  }

  return puzzReady;
}

// PUZZLE STATE FUNCTIONS

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
  targetNumber = (commandArgument.length() > 1) ? commandArgument.substring(1).toInt() : targetNumber;
  targetNumber = constrain(targetNumber, 0, 255);
}

// Perform the Initialization steps including any randomizations
void performInitialize() {

  // Perform first time tasks such as randomization of drivers
  if (!puzzleInitialized) {
    puzzleInitialized = true;
    int remainingPins[8];

    switch (puzzleDifficulty) {
      case DIFFICULTY_EASY:
      default:
        // Pins will be in order for Easy
        for (int p=0; p<8; p++) {
          bounceToggles[p].attach(togglePins[p]);                         // This also resets the debouncer
        }
        break;
      case DIFFICULTY_MEDIUM:
        // Randomize within each Nibble - Low with Low and High within High
        for (uint8_t nibble=0; nibble<8; nibble+=4) {
          for (int p=0; p<4;p++) {remainingPins[p] = togglePins[nibble+p];}        // Create shrinking array
          for (int t=0; t<4; t++) {
            Serial.print(F("."));
            int pinsLeftToAssign = 3 - t;
            int randomToggle =  random(pinsLeftToAssign);
            bounceToggles[nibble+t].attach(remainingPins[randomToggle]);           // This also resets the debouncer
            remainingPins[randomToggle] = remainingPins[pinsLeftToAssign];  // Move last element to one just used
          }
        }
        break;
      case DIFFICULTY_HARD:
        // Randomize all toggles Low Nibble mixed with High Nibble
        for (int p=0; p<8;p++) {remainingPins[p] = togglePins[p];}        // Create shrinking array
        for (int t=0; t<8; t++) {
          Serial.print(F("."));
          int pinsLeftToAssign = 7 - t;
          int randomToggle =  random(pinsLeftToAssign);
          bounceToggles[t].attach(remainingPins[randomToggle]);           // This also resets the debouncer
          remainingPins[randomToggle] = remainingPins[pinsLeftToAssign];  // Move last element to one just used
        }
        Serial.println(F(" Done"));
        break;
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

  uint8_t readByte = readToggles();

  // See if there is a change
  if (lastByte != readByte) {
    Serial.print("Byte: ");
    Serial.println(readByte);
    numPattern = byte2seg(readByte);
    updateDisplay(numPattern);
    dirPattern = (readByte < targetNumber) ? Lo2seg : Hi2seg;
    showNum = true;
    blinkDuration = INIT_DURATION;
    startMillis = millis();
    lastByte = readByte;
  }

  // Make the number blink if correct
  if (readByte == targetNumber) {
    puzzleSolved = true;
  }
  else {
    if (millis() - startMillis >= blinkDuration) {
      showNum = !showNum;
      if (showNum) {
        updateDisplay(numPattern);
        blinkDuration = NUM_DURATION;
      }
      else {
        updateDisplay(dirPattern);
        blinkDuration = DIR_DURATION;
      }
      startMillis = millis();
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

  Serial.print(F("MCP23017 . "));
  mcp.begin_I2C();
  Serial.println(F("Started"));
  delay(200);

  // configure toggles for input with pullup
  Serial.print(F("Toggle Switches "));
  for (int t=0; t<8; t++) {
    Serial.print(F("."));
    bounceToggles[t].interval(5);
    bounceToggles[t].attach(togglePins[t], INPUT_PULLUP);
  }
  Serial.println(F(" Initialized"));

  // configure all 16 pins for output on port chip for 7-segment display
  Serial.print(F("7-Segment pins "));
  for (int p = 0; p<16; p++) {
    Serial.print(F("."));
    mcp.pinMode(p, OUTPUT);
  }
  Serial.println(F(" Initialized"));
  updateDisplay(0x0000);

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

