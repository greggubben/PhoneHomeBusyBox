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
#include "Instructions.h"

// These definitions make the code more readable from the controller perspective
// allowing for library reuse and clarity of controller vs puzzle
#define ControllerStates        PuzzleStates
#define controllerState         puzzleState
#define setControllerState      setPuzzleState
#define controllerStatesString  puzzleStatesString
#define setupControllerStatus   setupPuzzleStatus

// Puzzle definitions
const int   ControllerId  = CONTROL_ID;
//const char* ControllerName = "Controller";
//bool controlInitialized = false;
char puzzleDifficulty = DIFFICULTY_EASY;
#define MAX_MINUTES 30

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
#define TFT_CS    9    // Connected to LOLIN pin D3
#define TFT_DC    10   // Connected to LOLIN pin D8
//#define TFT_MOSO      // Connected to LOLIN pin D6
//#define TFT_MOSI     // Connected to LOLIN pin D7
//#define TFT_SCK      // Connected to LOLIN pin D5
// initialize ILI9341 TFT library
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
#define TFT_WIDTH 320
#define TFT_HEIGHT 240

// Display order
const static int puzzleDisplayOrder[] = {SLIDER_ID, CONTROL_ID, FLIPBITS_ID, WIRES_ID, PHONE_ID, SPINDIGIT_ID};
//const String puzzleDisplayNames[] = {"Slider", "Control", "Flip", "Wires", "Phone", "Spin"};
#define PUZZLE_DISPLAY_ORDER_LENGTH 6



int difficulty = 0;
const char* difficultyDisplayNames[] = {"Easy", "Medium", "Hard"};
const uint16_t difficlutyDisplayColors[] = {ILI9341_GREEN, ILI9341_YELLOW, ILI9341_RED};
#define DIFFCULTY_DISPLAY_LENGTH 3

bool wasTouched = false;

typedef enum WakeStates {Unknown, Asked, Awake, Timedout} WakeStates;
const uint16_t wakeStateColors[] = {ILI9341_WHITE, ILI9341_YELLOW, ILI9341_GREEN, ILI9341_RED};

WakeStates puzzleWakeStatus[ID_ARRAY_LENGTH];

// Game Order
const static int puzzlePlayOrder[] = {SLIDER_ID, WIRES_ID, FLIPBITS_ID, SPINDIGIT_ID, PHONE_ID};
//const String puzzlePlayNames[] = {"Slider", "Wires", "Flip", "Spin", "Phone"};
const String puzzlePlayValues[] = {"7", "9", "50", "163", "7950163"};  // Hack - upgrade to parsing logic
#define PUZZLE_PLAY_LENGTH 5

const char* targetNumber = "7950163";
int currentPuzzle = 0;
int currentPuzzleID = 0;
int elapsedMinutes = 0;
int elapsedSeconds = 0;
unsigned long lastMillis = 0;

//PuzzleDefinition *currentPuzzleDefinition;
bool commandSent = false;
unsigned long commandSentAt = 0;
bool wakeTimeout = false;
#define TIMEOUT_MILLIS 5000

#define BACKGROUND_COLOR ILI9341_BLACK
#define PADDING 4

// Puzzle display grid constants
#define PUZZLE_TEXT_COLOR ILI9341_BLACK
#define PUZZLE_WIDTH   100
#define PUZZLE_HEIGHT  100
#define PUZZLE_RADIUS  10
#define PUZZLE_PADDING 5
#define PUZZLE_TEXT_SIZE 2
#define PUZZLE_TEXT_CHAR_HEIGHT 16
#define PUZZLE_TEXT_CHAR_WIDTH 12
#define PUZZLE_OFFSET_X 5
#define PUZZLE_OFFSET_Y 20

// Difficulty display grid constants
#define DIFFICULTY_TEXT_COLOR ILI9341_BLACK
#define DIFFICULTY_WIDTH   100
#define DIFFICULTY_HEIGHT  200
#define DIFFICULTY_RADIUS  20
#define DIFFICULTY_PADDING 5
#define DIFFICULTY_TEXT_SIZE 2
#define DIFFICULTY_TEXT_CHAR_HEIGHT 16
#define DIFFICULTY_TEXT_CHAR_WIDTH 12
#define DIFFICULTY_TEXT_Y (TFT_HEIGHT - WAKE_TEXT_CHAR_HEIGHT)/2
#define DIFFICULTY_OFFSET_X 5
#define DIFFICULTY_OFFSET_Y 20

#define TEXT_SIZE 3
#define TEXT_CHAR_HEIGHT 24
#define TEXT_CHAR_WIDTH 18

#define LINE_COLOR ILI9341_MAGENTA
#define LINE_HEIGHT 4

#define NAME_COLOR ILI9341_CYAN
#define NAME_START_X 0
#define NAME_START_Y PADDING

#define DIFF_START_X 0
#define DIFF_START_Y NAME_START_Y + TEXT_CHAR_HEIGHT + PADDING

#define TOP_LINE_Y DIFF_START_Y + TEXT_CHAR_HEIGHT + PADDING
#define TOP_LINE_END_Y TOP_LINE_Y + LINE_HEIGHT

#define TIME_COLOR ILI9341_WHITE
#define TIME_TEXT_SIZE 9
#define TIME_CHAR_HEIGHT 72
#define TIME_CHAR_WIDTH 54
#define TIME_MIDDLE_Y (TOP_LINE_END_Y + BOT_LINE_Y)/2
#define TIME_Y TIME_MIDDLE_Y - (TIME_CHAR_HEIGHT/2)
#define TIME_COLON_X (TFT_WIDTH - TIME_CHAR_WIDTH)/2
#define TIME_MINUTES_X TFT_WIDTH/2 - (TIME_CHAR_WIDTH*2.5)
#define TIME_SECONDS_X TFT_WIDTH/2 + (TIME_CHAR_WIDTH/2)

#define STATUS_COLOR ILI9341_ORANGE
#define STATUS_START_X 0
#define STATUS_START_Y TFT_HEIGHT - TEXT_CHAR_HEIGHT - PADDING

#define BOT_LINE_Y STATUS_START_Y - PADDING - LINE_HEIGHT

const char* const fullNames[] = {Controller_Name, Controller_Name, Flip_Name, Slider_Name, Wires_Name, Spin_Name, Phone_Name};
const char* const *initInstructions[] = {Select_Difficulty, Select_Difficulty, Flip_Init, Slider_Init, Wires_Init, Spin_Init, Phone_Init};
const char* const *playInstructions[] = {Select_Difficulty, Select_Difficulty, Flip_Play, Slider_Play, Wires_Play, Spin_Play, Phone_Play};


void printLCD(const char *const lines[]) {
  char lineBuffer[21];
  lcd.clear();
  for (int lineNum=0; lineNum<4; lineNum++) {
    strcpy_P(lineBuffer, (char *)pgm_read_word(&(lines[lineNum])));
    lcd.setCursor(0, lineNum);
    lcd.print(lineBuffer);
  }
}


void setupPlayFTF() {
  tft.fillScreen(BACKGROUND_COLOR);

  uint16_t textWidth = strlen(difficultyDisplayNames[difficulty]) * TEXT_CHAR_WIDTH;
  tft.setTextSize(TEXT_SIZE);
  tft.setCursor((TFT_WIDTH-textWidth)/2, DIFF_START_Y);
  tft.setTextColor(difficlutyDisplayColors[difficulty]);
  tft.print(difficultyDisplayNames[difficulty]);

  tft.fillRect(NAME_START_X, TOP_LINE_Y, TFT_WIDTH, LINE_HEIGHT, LINE_COLOR);
  //tft.drawFastHLine(0, STATUS_LINE_Y-5, TFT_WIDTH, LINE_COLOR);
  tft.fillRect(STATUS_START_X, BOT_LINE_Y, TFT_WIDTH, LINE_HEIGHT, LINE_COLOR);

  tft.setTextSize(TIME_TEXT_SIZE);
  tft.setTextColor(TIME_COLOR);
  //uint16_t textX, textY, textWidth, textHeight;
  //tft.getTextBounds("00", 0, 20, &textX, &textY, &textWidth, &textHeight);
  //Serial.print("00  textWidth=");
  //Serial.println(textWidth);
  //Serial.print("00 textHeight=");
  //Serial.println(textHeight);
  //tft.getTextBounds("00:00", 0, 20, &textX, &textY, &textWidth, &textHeight);
  //Serial.print("00:00  textWidth=");
  //Serial.println(textWidth);
  //Serial.print("00:00 textHeight=");
  //Serial.println(textHeight);
  tft.setCursor(TIME_COLON_X, TIME_Y);
  tft.print(":");

}


void printPuzzleNameTFT(const char*  nameText) {
  tft.fillRect(NAME_START_X, NAME_START_Y, TFT_WIDTH, TEXT_CHAR_HEIGHT, BACKGROUND_COLOR);
  tft.setTextSize(TEXT_SIZE);
  tft.setTextColor(NAME_COLOR);
  uint16_t textWidth = strlen_P(nameText) * TEXT_CHAR_WIDTH;
  tft.setCursor((TFT_WIDTH-textWidth)/2, NAME_START_Y);
  tft.print((__FlashStringHelper *) nameText);
}

void printPuzzleStatusTFT(const char*  statusText) {
  tft.fillRect(STATUS_START_X, STATUS_START_Y, TFT_WIDTH, TEXT_CHAR_HEIGHT, BACKGROUND_COLOR);
  tft.setTextSize(TEXT_SIZE);
  tft.setTextColor(STATUS_COLOR);
  uint16_t textWidth = strlen_P(statusText) * TEXT_CHAR_WIDTH;
  tft.setCursor((TFT_WIDTH-textWidth)/2, STATUS_START_Y);
  tft.print((__FlashStringHelper *) statusText);
}


void printMinutesTFT() {
  tft.fillRect(TIME_MINUTES_X, TIME_Y, TIME_CHAR_WIDTH*2, TIME_CHAR_HEIGHT, BACKGROUND_COLOR);
  tft.setTextSize(TIME_TEXT_SIZE);
  tft.setTextColor(TIME_COLOR);
  tft.setCursor(TIME_MINUTES_X, TIME_Y);
  if (elapsedMinutes < 10) {
    tft.print("0");
  }
  tft.print(elapsedMinutes);
}


void printSecondsTFT() {
  tft.fillRect(TIME_SECONDS_X, TIME_Y, TIME_CHAR_WIDTH*2, TIME_CHAR_HEIGHT, BACKGROUND_COLOR);
  tft.setTextSize(TIME_TEXT_SIZE);
  tft.setTextColor(TIME_COLOR);
  tft.setCursor(TIME_SECONDS_X, TIME_Y);
  if (elapsedSeconds < 10) {
    tft.print("0");
  }
  tft.print(elapsedSeconds);
}


bool addSecond() {
  elapsedSeconds++;
  if (elapsedSeconds >= 60) {
    elapsedSeconds = 0;
    elapsedMinutes++;
    printMinutesTFT();
  }
  printSecondsTFT();
  return (elapsedMinutes >= MAX_MINUTES);
}

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
    int startX = col*(PUZZLE_WIDTH+PUZZLE_PADDING) + PUZZLE_OFFSET_X;
    int startY = row*(PUZZLE_HEIGHT+PUZZLE_PADDING) + PUZZLE_OFFSET_Y;
//    int width = PUZZLE_WIDTH;
//    int height = PUZZLE_HEIGHT;

    tft.fillRoundRect(startX, startY, PUZZLE_WIDTH, PUZZLE_HEIGHT, PUZZLE_RADIUS, wakeStateColors[puzzleWakeStatus[thisPuzzleID]]);

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
    startX += (PUZZLE_WIDTH - (strlen(moduleShortNames[thisPuzzleID]) * PUZZLE_TEXT_CHAR_WIDTH))/2;
    startY += (PUZZLE_HEIGHT-PUZZLE_TEXT_CHAR_HEIGHT)/2;
    tft.setTextSize(PUZZLE_TEXT_SIZE);
    tft.setTextColor(PUZZLE_TEXT_COLOR);
    tft.setCursor(startX, startY);
    tft.print(moduleShortNames[thisPuzzleID]);

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

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Phone Home Busy Box"));

  tft.fillScreen(BACKGROUND_COLOR);
  drawWakeStatus();

}

// Wake up all the Puzzles
void performWake() {
  if (puzzleDisplayOrder[currentPuzzle] == CONTROL_ID) {
    Serial.println(F("Skipping Controller"));
    currentPuzzle++;
  }

  if (!commandSent) {
    //Serial.print(F("Waking "));
    //Serial.println(puzzleDisplayOrder[currentPuzzle]);
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
      lcd.setCursor(0,2);
      lcd.print(F("Tap Screen"));
      lcd.setCursor(0,3);
      lcd.print(F("to Continue"));
      while(!ts.touched()) {}
    }
    transitionToReadyState();
  }

}

void drawPuzzleDifficulty() {

  for (int difficulty=0; difficulty<DIFFCULTY_DISPLAY_LENGTH; difficulty++) {
    int row = 0;
    int col = difficulty % DIFFCULTY_DISPLAY_LENGTH;
    int startX = col*(DIFFICULTY_WIDTH+DIFFICULTY_PADDING) + DIFFICULTY_OFFSET_X;
    int startY = row*(DIFFICULTY_HEIGHT+DIFFICULTY_PADDING) + DIFFICULTY_OFFSET_Y;
//    int width = DIFFICULTY_WIDTH;
//    int height = DIFFICULTY_HEIGHT;

    // Draw Outline
    tft.fillRoundRect(startX, startY, DIFFICULTY_WIDTH, DIFFICULTY_HEIGHT, DIFFICULTY_RADIUS, difficlutyDisplayColors[difficulty]);


    // Draw Labels
    startX += (DIFFICULTY_WIDTH - (strlen(difficultyDisplayNames[difficulty]) * DIFFICULTY_TEXT_CHAR_WIDTH))/2;
    startY = (TFT_HEIGHT - DIFFICULTY_TEXT_CHAR_HEIGHT)/2;
    tft.setTextSize(DIFFICULTY_TEXT_SIZE);
    tft.setTextColor(DIFFICULTY_TEXT_COLOR);
    tft.setCursor(startX, startY);
    tft.print(difficultyDisplayNames[difficulty]);
  }

}


// Setup for performing Ready State activities
void transitionToReadyState() {
  setControllerState(ControllerStates::Ready);

  tft.fillScreen(BACKGROUND_COLOR);
  drawPuzzleDifficulty();

  printLCD(Select_Difficulty);   // Ask to select Difficulty
  //printLCD(Controller_Definition.init);   // Ask to select Difficulty
  //lcd.clear();
  //lcd.setCursor(0,0);
  //lcd.print(F("Please select a"));
  //lcd.setCursor(0,1);
  //lcd.print(F("Difficulty Level"));
  //lcd.setCursor(0,2);
  //lcd.print(F("to start the game"));
  //lcd.setCursor(0,3);
  //lcd.print(F("..."));

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
    difficulty = xVal/(DIFFICULTY_WIDTH+DIFFICULTY_PADDING);
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

  //tft.fillScreen(BACKGROUND_COLOR);
  setupPlayFTF();

  currentPuzzle = 0;
  currentPuzzleID = puzzlePlayOrder[currentPuzzle];
  //currentPuzzleDefinition = Puzzle_Definitions[currentPuzzleID];
  commandSent = false;

  elapsedMinutes = 0;
  elapsedSeconds = 0;
  lastMillis = millis();
  printMinutesTFT();
  printSecondsTFT();
}


void performPlaying() {

  if (puzzleWakeStatus[currentPuzzleID] != WakeStates::Awake) {
    Serial.println(F("Skipping Puzzle - not awake"));
    currentPuzzle++;
    currentPuzzleID = puzzlePlayOrder[currentPuzzle];
    //currentPuzzleDefinition = Puzzle_Definitions[currentPuzzleID];
  }

  else if (!commandSent) {
    printPuzzleNameTFT(fullNames[currentPuzzleID]);
    Serial.println((__FlashStringHelper *) fullNames[currentPuzzleID]);

    sendStart(currentPuzzleID, puzzleDifficulty, puzzlePlayValues[currentPuzzle].c_str(), puzzlePlayValues[currentPuzzle].length());
    commandSent = true;
    //lcd.clear();
    //lcd.setCursor(0,0);
    //lcd.print(moduleShortNames[currentPuzzleID]);

  }
  else {
      if (commandReady) {
        switch (command) {
          case COMMAND_INIT:
            //TODO: Display intialization instructions
            //printLCD((const char* const*)&(currentPuzzleDefinition->init));
            printLCD(initInstructions[currentPuzzleID]);
//            printTftPuzzleStatus(&(puzzleStatus[0]));
            printPuzzleStatusTFT(To_Init);
            Serial.println((__FlashStringHelper *) To_Init);
            //lcd.setCursor(0,1);
            //lcd.print("Initialize");
            clearCommand();
            break;
          case COMMAND_PLAY:
            //TODO: Display puzzle play instructions
            //printLCD(Puzzle_Definitions[currentPuzzle].init);
            printLCD(playInstructions[currentPuzzleID]);
//            printTftPuzzleStatus(&(puzzleStatus[1]));
            printPuzzleStatusTFT(To_Play);
            //lcd.setCursor(0,1);
            //lcd.print("Playing   ");
            clearCommand();
            break;
          case COMMAND_DONE:
            //lcd.setCursor(0,1);
            //lcd.print("Done      ");
            currentPuzzle++;
            currentPuzzleID = puzzlePlayOrder[currentPuzzle];
            //currentPuzzleDefinition = Puzzle_Definitions[currentPuzzleID];
            commandSent = false;
            clearCommand();
            break;
        }

      }

  }

  unsigned long currMillis = millis();
  if (currMillis-lastMillis > 1000) {
    if (addSecond()) {
      // Reach the maximum minutes allowed
      outOfTime();
    }
    lastMillis = currMillis;
  }

  if (currentPuzzle>=PUZZLE_PLAY_LENGTH) {
    transitionToSolvedState();
  }

}

// Game is over
void transitionToSolvedState() {
  // Play the entertainer
  setControllerState(ControllerStates::Solved);

  printLCD(Winner);   // Ask to select Difficulty

  playMusic(SPEAKER_PIN, entertainer);

}


// Game is over
void outOfTime() {
  // Play the entertainer
  setControllerState(ControllerStates::Solved);

  printLCD(OutOfTime);   // Ask to select Difficulty
}

void setup()
{
  Serial.begin(9600);
  //Serial.println(__FILE__);
  //Serial.println(F("Compiled: " __DATE__ ));
  Serial.print(F("Controller   ID: "));
  Serial.println(ControllerId);
  Serial.print(F("Controller Name: "));
  Serial.println((__FlashStringHelper *) Controller_Name);

  // Speaker
  pinMode(SPEAKER_PIN, OUTPUT);


	// initialize the LCD
  Serial.print(F("Starting LCD ... "));
  lcd.init();
  lcd.clear();
  lcd.home();
  //lcd.autoscroll();
	lcd.backlight();
  //lcd.setCursor(0,0);
  //lcd.print(F("Phone Home Busy Box"));
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
  tft.fillScreen(BACKGROUND_COLOR);
  // origin = left,top landscape (USB left upper)
  tft.setRotation(1); 
  //tft.clear();
  //tft.home();
  Serial.println(F(" Started"));
  //Serial.print(F("Width: "));
  //Serial.print(tft.width());
  //Serial.print(F(" Height: "));
  //Serial.println(tft.height());

  //tft.println(F("Phone Home Busy Box"));
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
  //tft.println(F("Setup Complete"));
  
  // Wait for other modules to finish their startup
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
      if (ts.touched()) {
        transitionToWakeState();
      }
      break;
    default:
      Serial.print(F("Invalid State: "));
      Serial.println(controllerState);
      break;
  }
  if (commandReady) {
    if (command == COMMAND_WAKE) {
      // Wake could happen at any time - good for debugging
      transitionToWakeState();
    }
    else {
      // Unknown command or command not valid in current state
      Serial.print(F("Command '"));
      Serial.print(command);
      Serial.print(F("' "));
      if (commandArgument.length() > 0) {
        Serial.print(F("with args "));
        Serial.print(commandArgument);
      }
      Serial.print(F(" was not processed in state: "));
      Serial.println(puzzleStatesString[controllerState]);
    }
    clearCommand();
  }

}