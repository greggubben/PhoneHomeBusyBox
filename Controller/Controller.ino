// Connect Wires
// Receive Pins (top row of pins) and bit lights should always match
// The driver pins will be randomized

#define PHONEHOME_CONTROLLER

#include <Arduino.h>
#include <PhoneHome_Definitions.h>
#include <PhoneHome_Command.h>
#include <PhoneHome_PuzzleState.h>
//#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
//#include <Adafruit_GFX.h>
//#include <SPI.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include "Melody.h"
#include "Entertainer.h"

// These definitions make the code more readable from the controller perspective
// allowing for library reuse and clarity of controller vs puzzle
#define ControllerStates        PuzzleStates
#define controllerState         puzzleState
#define setControllerState      setPuzzleState
#define controllerStatesString  puzzleStatesString
#define setupControllerStatus   setupPuzzleStatus

// Puzzle definitions
const int   ControllerId  = CONTROL_ID;
const char* ControllerName = "Controller";
//bool controlInitialized = false;
char puzzleDifficulty = DIFFICULTY_EASY;

//Speaker
#define SPEAKER_PIN 4

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27,20,4);

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 500
#define TS_MINY 500
#define TS_MAXX 3300
#define TS_MAXY 3500

// Touch Screen definitions
#define TS_CS 7   // Connected to LOLIN pin D3
XPT2046_Touchscreen ts(TS_CS);

// TFT Screen definitions
#define TFT_RST   8    // Connected to LOLIN pin RST
#define TFT_CS    9      // Connected to LOLIN pin
#define TFT_DC    10     // Connected to LOLIN pin 
//#define TFT_MOSO        // Connected to LOLIN pin 
//#define TFT_MOSI        // Connected to LOLIN pin 
//#define TFT_SCK         // Connected to LOLIN pin 
// initialize ILI9341 TFT library
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
#define TFT_WIDTH 320
#define TFT_HEIGHT 240

// Display order
const static int puzzleDisplayOrder[] = {SLIDER_ID, CONTROL_ID, FLIPBITS_ID, WIRES_ID, PHONE_ID, SPINDIGIT_ID};
const String puzzleDisplayNames[] = {"Slider", "Control", "Flip", "Wires", "Phone", "Spin"};
#define PUZZLE_DISPLAY_ORDER_LENGTH 6


// Puzzle display grid constants
#define PUZZLE_WIDTH   100
#define PUZZLE_HEIGHT  100
#define PUZZLE_RADIUS  10
#define PUZZLE_PADDING 5


const String difficultyDisplayNames[] = {"Easy", "Medium", "Hard"};
const uint16_t difficlutyDisplayColors[] = {ILI9341_GREEN, ILI9341_YELLOW, ILI9341_RED};
#define DIFFCULTY_DISPLAY_LENGTH 3

// Difficulty display grid constants
#define DIFFICULTY_WIDTH   100
#define DIFFICULTY_HEIGHT  220
#define DIFFICULTY_RADIUS  20
#define DIFFICULTY_PADDING 5

bool wasTouched = false;

typedef enum WakeStates {Unknown, Asked, Awake, Timedout} WakeStates;
const uint16_t wakeStateColors[] = {ILI9341_WHITE, ILI9341_YELLOW, ILI9341_GREEN, ILI9341_RED};

WakeStates puzzleWakeStatus[ID_ARRAY_LENGTH];

// Game Order
const static int puzzlePlayOrder[] = {SLIDER_ID, WIRES_ID, FLIPBITS_ID, SPINDIGIT_ID, PHONE_ID};
const String puzzlePlayNames[] = {"Slider", "Wires", "Flip", "Spin", "Phone"};
const String puzzlePlayValues[] = {"7", "9", "50", "163", "7950163"};  // Hack - upgrade to parsing logic
#define PUZZLE_PLAY_LENGTH 5

const char* targetNumber = "7950163";
int currentPuzzle = 0;
bool commandSent = false;
unsigned long commandSentAt = 0;
bool wakeTimeout = false;
#define TIMEOUT_MILLIS 5000


void playMusic(int speakerPin, Note music[]) {
    
  // iterate over the notes of the melody:
  int thisNote=0;
  while (music[thisNote].note != END_NOTES) {
  
    int dur = music[thisNote].duration;
    
    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000/music[thisNote].duration;
    tone(speakerPin, music[thisNote].note, noteDuration);
  
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);

    // stop the tone playing:
    noTone(speakerPin);
    thisNote++;
  }
}

// Draw just the one puzzle
void drawWakeStatus (int puzzle) {
    int thisPuzzleID = puzzleDisplayOrder[puzzle];
    int row = puzzle / 3;
    int col = puzzle % 3;
    int startX = col*(PUZZLE_WIDTH+PUZZLE_PADDING) + PUZZLE_PADDING;
    int startY = row*(PUZZLE_HEIGHT+PUZZLE_PADDING) + PUZZLE_PADDING;
    int width = PUZZLE_WIDTH;
    int height = PUZZLE_HEIGHT;

    tft.fillRoundRect(startX, startY, width, height, PUZZLE_RADIUS, wakeStateColors[puzzleWakeStatus[thisPuzzleID]]);

    // Draw gap
    //startX += PUZZLE_PADDING;
    //startY += PUZZLE_PADDING;
    //width -= PUZZLE_PADDING*2;
    //height -= PUZZLE_PADDING*2;
    //tft.fillRoundRect(startX, startY, width, height, PUZZLE_RADIUS, ILI9341_BLACK);

    // Draw Center
    //startX += PUZZLE_PADDING;
    //startY += PUZZLE_PADDING;
    //width -= PUZZLE_PADDING*2;
    //height -= PUZZLE_PADDING*2;
    //uint16_t boxBackgroundColor = ILI9341_DARKGREY;
    //tft.fillRoundRect(startX, startY, width, height, PUZZLE_RADIUS, boxBackgroundColor);

    // Draw Labels
    startX += PUZZLE_PADDING;
    startY += height/2;
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(startX, startY);
    tft.print(puzzleDisplayNames[puzzle]);

}

// Draw all the puzzles
void drawWakeStatus () {
  for (int puzzle=0; puzzle<PUZZLE_DISPLAY_ORDER_LENGTH; puzzle++) {
    drawWakeStatus(puzzle);
  }
}



// Setup for performing Wake State activities
void transitionToWakeState() {

  // Initialize the status pixel and set initial controller state Wake
  setupControllerStatus();

  for (int puzzle=0; puzzle<PUZZLE_DISPLAY_ORDER_LENGTH; puzzle++) {
    puzzleWakeStatus[puzzleDisplayOrder[puzzle]] = (puzzleDisplayOrder[puzzle] == CONTROL_ID) ? WakeStates::Awake : WakeStates::Unknown;
  }
  currentPuzzle = 0;
  commandSent = false;
  wakeTimeout = false;

  tft.fillScreen(ILI9341_BLACK);
  drawWakeStatus();

}

// Wake up all the Puzzles
void performWake() {
  if (puzzleDisplayOrder[currentPuzzle] == CONTROL_ID) {
    Serial.println(F("Skipping Controller"));
    currentPuzzle++;
  }

  if (!commandSent) {
    Serial.print(F("Waking "));
    Serial.println(puzzleDisplayOrder[currentPuzzle]);
    sendWake(puzzleDisplayOrder[currentPuzzle]);
    puzzleWakeStatus[puzzleDisplayOrder[currentPuzzle]] = WakeStates::Asked;
    commandSent = true;
    commandSentAt = millis();
    drawWakeStatus(currentPuzzle);
  }
  else {
    if (commandReady && command == COMMAND_ACK) {
      Serial.print(F("ACK Received from "));
      Serial.println(commandArgument);
      puzzleWakeStatus[puzzleDisplayOrder[currentPuzzle]] = WakeStates::Awake;
      drawWakeStatus(currentPuzzle);

      // Move on to Next puzzle
      currentPuzzle++;
      commandSent = false;
      clearCommand();
    }
    else if ((millis() - commandSentAt) > TIMEOUT_MILLIS) {
      Serial.println(F("Timeout"));
      puzzleWakeStatus[puzzleDisplayOrder[currentPuzzle]] = WakeStates::Timedout;
      wakeTimeout = true;
      drawWakeStatus(currentPuzzle);

      // Move on to Next puzzle
      currentPuzzle++;
      commandSent = false;
    }
  }

  if (currentPuzzle >= PUZZLE_DISPLAY_ORDER_LENGTH) {
    if (wakeTimeout) {
      // pause waiting for the screen to be touched
      while(!ts.touched()) {}
    }
    transitionToReadyState();
  }

}

void drawPuzzleDifficulty() {

  for (int difficulty=0; difficulty<DIFFCULTY_DISPLAY_LENGTH; difficulty++) {
    int row = 0;
    int col = difficulty % DIFFCULTY_DISPLAY_LENGTH;
    int startX = col*(DIFFICULTY_WIDTH+DIFFICULTY_PADDING) + DIFFICULTY_PADDING;
    int startY = row*(DIFFICULTY_HEIGHT+DIFFICULTY_PADDING) + DIFFICULTY_PADDING;
    int width = DIFFICULTY_WIDTH;
    int height = DIFFICULTY_HEIGHT;

    // Draw Outline
    tft.fillRoundRect(startX, startY, width, height, DIFFICULTY_RADIUS, difficlutyDisplayColors[difficulty]);


    // Draw Labels
    startX += DIFFICULTY_PADDING;
    startY += height/2;
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(startX, startY);
    tft.print(difficultyDisplayNames[difficulty]);
  }

}


// Setup for performing Ready State activities
void transitionToReadyState() {
  setControllerState(ControllerStates::Ready);

  tft.fillScreen(ILI9341_BLACK);
  drawPuzzleDifficulty();

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Please select a"));
  lcd.setCursor(0,1);
  lcd.print(F("Difficulty Level"));
  lcd.setCursor(0,2);
  lcd.print(F("to start the game"));
  lcd.setCursor(0,3);
  lcd.print(F("..."));

}

void performReady() {
  boolean istouched = ts.touched();
  if (istouched && !wasTouched) {
    TS_Point p = ts.getPoint();
    //Serial.print("Pressure = ");
    //Serial.print(p.z);
    //Serial.print(", x = ");
    //Serial.print(p.x);
    //Serial.print(", y = ");
    //Serial.print(p.y);
    //delay(30);
    //Serial.println();
    int yVal = map(p.x, TS_MINY, TS_MAXY, 0, TFT_HEIGHT);
    int xVal = map(p.y, TS_MINX, TS_MAXX, 0, TFT_WIDTH);
    //Serial.print(F("TFT X: "));
    //Serial.print(xVal);
    //Serial.print(F(" Y: "));
    //Serial.print(yVal);
    //Serial.println();
    int difficulty = xVal/(DIFFICULTY_WIDTH+DIFFICULTY_PADDING);
    puzzleDifficulty = difficultyDisplayNames[difficulty][0];
    Serial.print(F("Difficulty = "));
    Serial.print(puzzleDifficulty);
    Serial.print(F("("));
    Serial.print(difficulty);
    Serial.println(F(")"));
    transitionToPlayingState();
  }
  wasTouched = istouched;
}


void transitionToPlayingState() {
  setControllerState(ControllerStates::Playing);

  tft.fillScreen(ILI9341_BLACK);

  currentPuzzle = 0;
  commandSent = false;
}


void performPlaying() {

  if (puzzleWakeStatus[puzzlePlayOrder[currentPuzzle]] != WakeStates::Awake) {
    Serial.println(F("Skipping Puzzle - not awake"));
    currentPuzzle++;
  }

  else if (!commandSent) {

    sendStart(puzzlePlayOrder[currentPuzzle], puzzleDifficulty, puzzlePlayValues[currentPuzzle].c_str(), puzzlePlayValues[currentPuzzle].length());
    commandSent = true;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(puzzlePlayNames[currentPuzzle]);

  }
  else {
      if (commandReady) {
        switch (command) {
          case COMMAND_INIT:
            //TODO: Display intialization instructions
            lcd.setCursor(0,1);
            lcd.print("Initialize");
            clearCommand();
            break;
          case COMMAND_PLAY:
            //TODO: Display puzzle play instructions
            lcd.setCursor(0,1);
            lcd.print("Playing   ");
            clearCommand();
            break;
          case COMMAND_DONE:
            lcd.setCursor(0,1);
            lcd.print("Done      ");
            currentPuzzle++;
            commandSent = false;
            clearCommand();
            break;
        }

      }

  }

  if (currentPuzzle>=PUZZLE_PLAY_LENGTH) {
    transitionToSolvedState();
  }

}

// Game is over
void transitionToSolvedState() {
  // Play the entertainer
  setControllerState(ControllerStates::Solved);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("YOU WON!!!"));
  playMusic(SPEAKER_PIN, entertainer);
  //lcd.setCursor(0,1);
  //lcd.print(F("Difficulty Level"));
  //lcd.setCursor(0,2);
  //lcd.print(F("to start the game"));
  //lcd.setCursor(0,3);
  //lcd.print(F("..."));

}

void setup()
{
  Serial.begin(9600);
  Serial.println(__FILE__);
  Serial.println(F("Compiled: " __DATE__ ));
  Serial.print(F("Controller   ID: "));
  Serial.println(ControllerId);
  Serial.print(F("Controller Name: "));
  Serial.println(ControllerName);

  // Speaker
  pinMode(SPEAKER_PIN, OUTPUT);


	// initialize the LCD
  Serial.print(F("Starting LCD ... "));
  lcd.init();
  lcd.clear();
  lcd.home();
  //lcd.autoscroll();
	lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(F("Phone Home Busy Box"));
  //lcd.setCursor(0,1);
  //lcd.print(F("Controller Module"));
  //lcd.setCursor(0,2);
  //lcd.print(F("  ID: "));
  //lcd.print(ControllerId);
  //lcd.setCursor(0,3);
  //lcd.print(F("Name: "));
  //lcd.print(ControllerName);
  Serial.println(F(" Started"));

  Serial.print(F("Starting TFT ... "));
  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  // origin = left,top landscape (USB left upper)
  tft.setRotation(1); 
  //tft.clear();
  //tft.home();
  //Serial.println(F(" Started"));
  //Serial.print(F("Width: "));
  //Serial.print(tft.width());
  //Serial.print(F(" Height: "));
  //Serial.println(tft.height());

  tft.println(F("Phone Home Busy Box"));
  //tft.println(F("Controller Module"));
  //tft.print(F("  ID: "));
  //tft.println(ControllerId);
  //tft.print(F("Name: "));
  //tft.println(ControllerName);
  //tft.print(F("Width: "));
  //tft.print(tft.width());
  //tft.print(F(" Height: "));
  //tft.println(tft.height());

  Serial.print(F("Starting Touchscreen ... "));
  if (!ts.begin()) { 
    Serial.println(F("Unable to start"));
  } 
  else { 
    Serial.println(F(" Started")); 
  }
  ts.setRotation(2);

  setupPJON(ControllerId);
  
  Serial.println(F("Setup Complete\n"));
  tft.println(F("Setup Complete"));
  delay(1000);

   // Initialize the controller state
  transitionToWakeState();
}

void loop()
{
  loopPJON();

  switch (controllerState) {
    case ControllerStates::Starting:
      performWake();
      break;
    case ControllerStates::Ready:
      performReady();
      break;
    case ControllerStates::Intialize:
      // This state is not used by the controller
      // It is only here for completeness
      break;
    case ControllerStates::Playing:
      performPlaying();
      break;
    case ControllerStates::Solved:
      if (commandReady && command == COMMAND_WAKE) {
        transitionToWakeState();
      }
      break;
    default:
      Serial.print(F("Invalid State: "));
      Serial.println(controllerState);
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
    Serial.println(puzzleStatesString[controllerState]);
    clearCommand();
  }

}