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
const char* PuzzleName = "Slider";
bool puzzleInitialized = false;
char puzzleDifficulty = " ";


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
  delay(1000);
  digitalWrite(analogMeterPlusPin, LOW);
  digitalWrite(analogMeterNegPin, HIGH);
  delay(1000);
  // Turn off Lights and other indicators
  digitalWrite(slider1LedPin, LOW);
  digitalWrite(slider2LedPin, LOW);
  digitalWrite(slider3LedPin, LOW);
  digitalWrite(analogMeterPlusPin, LOW);
  digitalWrite(analogMeterNegPin, LOW);
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
  sendAck(PuzzleName);
  setPuzzleState(PuzzleStates::Ready);
  clearCommand();

}

// Perform actions when a Start command is received
void performStart(String commandArgument) {
  puzzleInitialized = false;
  setPuzzleState(PuzzleStates::Intialize);
  clearCommand();

  puzzleDifficulty = commandArgument[0];
  targetNumber = commandArgument.substring(1,2).toInt();
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
  
  // Turn on/off LEDs as appropriate
  digitalWrite(slider1LedPin, slider1LedValue);
  digitalWrite(slider2LedPin, slider2LedValue);
  digitalWrite(slider3LedPin, slider3LedValue);

  int meterNumber = constrain(slider123Number, -15, 15);
  int meterValue = (255 * abs(meterNumber))/15;
  uint8_t pwmPin = analogMeterPlusPin;
  uint8_t gndPin = analogMeterNegPin;
  if (meterNumber <0) {
    pwmPin = analogMeterNegPin;
    gndPin = analogMeterPlusPin;
  }

  analogWrite(gndPin, 0);
  digitalWrite(gndPin,LOW);
  analogWrite(pwmPin, meterValue);

  // Only print if there is a change
  if (lastSlider1Number != slider1Number || lastSlider2Number != slider2Number || lastSlider3Number != slider3Number) {
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
    Serial.print(F("Meter: "));
    Serial.print(meterNumber);
    Serial.print(F(" -> "));
    Serial.print(meterValue);
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
        puzzleSolved = (slider1Number > slider2Number && slider2Number > slider3Number);
        break;
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


