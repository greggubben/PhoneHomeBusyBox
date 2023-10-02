/*!
 * Phone Home Briefcase Game common definitions
 *
 * Common file for definitions to be used by all puzzle modules in the game.
 */

#ifndef PHONEHOME_PUZZLE_STATE_H
#define PHONEHOME_PUZZLE_STATE_H

#include <Arduino.h>
#include "PhoneHome_Definitions.h"

#ifndef PHONEHOME_CONTROLLER
#include <Adafruit_NeoPixel.h>

// Number of status Pixels
#define STATUS_PIXELS 1

// Status pixel definition
Adafruit_NeoPixel statusPixel(STATUS_PIXELS, STATUS_LED_PIN, NEO_RGB + NEO_KHZ800);

#endif


// Define the States for the Puzzle
//
// Power on                                           --> Startup
// Startup    -- (when connected to control)          --> Ready
// Ready      -- (when told to start puzzle)          --> Initialize
// Initialize -- (when puzzle is in start config)     --> Playing
// Playing    -- (when the puzzle has been completed) --> Solved
// Tuning     -- (when in startup and told to tune)   --> Tuning
typedef enum PuzzleStates {Starting, Ready, Intialize, Playing, Solved, Tuning} PuzzleStates;
// Character versions of Puzzle State for Debug Printing
const String puzzleStatesString[] = {"Starting", "Ready", "Intialize", "Playing", "Solved", "Tuning"};

#ifndef PHONEHOME_CONTROLLER

// NeoPixel colors representing each state
const uint32_t puzzleColor[] ={ Adafruit_NeoPixel::Color(255,0,255),   // Startup    = Purple
                                Adafruit_NeoPixel::Color(0,0,255),     // Ready      = Blue
                                Adafruit_NeoPixel::Color(255,0,0),     // Initialize = Red
                                Adafruit_NeoPixel::Color(255,255,0),   // Playing    = Yellow
                                Adafruit_NeoPixel::Color(0,255,0),     // Solved     = Green
                                Adafruit_NeoPixel::Color(0,255,255)};  // Tuning     = Cyan

#endif

// Current state of the Puzzle
PuzzleStates puzzleState = Starting;

// Set a new puzzle state and change indicators accordingly
void setPuzzleState(PuzzleStates newPuzzleState) {
  puzzleState = newPuzzleState;
#ifndef PHONEHOME_CONTROLLER
  statusPixel.setPixelColor(0, statusPixel.gamma32(puzzleColor[puzzleState]));
  statusPixel.show();
#endif
  Serial.print(F("New Status = "));
  Serial.println(puzzleStatesString[puzzleState]);
}

// Setuo the status pixel and set the initial puzzle state
void setupPuzzleStatus() {
#ifndef PHONEHOME_CONTROLLER
  statusPixel.begin();
#endif
  setPuzzleState(PuzzleStates::Starting);

  // Setup randomization
  randomSeed(analogRead(RANDOM_SEED_PIN));

}





#endif // PHONEHOME_PUZZLE_STATE_H
