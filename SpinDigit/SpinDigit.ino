/*
 * FlipBits
 *
 * Flip bits using toggle switches to find the right binary number on the display
 * Each toggle switch represents one bit between 2^0 (1) and 2^7 (128)
 * The 0 to 255 decimal number will be represented on the display in hex 00 to FF
 * The display will flash Lo if the number is lower than target and
 * the display will flash Hi if the number is higher than target.
 */

#include <Adafruit_MCP23X17.h>
#include <PhoneHome_Definitions.h>
#include <PhoneHome_Command.h>
#include <PhoneHome_PuzzleState.h>

// Puzzle definitions
const int   PuzzleId  = SPINDIGIT_ID;
const char* PuzzleName = "Spin Digit";
bool puzzleInitialized = false;
char puzzleDifficulty = DIFFICULTY_EASY;

// Target number
int targetNumber = 163;
int targetDigits[3] = {1,6,3};

#define DIAL_LIGHT_PIN 2
#define ODD_BAR_PIN 5
#define EVEN_BAR_PIN 6

// Tuning Commands
const char TUNE_COMMAND_UP   = 'U';  // Go up in value
const char TUNE_COMMAND_DOWN = 'D';  // Go down in value


// 16 bit pia extender
Adafruit_MCP23X17 mcp;


uint16_t lastRead  = 0x1000;  // This is outside the range of uint8 so first time through will not match
uint16_t dialValue = 0x1000;  // This is outside the range of uint8 so first time through will not match
int dialDigits[3] = {11,11,11}; // This is outside range of digits on the wheel
unsigned long changeStartMillis = 0;
uint16_t lastDialValue = 0x1000;
int initDigits[3] = {0,0,0};

// for tuning
int tuneValue = 0;
int lastTuneValue = 0;

// digit to pwm value
void pwmWrite (uint8_t pin, int digit) {
  int pwmValue = 0;
  if (digit > 11) digit = 11;
  if (digit > 0) {
    pwmValue = (digit*11) + 3;
  }
  if (pwmValue == 0) {
    digitalWrite(pin, LOW);
  }
  else {
    analogWrite( pin, pwmValue);
  }
}

// Read the values of the dials
uint16_t readDials() {

  uint16_t readValue = mcp.readGPIOAB();

  readValue &= 0x0FFF;

  if (readValue != dialValue) {

    if (readValue != lastRead) {
      lastRead = readValue;
      changeStartMillis = millis();
    }

    // Wait for the dial to settle in
    if (millis() - changeStartMillis > 20) {
      dialValue = readValue;
      dialDigits[2] = dialValue >> 8;
      dialDigits[1] = (dialValue&0x0F0)>>4;
      dialDigits[0] = dialValue&0x00F;
      Serial.print("Dial Value = ");
      Serial.print(dialValue,HEX);
      Serial.println();
      Serial.print("Dial Digits = ");
      Serial.print(dialDigits[2]);
      Serial.print(", ");
      Serial.print(dialDigits[1]);
      Serial.print(", ");
      Serial.print(dialDigits[0]);
      Serial.println();
    }
  }

  return dialValue;
}


// Turn on all visible lights and indicators for a little while as a test
void flashDisplays () {
  Serial.println(F("Flashing Displays"));
  // Turn on Lights and other indicators
  digitalWrite(DIAL_LIGHT_PIN, HIGH);
  digitalWrite(ODD_BAR_PIN, HIGH);
  digitalWrite(EVEN_BAR_PIN, HIGH);
  delay(1000);
  // Turn off Lights and other indicators
  digitalWrite(DIAL_LIGHT_PIN, LOW);
  digitalWrite(ODD_BAR_PIN, LOW);
  digitalWrite(EVEN_BAR_PIN, LOW);
}

// Compare the digits
bool compareDigits(int current[3], int target[3]) {
  return ((current[0] == target[0]) && (current[1] == target[1]) && (current[2] == target[2]));
}


// Check if the puzzle is in a ready state
bool puzzleReady() {

  readDials();

  return compareDigits(dialDigits, initDigits);
}

// Display using the 2 bars how many digits away the solution is
void displayDigitsAway(int current[3], int target[3]) {
  int digitsAway = abs(current[0] - target[0]) + abs(current[1] - target[1]) + abs(current[2] - target[2]);
  int oddAway = digitsAway/2 + digitsAway%2;
  int evenAway = digitsAway/2;
  Serial.print(F("digits Away: "));
  Serial.print(digitsAway);
  Serial.print(F("  odd Away: "));
  Serial.print(oddAway);
  Serial.print(F("  even Away: "));
  Serial.print(evenAway);
  Serial.println();

  pwmWrite(ODD_BAR_PIN, oddAway);
  pwmWrite(EVEN_BAR_PIN, evenAway);
}

// PUZZLE STATE FUNCTIONS

// Perform actions when a Wake command is received
void performWake() {
  flashDisplays();
  sendAck(PuzzleName);
  setPuzzleState(PuzzleStates::Ready);
  clearCommand();

}

// Perform actions asked to Tune
void performTune(char command) {
  switch (command) {
    case COMMAND_TUNE:
      sendAck(PuzzleName);
      setPuzzleState(PuzzleStates::Tuning);
      tuneValue = 0;
      lastTuneValue = -1;
      break;
    case TUNE_COMMAND_UP:
      tuneValue++;
      if (tuneValue > 255) tuneValue = 255;
      break;
    case TUNE_COMMAND_DOWN:
      tuneValue--;
      if (tuneValue < 0) tuneValue = 0;
      break;
  }
  if (tuneValue != lastTuneValue) {
    Serial.print(F("PWM Value: "));
    Serial.println(tuneValue);
    analogWrite(ODD_BAR_PIN, tuneValue);
    analogWrite(EVEN_BAR_PIN, tuneValue);
    lastTuneValue = tuneValue;
  }
  clearCommand();

}

// Perform actions when a Start command is received
void performStart(String commandArgument) {
  puzzleInitialized = false;
  setPuzzleState(PuzzleStates::Intialize);
  clearCommand();

  puzzleDifficulty = (commandArgument.length() > 0) ? commandArgument[0] : DIFFICULTY_EASY;
  targetNumber = (commandArgument.length() > 1) ? commandArgument.substring(1).toInt() : 163;
  targetNumber = constrain(targetNumber, 0, 999);
  targetDigits[2] = targetNumber/100;
  targetDigits[1] = (targetNumber%100)/10;
  targetDigits[0] = targetNumber%10;
  Serial.print("Difficulty: ");
  Serial.println(puzzleDifficulty);
  Serial.print("Target Number: ");
  Serial.println(targetNumber);
  Serial.print("Target Digits: ");
  Serial.print(targetDigits[2]);
  Serial.print(", ");
  Serial.print(targetDigits[1]);
  Serial.print(", ");
  Serial.print(targetDigits[0]);
  Serial.println();

  digitalWrite(DIAL_LIGHT_PIN, HIGH);

}

// Perform the Initialization steps including any randomizations
void performInitialize() {

  // Perform first time tasks such as randomization of drivers
  if (!puzzleInitialized) {
    puzzleInitialized = true;

    lastDialValue = 0x1000;

    switch (puzzleDifficulty) {
      case DIFFICULTY_EASY:
      case DIFFICULTY_MEDIUM:
      default:
        // Digits will be in order for Easy and Medium
        break;
      case DIFFICULTY_HARD:
        // Digits will be randomized for Hard
        //Serial.print(F("Randomize Digits "));
        //int remainingDigits[3];
        //for (int p=0; p<3;p++) {remainingDigits[p] = targetDigits[p];}        // Create shrinking array
        //for (int t=0; t<3; t++) {
        //  Serial.print(F("."));
        //  int digitsLeftToAssign = 2 - t;
        //  int randomDigit =  random(digitsLeftToAssign);
        //  targetDigits[t] = remainingDigits[randomDigit];
        //  remainingDigits[randomDigit] = remainingDigits[digitsLeftToAssign];  // Move last element to one just used
        //}
        //Serial.println(F(" Done"));
        break;
    }

    if (!puzzleReady()) {
      // Need to let Command know we will be in this state for a while
      sendInitialize();
    }
  }

  if (dialValue != lastDialValue) {
    // Display the digits away
    displayDigitsAway(dialDigits, initDigits);
    lastDialValue = dialValue;
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

  readDials();

  if (dialValue != lastDialValue) {
    // Display the digits away
    displayDigitsAway(dialDigits, targetDigits);
    lastDialValue = dialValue;
    puzzleSolved = compareDigits(dialDigits, targetDigits);
  }

  if (puzzleSolved) {
    // Inform Command the puzzle has been solved
    digitalWrite(DIAL_LIGHT_PIN, LOW);
    sendSolved();
    setPuzzleState(PuzzleStates::Solved);
  }
}


void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println(__FILE__);
  Serial.println(F("Compiled: " __DATE__ ", " __TIME__));
  Serial.print(F("Puzzle   ID: "));
  Serial.println(PuzzleId);
  Serial.print(F("Puzzle Name: "));
  Serial.println(PuzzleName);

  // Initialize the status pixel and set initial puzzle state
  setupPuzzleStatus();

  Serial.print(F("MCP23017 . "));
  mcp.begin_I2C();
  Serial.println(F("Started"));
  delay(200);

  pinMode(DIAL_LIGHT_PIN, OUTPUT);
  digitalWrite(DIAL_LIGHT_PIN, LOW);
  pinMode(ODD_BAR_PIN, OUTPUT);
  digitalWrite(ODD_BAR_PIN, LOW);
  pinMode(EVEN_BAR_PIN, OUTPUT);
  digitalWrite(EVEN_BAR_PIN, LOW);

  // configure pins for each of Digit Dials for input with pullup
  Serial.print(F("Digit Dial Inputs "));
  for (int p = 0; p<16; p++) {
    Serial.print(F("."));
    mcp.pinMode(p, INPUT_PULLUP);
  }
  Serial.println(F(" Initialized"));

  Serial.println("SETUP complete");
  Serial.println();
}

void loop() {
  switch (puzzleState) {
    case PuzzleStates::Starting:
      if (commandReady && command == COMMAND_WAKE) {
        performWake();
      }
      if (commandReady && command == COMMAND_TUNE) {
        performTune(command);
      }
      break;
    case PuzzleStates::Tuning:
      if (commandReady) {
        performTune(command);
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
      digitalWrite(DIAL_LIGHT_PIN, LOW);
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

