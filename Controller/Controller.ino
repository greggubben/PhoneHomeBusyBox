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
const uint8_t   ControllerId  = CONTROL_ID;
const char* ControllerShortName = "Control";
const char* ControllerName = "Controller";
//bool controlInitialized = false;
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
const static uint8_t puzzleDisplayOrder[] = {SLIDER_ID, CONTROL_ID, FLIPBITS_ID, WIRES_ID, PHONE_ID, SPINDIGIT_ID};
//const String puzzleDisplayNames[] = {"Slider", "Control", "Flip", "Wires", "Phone", "Spin"};
#define PUZZLE_DISPLAY_ORDER_LENGTH 6



char puzzleDifficulty = DIFFICULTY_EASY;
uint8_t difficulty = 0;
const char* difficultyDisplayNames[] = {"Easy", "Medium", "Hard"};
const uint16_t difficlutyDisplayColors[] = {ILI9341_GREEN, ILI9341_YELLOW, ILI9341_RED};
#define DIFFCULTY_DISPLAY_LENGTH 3

bool wasTouched = false;

typedef enum WakeStates {Unknown, Asked, Awake, Timedout} WakeStates;
const uint16_t wakeStateColors[] = {ILI9341_WHITE, ILI9341_YELLOW, ILI9341_GREEN, ILI9341_RED};

WakeStates puzzleWakeStatus[ID_ARRAY_LENGTH];

// Game Order
#define PUZZLE_PLAY_LENGTH 5
#define TARGET_NUMBER_LENGTH 7
#define MAX_TARGET_NUMBERS 3
const  uint8_t defaultPlayOrder[PUZZLE_PLAY_LENGTH] = {SLIDER_ID, WIRES_ID, FLIPBITS_ID, SPINDIGIT_ID, PHONE_ID};
uint8_t puzzlePlayOrder[PUZZLE_PLAY_LENGTH];
const char* targetPuzzleNumbers[MAX_TARGET_NUMBERS] = {"7950163", "7955769", "8675309"};
uint8_t targetNumberPos = 0;
const uint8_t difficultyPuzzleChoice[DIFFCULTY_DISPLAY_LENGTH] = {1, 2, MAX_TARGET_NUMBERS};
char randomPuzzleNumber[TARGET_NUMBER_LENGTH+1];
uint8_t nextNumberPosition = 0;
byte puzzleNumberSize[ID_ARRAY_LENGTH] = {0, 0, 0, 0, 0, 0, 0};   // Based on ID numbers

//const char* targetNumber = "7950163";
uint8_t currentPuzzle = 0;
uint8_t currentPuzzleID = 0;
//char currentPuzzleShortName[10] = "";
//char currentPuzzleLongName[21] = "";
uint8_t elapsedMinutes = 0;
uint8_t elapsedSeconds = 0;
unsigned long lastMillis = 0;
uint8_t lcdDisplayLine = 0;
bool puzzleDone = false;
bool doneInstructions = false;
bool showNextArrow = false;

//PuzzleDefinition *currentPuzzleDefinition;
bool commandSent = false;
unsigned long commandSentAt = 0;
bool wakeTimeout = false;
#define TIMEOUT_MILLIS 5000

#define BACKGROUND_COLOR  ILI9341_BLACK
#define PADDING           4

// Puzzle display grid constants
#define PUZZLE_TEXT_COLOR       ILI9341_BLACK
#define PUZZLE_WIDTH            100
#define PUZZLE_HEIGHT           100
#define PUZZLE_RADIUS           10
#define PUZZLE_PADDING          5
#define PUZZLE_TEXT_SIZE        2
#define PUZZLE_TEXT_CHAR_HEIGHT 16
#define PUZZLE_TEXT_CHAR_WIDTH  12
#define PUZZLE_OFFSET_X         5
#define PUZZLE_OFFSET_Y         20

// Difficulty display grid constants
#define DIFFICULTY_TEXT_COLOR       ILI9341_BLACK
#define DIFFICULTY_WIDTH            100
#define DIFFICULTY_HEIGHT           200
#define DIFFICULTY_RADIUS           20
#define DIFFICULTY_PADDING          5
#define DIFFICULTY_TEXT_SIZE        2
#define DIFFICULTY_TEXT_CHAR_HEIGHT 16
#define DIFFICULTY_TEXT_CHAR_WIDTH  12
#define DIFFICULTY_TEXT_Y           (TFT_HEIGHT - DIFFICULTY_TEXT_CHAR_HEIGHT)/2
#define DIFFICULTY_OFFSET_X         5
#define DIFFICULTY_OFFSET_Y         20

#define TEXT_SIZE         3
#define TEXT_CHAR_HEIGHT  24
#define TEXT_CHAR_WIDTH   18

#define LINE_COLOR  ILI9341_MAGENTA
#define LINE_HEIGHT 4

#define NAME_COLOR    ILI9341_CYAN
#define NAME_START_X  0
#define NAME_START_Y  PADDING

#define DIFF_START_X 0
#define DIFF_START_Y NAME_START_Y + TEXT_CHAR_HEIGHT + PADDING

#define TOP_LINE_Y      DIFF_START_Y + TEXT_CHAR_HEIGHT + PADDING
#define TOP_LINE_END_Y  TOP_LINE_Y + LINE_HEIGHT

#define STATUS_COLOR    ILI9341_ORANGE
#define STATUS_START_X  0
#define STATUS_START_Y  TFT_HEIGHT - TEXT_CHAR_HEIGHT - PADDING

#define BOT_LINE_Y STATUS_START_Y - PADDING - LINE_HEIGHT

#define TIME_COLOR        ILI9341_WHITE
#define TIME_TEXT_SIZE    9
#define TIME_CHAR_HEIGHT  72
#define TIME_CHAR_WIDTH   54
#define TIME_MIDDLE_Y     (TOP_LINE_END_Y + BOT_LINE_Y)/2
#define TIME_Y            TIME_MIDDLE_Y - (TIME_CHAR_HEIGHT/2)
#define TIME_COLON_X      (TFT_WIDTH - TIME_CHAR_WIDTH)/2
#define TIME_MINUTES_X    TFT_WIDTH/2 - (TIME_CHAR_WIDTH*2.5)
#define TIME_SECONDS_X    TFT_WIDTH/2 + (TIME_CHAR_WIDTH/2)

#define NEXT_COLOR            ILI9341_PINK
#define NEXT_CIRCLE_X         TFT_WIDTH/2
#define NEXT_CIRCLE_Y         TIME_MIDDLE_Y
//#define NEXT_CIRCLE_RADIUS  (BOT_LINE_Y - TOP_LINE_END_Y)/2 - (PADDING*4)
#define NEXT_CIRCLE_RADIUS    60
#define NEXT_TRIANGLE_LEFT_X  (TFT_WIDTH/2) - 35
#define NEXT_TRIANGLE_TOP_Y   TIME_MIDDLE_Y - 45
#define NEXT_TRIANGLE_BOT_Y   TIME_MIDDLE_Y + 45
#define NEXT_TRIANGLE_RIGHT_X (TFT_WIDTH/2) + NEXT_CIRCLE_RADIUS - PADDING
#define NEXT_TRIANGLE_RIGHT_Y TIME_MIDDLE_Y
#define NEXT_TEXT_COLOR       ILI9341_BLACK
#define NEXT_TEXT_SIZE        3
#define NEXT_TEXT_CHAR_HEIGHT 24
#define NEXT_TEXT_CHAR_WIDTH  18
#define NEXT_TEXT_LENGTH      4
#define NEXT_TEXT_Y           TIME_MIDDLE_Y - (NEXT_TEXT_CHAR_HEIGHT / 2)+3
#define NEXT_TEXT_X           NEXT_TRIANGLE_LEFT_X + 3



/*
 * Print a set of 4 lines from PROGMEM to the LCD display
 */
void printLCD(const char *const lines[]) {
  char lineBuffer[21];
  lcd.clear();
  for (int lineNum=0; lineNum<4; lineNum++) {
    strcpy_P(lineBuffer, (char *)pgm_read_word(&(lines[lineNum])));
    lcd.setCursor(0, lineNum);
    lcd.print(lineBuffer);
  }
}


void printPuzzleNameTFT(char*  nameText) {
  tft.fillRect(NAME_START_X, NAME_START_Y, TFT_WIDTH, TEXT_CHAR_HEIGHT, BACKGROUND_COLOR);
  tft.setTextSize(TEXT_SIZE);
  tft.setTextColor(NAME_COLOR);
  uint16_t textWidth = strlen(nameText) * TEXT_CHAR_WIDTH;
  tft.setCursor((TFT_WIDTH-textWidth)/2, NAME_START_Y);
  tft.print(nameText);
}

void printPuzzleStatusTFT(const char*  statusText) {
  tft.fillRect(STATUS_START_X, STATUS_START_Y, TFT_WIDTH, TEXT_CHAR_HEIGHT, BACKGROUND_COLOR);
  tft.setTextSize(TEXT_SIZE);
  tft.setTextColor(STATUS_COLOR);
  uint16_t textWidth = strlen(statusText) * TEXT_CHAR_WIDTH;
  tft.setCursor((TFT_WIDTH-textWidth)/2, STATUS_START_Y);
  tft.print(statusText);
}

void printColonTFT() {
  tft.setTextSize(TIME_TEXT_SIZE);
  tft.setTextColor(TIME_COLOR);
  tft.setCursor(TIME_COLON_X, TIME_Y);
  tft.print(":");
}

void printNextArrow() {
  tft.drawCircle(NEXT_CIRCLE_X, NEXT_CIRCLE_Y, NEXT_CIRCLE_RADIUS, NEXT_COLOR);
  tft.fillTriangle(NEXT_TRIANGLE_LEFT_X, NEXT_TRIANGLE_TOP_Y, NEXT_TRIANGLE_LEFT_X, NEXT_TRIANGLE_BOT_Y, NEXT_TRIANGLE_RIGHT_X, NEXT_TRIANGLE_RIGHT_Y, NEXT_COLOR);
  tft.setTextColor(NEXT_TEXT_COLOR);
  tft.setTextSize(NEXT_TEXT_SIZE);
  tft.setCursor(NEXT_TEXT_X, NEXT_TEXT_Y);
  tft.print(F("NEXT"));
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
  tft.flush();
  if (showNextArrow) {
    printNextArrow();
  }
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
  tft.flush();
  if (showNextArrow) {
    printNextArrow();
  }
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


void showTime() {
  showNextArrow = false;
  tft.fillRect(0, NEXT_CIRCLE_Y-NEXT_CIRCLE_RADIUS-1, TFT_WIDTH, (NEXT_CIRCLE_RADIUS*2)+2, BACKGROUND_COLOR);
  printColonTFT();
  printSecondsTFT();
  printMinutesTFT();
}

void showNext() {
  showNextArrow = true;
  printNextArrow();
}


void showPlayFTF() {
  tft.fillScreen(BACKGROUND_COLOR);

  uint16_t textWidth = strlen(difficultyDisplayNames[difficulty]) * TEXT_CHAR_WIDTH;
  tft.setTextSize(TEXT_SIZE);
  tft.setCursor((TFT_WIDTH-textWidth)/2, DIFF_START_Y);
  tft.setTextColor(difficlutyDisplayColors[difficulty]);
  tft.print(difficultyDisplayNames[difficulty]);

  tft.fillRect(NAME_START_X, TOP_LINE_Y, TFT_WIDTH, LINE_HEIGHT, LINE_COLOR);
  //tft.drawFastHLine(0, STATUS_LINE_Y-5, TFT_WIDTH, LINE_COLOR);
  tft.fillRect(STATUS_START_X, BOT_LINE_Y, TFT_WIDTH, LINE_HEIGHT, LINE_COLOR);

  showTime();
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
void drawWakeStatus (uint8_t puzzle, char *puzzleName) {
    uint8_t thisPuzzleID = puzzleDisplayOrder[puzzle];
    uint8_t row = puzzle / 3;
    uint8_t col = puzzle % 3;
    uint16_t startX = col*(PUZZLE_WIDTH+PUZZLE_PADDING) + PUZZLE_OFFSET_X;
    uint16_t startY = row*(PUZZLE_HEIGHT+PUZZLE_PADDING) + PUZZLE_OFFSET_Y;
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
    startX += (PUZZLE_WIDTH - (strlen(puzzleName) * PUZZLE_TEXT_CHAR_WIDTH))/2;
    startY += (PUZZLE_HEIGHT-PUZZLE_TEXT_CHAR_HEIGHT)/2;
    tft.setTextSize(PUZZLE_TEXT_SIZE);
    tft.setTextColor(PUZZLE_TEXT_COLOR);
    tft.setCursor(startX, startY);
    tft.print(puzzleName);

}

// Draw all the puzzles
void drawWakeStatus () {
  char wakingPuzzleName[] = "??";
  for (uint8_t puzzle=0; puzzle<PUZZLE_DISPLAY_ORDER_LENGTH; puzzle++) {
    wakingPuzzleName[0] = '0' + puzzleDisplayOrder[puzzle];
    drawWakeStatus(puzzle, wakingPuzzleName);
  }
}



// Setup for performing Wake State activities
void transitionToWakeState() {

  // Initialize the status pixel and set initial controller state Wake
  setupControllerStatus();

  for (uint8_t puzzle=0; puzzle<PUZZLE_DISPLAY_ORDER_LENGTH; puzzle++) {
    puzzleWakeStatus[puzzleDisplayOrder[puzzle]] = WakeStates::Unknown;
  }
  currentPuzzle = 0;
  currentPuzzleID = puzzleDisplayOrder[currentPuzzle];
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
  bool next = false;
  if (puzzleDisplayOrder[currentPuzzle] == CONTROL_ID) {
    Serial.println(F("Skipping Controller"));
    puzzleWakeStatus[currentPuzzleID] = WakeStates::Awake;
    drawWakeStatus(currentPuzzle, ControllerShortName);
    next = true;
  }

  else if (!commandSent) {
    //Serial.print(F("Waking "));
    //Serial.println(puzzleDisplayOrder[currentPuzzle]);
    sendWake(currentPuzzleID);
    puzzleWakeStatus[currentPuzzleID] = WakeStates::Asked;
    commandSent = true;
    commandSentAt = millis();
    char wakingPuzzleName[] = "??";
    wakingPuzzleName[0] = '0' + currentPuzzleID;
    drawWakeStatus(currentPuzzle, wakingPuzzleName);
  }
  else {
    if (commandReady && command == COMMAND_ACK) {
      //Serial.print(F("ACK Received from "));
      //Serial.println(commandArgument);
      puzzleWakeStatus[currentPuzzleID] = WakeStates::Awake;
      //strncpy(puzzleShortNames[puzzleDisplayOrder[currentPuzzleID]], commandArgument, 11);
      //puzzleShortNames[currentPuzzle][9] = 0;
      puzzleNumberSize[currentPuzzleID] = (commandArgument[0] == 'A') ? 'A' : commandArgument[0] - '0';
      drawWakeStatus(currentPuzzle, &commandArgument[1]);

      // Move on to Next puzzle
      next = true;
      clearCommand();
    }
    else if ((millis() - commandSentAt) > TIMEOUT_MILLIS) {
      Serial.println(F("Timeout"));
      puzzleWakeStatus[puzzleDisplayOrder[currentPuzzle]] = WakeStates::Timedout;
      wakeTimeout = true;
      char wakingPuzzleName[] = "??";
      wakingPuzzleName[0] = '0' + currentPuzzleID;
      drawWakeStatus(currentPuzzle, wakingPuzzleName);
      next = true;
    }
  }

  if (next) {
      // Move on to Next puzzle
      currentPuzzle++;
      currentPuzzleID = puzzleDisplayOrder[currentPuzzle];
      commandSent = false;
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

  for (uint8_t difficulty=0; difficulty<DIFFCULTY_DISPLAY_LENGTH; difficulty++) {
    uint8_t row = 0;
    uint8_t col = difficulty % DIFFCULTY_DISPLAY_LENGTH;
    uint16_t startX = col*(DIFFICULTY_WIDTH+DIFFICULTY_PADDING) + DIFFICULTY_OFFSET_X;
    uint16_t startY = row*(DIFFICULTY_HEIGHT+DIFFICULTY_PADDING) + DIFFICULTY_OFFSET_Y;
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

  // Setup the Target number
  targetNumberPos = rand() % difficultyPuzzleChoice[difficulty];
  nextNumberPosition = 0;
  // Copy the number over as is
  strncpy(randomPuzzleNumber, targetPuzzleNumbers[targetNumberPos], TARGET_NUMBER_LENGTH);
  randomPuzzleNumber[TARGET_NUMBER_LENGTH] = 0;

  // Copy the puzzle play order as is
  for (uint8_t p=0; p<PUZZLE_PLAY_LENGTH; p++) {
    puzzlePlayOrder[p] = defaultPlayOrder[p];
  }
  switch (puzzleDifficulty) {
    case DIFFICULTY_HARD:
      // randomize the digits
      for (uint8_t n=0; n<TARGET_NUMBER_LENGTH; n++) {
        uint8_t r = rand() % TARGET_NUMBER_LENGTH;
       char parkedChar = randomPuzzleNumber[n];
       randomPuzzleNumber[n] = randomPuzzleNumber[r];
       randomPuzzleNumber[r] = parkedChar;
      }

    case DIFFICULTY_MEDIUM:
      // randomize the play order - skipping the last (phone dialer)
      for (uint8_t p=0; p<PUZZLE_PLAY_LENGTH-1; p++) {
        uint8_t r = rand() % (PUZZLE_PLAY_LENGTH-1);
        uint8_t parkedNum = puzzlePlayOrder[p];
        puzzlePlayOrder[p] = puzzlePlayOrder[r];
        puzzlePlayOrder[r] = parkedNum;
      }

    default:
    case DIFFICULTY_EASY:
      break;
  }
  randomPuzzleNumber[TARGET_NUMBER_LENGTH] = 0;

  currentPuzzle = 0;
  currentPuzzleID = puzzlePlayOrder[currentPuzzle];
  //currentPuzzleLongName[0] = 0;

  commandSent = false;

  elapsedMinutes = 0;
  elapsedSeconds = 0;
  lastMillis = millis();
  showPlayFTF();
}


void performPlaying() {

  if (puzzleWakeStatus[currentPuzzleID] != WakeStates::Awake) {
    Serial.print(F("Skipping Puzzle "));
    Serial.print(currentPuzzleID);
    Serial.println(F(" - not awake"));
    currentPuzzle++;
    currentPuzzleID = puzzlePlayOrder[currentPuzzle];
    //currentPuzzleDefinition = Puzzle_Definitions[currentPuzzleID];
  }

  else if (!commandSent) {
    printPuzzleNameTFT("");

    char numbersToSend[TARGET_NUMBER_LENGTH+1];
    uint8_t numbersSize = puzzleNumberSize[currentPuzzleID];
    if (numbersSize == 'A') {
      numbersSize = TARGET_NUMBER_LENGTH;
      strncpy(numbersToSend, targetPuzzleNumbers[targetNumberPos], numbersSize);
    }
    else {
      for (int c=0; c<numbersSize; c++) {
        numbersToSend[c] = randomPuzzleNumber[nextNumberPosition++];
      }
    }
    numbersToSend[numbersSize] = 0;
    //Serial.print(F("numbersToSend = "));
    //Serial.println(numbersToSend);
    //Serial.print(F("numbersSize = "));
    //Serial.println(numbersSize);
    sendStart(currentPuzzleID, puzzleDifficulty, numbersToSend, numbersSize);
    commandSent = true;
    puzzleDone = false;
    //lcd.clear();
    //lcd.setCursor(0,0);
    //lcd.print(moduleShortNames[currentPuzzleID]);

  }
  else {
    if (commandReady) {
      switch (command) {
        case COMMAND_INIT:
          //strncpy(currentPuzzleLongName, commandArgument, 21);
          printPuzzleNameTFT(commandArgument);
          printPuzzleStatusTFT(To_Init);
          lcdDisplayLine = 0;
          lcd.clear();
          clearCommand();
          sendNext(currentPuzzleID);
          break;
        case COMMAND_PLAY:
          //strncpy(currentPuzzleLongName, commandArgument, 21);
          printPuzzleNameTFT(commandArgument);
          printPuzzleStatusTFT(To_Play);
          lcdDisplayLine = 0;
          lcd.clear();
          clearCommand();
          sendNext(currentPuzzleID);
          break;
        case COMMAND_DONE:
          puzzleDone = true;
          doneInstructions = (commandArgument[0] == 'Y');
          if (doneInstructions) {
            printPuzzleStatusTFT(To_Done);
            lcdDisplayLine = 0;
            lcd.clear();
            lcd.setCursor(0,3);
            lcd.print(F("Tap Screen for Next"));
            showNext();
          }
          clearCommand();
          if (doneInstructions) {
            sendNext(currentPuzzleID);
          }
          break;
        case COMMAND_LINE:
          lcd.setCursor(0, lcdDisplayLine++);
          lcd.print(commandArgument);
          clearCommand();
          sendNext(currentPuzzleID);
          break;
      }

    }
    if (puzzleDone && (!doneInstructions || ts.touched())) {
      currentPuzzle++;
      currentPuzzleID = puzzlePlayOrder[currentPuzzle];
      //currentPuzzleDefinition = Puzzle_Definitions[currentPuzzleID];
      commandSent = false;
      if (doneInstructions) {
        showTime();
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

  printPuzzleStatusTFT(To_Win);
  printLCD(Winner);

  playMusic(SPEAKER_PIN, entertainer);

}


// Game is over
void outOfTime() {
  // Play the entertainer
  setControllerState(ControllerStates::Solved);

  printPuzzleStatusTFT(To_Time);
  printLCD(OutOfTime);
}

void setup()
{
  Serial.begin(9600);
  //Serial.println(__FILE__);
  //Serial.println(F("Compiled: " __DATE__ ));
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
      if (commandArgument[0] != 0) {
        Serial.print(F("with args "));
        Serial.print(commandArgument);
      }
      Serial.print(F(" was not processed in state: "));
      Serial.println(puzzleStatesString[controllerState]);
    }
    clearCommand();
  }

}