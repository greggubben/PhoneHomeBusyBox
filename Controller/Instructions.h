/*
 * Instructure for playing the game
 */
#include <avr/pgmspace.h>

// All Definitions
//                                "                    "
const char To_Init[] = "Initialize Puzzle";
const char To_Play[] = "Play Puzzle";
const char To_Done[] = "Determine Number";
const char Blank_Line[] PROGMEM = "";


// Controller
//const char Controller_Name[] PROGMEM =   "Controller";

// Select Difficulty
//                                         "                    "
const char Select_Difficulty_1[] PROGMEM = "Please select a";
const char Select_Difficulty_2[] PROGMEM = "Difficulty Level";
const char Select_Difficulty_3[] PROGMEM = "to start the game";
//prog_char Select_Difficulty_4[] PROGMEM = "...";

const char *const Select_Difficulty[] PROGMEM =       // Instructions for Selecting Difficulty
{   
  Select_Difficulty_1,
  Select_Difficulty_2,
  Select_Difficulty_3,
  Blank_Line
};

// Winner
//                              "                    "
const char Winner_1[] PROGMEM = "Congratulations!!!";
const char Winner_2[] PROGMEM = "You WON!!!";

const char PlayAgain_1[] PROGMEM = "Tap Screen to";
const char PlayAgain_2[] PROGMEM = "Play Again";

const char *const Winner[] PROGMEM =       // Display Winner
{   
  Winner_1,
  Winner_2,
  PlayAgain_1,
  PlayAgain_2
};

// Out of Time
//                                 "                    "
const char OutOfTime_1[] PROGMEM = "You are out of time!";
const char OutOfTime_2[] PROGMEM = "Better Luck Next Try";

const char *const OutOfTime[] PROGMEM =       // Display Winner
{   
  OutOfTime_1,
  OutOfTime_2,
  PlayAgain_1,
  PlayAgain_2
};

