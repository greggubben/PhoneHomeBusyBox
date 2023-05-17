/*!
 * Phone Home Briefcase Game common definitions
 *
 * Common file for definitions to be used by all puzzle modules in the game.
 */

#ifndef PHONEHOME_PUZZLE_STATE_H
#define PHONEHOME_PUZZLE_STATE_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "PhoneHome_Definitions.h"



// Number of status Pixels
#define STATUS_PIXELS 1

// Status pixel definition
Adafruit_NeoPixel statusPixel(STATUS_PIXELS, STATUS_LED_PIN, NEO_RGB + NEO_KHZ800);


// Define the States for the Puzzle
//
// Power on                                           --> Startup
// Startup    -- (when connected to control)          --> Ready
// Ready      -- (when told to start puzzle)          --> Initialize
// Initialize -- (when puzzle is in start config)     --> Playing
// Playing    -- (when the puzzle has been completed) --> Solved
typedef enum PuzzleStates {Starting, Ready, Intialize, Playing, Solved} PuzzleStates;
// Character versions of Puzzle State for Debug Printing
const String puzzleStatesString[] = {"Starting", "Ready", "Intialize", "Playing", "Solved"};
// NeoPixel colors representing each state
const uint32_t puzzleColor[] ={ Adafruit_NeoPixel::Color(255,0,255),   // Startup    = Purple
                                Adafruit_NeoPixel::Color(0,0,255),     // Ready      = Blue
                                Adafruit_NeoPixel::Color(255,0,0),     // Initialize = Red
                                Adafruit_NeoPixel::Color(255,255,0),   // Playing    = Yellow
                                Adafruit_NeoPixel::Color(0,255,0)};    // Solved     = Green
// Current state of the Puzzle
PuzzleStates puzzleState = Starting;

// Set a new puzzle state and change indicators accordingly
void setPuzzleState(PuzzleStates newPuzzleState) {
  puzzleState = newPuzzleState;
  statusPixel.setPixelColor(0, statusPixel.gamma32(puzzleColor[puzzleState]));
  statusPixel.show();
  Serial.print(F("New Status = "));
  Serial.println(puzzleStatesString[puzzleState]);
}

// Setuo the status pixel and set the initial puzzle state
void setupPuzzleStatus() {
  statusPixel.begin();
  setPuzzleState(PuzzleStates::Starting);

  // Setup randomization
  randomSeed(analogRead(RANDOM_SEED_PIN));

}





#endif // PHONEHOME_PUZZLE_STATE_H
