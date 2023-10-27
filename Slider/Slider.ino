/*
 * Phone Home Slider Puzzle
 *
 * Move the sliders to find the right number which will be displayed on the analog meter display.
 * The first slider (green) adds 0-9 to the number.
 * The second slider (red) subtracts 0-9 from the number.
 * The third slider (green)adds 0-9 to the number.
 *
 * When the first slider reaches the target number it's led will light up ignoring the second and third slider.
 * When the first and second sliders reach the target number, those 2 leds will light up ignoring the third slider.
 * When the all 3 sliders reach the target number, all 3 leds will light up.
 *
 * All sliders must be not be zero for the puzzle to be solved.
 * The level of difficulty may change the behavior of the sliders.
 */

#include "PhoneHome_Definitions.h"
#include "PhoneHome_Command.h"
#include "PhoneHome_PuzzleState.h"

// Puzzle definitions
const int   PuzzleId  = SLIDER_ID;
const char* PuzzleShortName = "Slider";
const char* PuzzleLongName = "Slide into Home";
bool puzzleInitialized = false;
char puzzleDifficulty = " ";
#define MAX_NUMBER_LENGTH 1


// Tuning Commands
#define TUNE_COMMAND_UP   'U'  // Go up in value
#define TUNE_COMMAND_DOWN 'D'  // Go down in value

int16_t tuneValue = 0;
int16_t lastTuneValue = 0;

// Slider pins
const uint8_t slider1Pin = A4;
const uint8_t slider2Pin = A3;
const uint8_t slider3Pin = A2;

// Slider LED pins
const uint8_t slider1LedPin = 4;
const uint8_t slider2LedPin = 3;
const uint8_t slider3LedPin = 2;

// Analog meter pins
const uint8_t analogMeterPlusPin = 9;
const uint8_t analogMeterNegPin = 10;

int targetNumber = 7;

// Last values to control debug prints
int lastSlider1Number = 0;
int lastSlider2Number = 0;
int lastSlider3Number = 0;

// Number (-15 to +15) to PWM Values (-255 to +255)
const int16_t Number2PWMValue[] = {-255, -254, -238, -219, -200, -182, -163, -144, -126, -107,  -89,  -70,  -53,  -34,  -16,
                                      0,
                                     20,   39,   57,   76,   95,  113,  132,  151,  170,  187,  206,  225,  243,  254,  255};

// Instuctions
int instructionLine = 0;

//
// Instrutions for initializing the puzzle
//
// Limit to 20 chars  "                    "
const char Init_1[] = "Move All Sliders";
const char Init_2[] = "Down to Bottom";

const char *const Init[] =
{   
  Init_1,
  Init_2
};

const int initLines = 2;

//
// Instructions for playing puzzle
//
// Limit to 20 chars  "                    "
const char Play_1[] = "Adjust Sliders until";
const char Play_2[] = "All LEDS are ON.";
//const char Play_3[] = "  Some + others -";

const char *const Play[] =
{   
  Play_1,
  Play_2
//  Play_3
};

const int playLines = 2;

//
// Instructions for determining number
//
// Limit to 20 chars  "                    "
const char Done_1[] = "The Number will the";
const char Done_2[] = "Value on the Meter.";

const char *const Done[] =
{   
  Done_1,
  Done_2
};

const int doneLines = 2;

// Read a slider value and convert it to 0-9
int sliderRead(uint8_t sliderPin) {
  int sliderValue = analogRead(sliderPin);
  return 9-constrain(sliderValue / 100, 0, 9);
}

// Turn on all visible lights and indicators for a little while as a test
void flashDisplays () {
  Serial.println(F("Flashing Displays"));
  // Turn on Lights and other indicators
  digitalWrite(slider1LedPin, HIGH);
  digitalWrite(slider2LedPin, HIGH);
  digitalWrite(slider3LedPin, HIGH);
  digitalWrite(analogMeterPlusPin, HIGH);
  digitalWrite(analogMeterNegPin, LOW);
  delay(FLASH_DISPLAYS_DURATION);
  digitalWrite(analogMeterPlusPin, LOW);
  digitalWrite(analogMeterNegPin, HIGH);
  delay(FLASH_DISPLAYS_DURATION);
  // Turn off Lights and other indicators
  digitalWrite(slider1LedPin, LOW);
  digitalWrite(slider2LedPin, LOW);
  digitalWrite(slider3LedPin, LOW);
  digitalWrite(analogMeterPlusPin, LOW);
  digitalWrite(analogMeterNegPin, LOW);
}

void displayMeterNumber (int8_t sliderNumber) {
  int8_t meterNumber = constrain(sliderNumber, -15, 15);
  int16_t meterValue = Number2PWMValue[meterNumber+15];
  //int16_t meterValue = (255 * meterNumber)/15;
  displayMeterValue(meterValue);
}

void displayMeterValue (int16_t meterValue) {

  switch (meterValue) {
    case 0:
      digitalWrite(analogMeterPlusPin, LOW);
      digitalWrite(analogMeterNegPin, LOW);
      Serial.println("Full 0");
      break;
    case 255:
      digitalWrite(analogMeterPlusPin, HIGH);
      digitalWrite(analogMeterNegPin, LOW);
      Serial.println("Full 15");
      break;
    case -255:
      digitalWrite(analogMeterPlusPin, LOW);
      digitalWrite(analogMeterNegPin, HIGH);
      Serial.println("Full -15");
      break;
    default:
      uint8_t pwmPin = analogMeterPlusPin;
      uint8_t gndPin = analogMeterNegPin;
      uint8_t pwmValue = abs(meterValue);

      if (meterValue<0) {
        pwmPin = analogMeterNegPin;
        gndPin = analogMeterPlusPin;
      }

      analogWrite(gndPin, 0);
      digitalWrite(gndPin,LOW);
      analogWrite(pwmPin, pwmValue);
      break;
  }
}


// Check if the puzzle is in a ready state
// All sliders must be returned to 0
bool puzzleReady() {
  int slider1Number = sliderRead(slider1Pin);
  int slider2Number = sliderRead(slider2Pin);
  int slider3Number = sliderRead(slider3Pin);


  return (slider1Number == 0 && slider2Number == 0 && slider3Number == 0);
}


// Perform actions when a Wake command is received
void performWake() {
  flashDisplays();
  sendAck(MAX_NUMBER_LENGTH, PuzzleShortName);
  setPuzzleState(PuzzleStates::Ready);
  clearCommand();

}


// Perform actions asked to Tune
void performTune(char command) {
  uint8_t adjAmount = 1;
  if (commandArgument[0] != 0) {
    adjAmount = 0;
    for (uint8_t c=0; c<strlen(commandArgument); c++) {
      adjAmount = (adjAmount * 10) + (commandArgument[c] - '0');
    }
  }
  switch (command) {
    case COMMAND_TUNE:
      //sendAck(PuzzleName);
      setPuzzleState(PuzzleStates::Tuning);
      tuneValue = 0;
      lastTuneValue = -1;
      clearCommand();
      break;
    case TUNE_COMMAND_UP:
      tuneValue += adjAmount;
      if (tuneValue > 255) tuneValue = 255;
      clearCommand();
      break;
    case TUNE_COMMAND_DOWN:
      tuneValue -= adjAmount;
      if (tuneValue < -255) tuneValue = -255;
      clearCommand();
      break;
  }
  if (tuneValue != lastTuneValue) {
    Serial.print(F("PWM Value: "));
    Serial.println(tuneValue);
    displayMeterValue(tuneValue);
    lastTuneValue = tuneValue;
  }

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
  if (!puzzleInitialized) {
    puzzleInitialized = true;
    if (!puzzleReady()) {
      // Need to let Command know we will be in this state for a while
      sendInitialize(PuzzleLongName);
      instructionLine = 0;
    }
  }
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
  // Default is to have the LEDs off
  uint8_t slider1LedValue = LOW;
  uint8_t slider2LedValue = LOW;
  uint8_t slider3LedValue = LOW;

  // Get the slider positions
  int slider1Number = sliderRead(slider1Pin);
  int slider2Number = sliderRead(slider2Pin);
  int slider3Number = sliderRead(slider3Pin);


  int slider12Number = slider1Number - slider2Number;
  int slider123Number = slider1Number - slider2Number + slider3Number;
  if (slider1Number != 0 && slider2Number != 0 && slider3Number != 0 && slider123Number == targetNumber) {
    slider1LedValue = HIGH;
    slider2LedValue = HIGH;
    slider3LedValue = HIGH;
  }
  else if (slider1Number != 0 && slider2Number != 0 && slider3Number == 0 && slider12Number == targetNumber) {
    slider1LedValue = HIGH;
    slider2LedValue = HIGH;
  }
  else if (slider1Number != 0 && slider2Number == 0 && slider3Number == 0 && slider1Number == targetNumber) {
    slider1LedValue = HIGH;
  }
  
  // Only display and print if there is a change
  if (lastSlider1Number != slider1Number || lastSlider2Number != slider2Number || lastSlider3Number != slider3Number) {
    // Turn on/off LEDs as appropriate
    digitalWrite(slider1LedPin, slider1LedValue);
    digitalWrite(slider2LedPin, slider2LedValue);
    digitalWrite(slider3LedPin, slider3LedValue);

    displayMeterNumber(slider123Number);

    lastSlider1Number = slider1Number;
    lastSlider2Number = slider2Number;
    lastSlider3Number = slider3Number;
    Serial.print(F("1: "));
    Serial.print(slider1Number);
    Serial.print(F("; "));
    Serial.print(F("2: "));
    Serial.print(slider2Number);
    Serial.print(F("; "));
    Serial.print(F("3: "));
    Serial.print(slider3Number);
    Serial.print(F("; "));
    Serial.print(F("1-2: "));
    Serial.print(slider12Number);
    Serial.print(F("; "));
    Serial.print(F("1-2+3: "));
    Serial.print(slider123Number);
    Serial.print(F("; "));
    //Serial.print(F("Meter: "));
    //Serial.print(meterNumber);
    //Serial.print(F(" -> "));
    //Serial.print(meterValue);
    Serial.println();
  }

  bool puzzleSolved = false;

  // Minimum requirement for all complexity levels is slider not 0 and target number reached
  if (slider1Number != 0 && slider2Number != 0 && slider3Number != 0 && slider123Number == targetNumber) {
    switch (puzzleDifficulty) {
      case DIFFICULTY_EASY:
      default:
        // For Easy all sliders have to not be zero
        puzzleSolved = true;
        break;
      case DIFFICULTY_MEDIUM:
        // None of the slider positions can be the same
        puzzleSolved = (slider1Number != slider2Number && slider2Number != slider3Number && slider1Number != slider3Number);
        break;
      case DIFFICULTY_HARD:
        // Must be 2 away
        puzzleSolved = (abs(slider1Number - slider2Number) > 1 && abs(slider2Number - slider3Number) > 1 && abs(slider1Number - slider3Number) > 1);
        break;
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

  // Set up the slider input pins
  pinMode(slider1Pin, INPUT);
  pinMode(slider2Pin, INPUT);
  pinMode(slider3Pin, INPUT);

  // Set up the slider led output pins
  pinMode(slider1LedPin, OUTPUT);
  pinMode(slider2LedPin, OUTPUT);
  pinMode(slider3LedPin, OUTPUT);

  // Make sure the slider LEDs are off
  digitalWrite(slider1LedPin, LOW);
  digitalWrite(slider2LedPin, LOW);
  digitalWrite(slider3LedPin, LOW);

  // Set up the meter pins
  pinMode(analogMeterPlusPin, OUTPUT);
  pinMode(analogMeterNegPin, OUTPUT);
  digitalWrite(analogMeterPlusPin, LOW);
  digitalWrite(analogMeterNegPin, LOW);

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
    if (command == COMMAND_WAKE) {
      // Could perform wake at any time due to time limit reached
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


