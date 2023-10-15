/*
 * Instructure for playing the game
 */
#include <avr/pgmspace.h>

// All Definitions
//                                "                    "
const char To_Init[] PROGMEM    = "Initialize Puzzle";
const char To_Play[] PROGMEM    = "Play Puzzle";
const char Blank_Line[] PROGMEM = "";

// Select Difficulty
//                                         "                    "
const char Select_Difficulty_1[] PROGMEM = "Please select a";
const char Select_Difficulty_2[] PROGMEM = "Difficulty Level";
const char Select_Difficulty_3[] PROGMEM = "to start the game";
//prog_char Select_Difficulty_4[] PROGMEM = "...";

const char *Select_Difficulty[] =       // Instructions for Selecting Difficulty
{   
  Select_Difficulty_1,
  Select_Difficulty_2,
  Select_Difficulty_3,
  Blank_Line
};



// Slider Definitions
const char Slider_Name[] PROGMEM = "Slide into Home";

const char Slider_Init_Play[] PROGMEM = "Move All Sliders";
const char Slider_Init_2[] PROGMEM = "Down to Bottom";

const char Slider_Play_2[] PROGMEM = "Until Number Found";

const char *Slider_Init[] =       // Instructions for Initializing Puzzle
{   
  Slider_Name,
  To_Init,
  Slider_Init_Play,
  Slider_Init_2
};

const char *Slider_Play[] =       // Instructions for Playing Puzzle
{   
  Slider_Name,
  To_Play,
  Slider_Init_Play,
  Slider_Play_2
};

// Wires Definitions
const char Wires_Name[] PROGMEM = "Hook Me Up";

const char Wires_Init_1[] PROGMEM = "Disconnect All Wires";

const char Wires_Play_1[] PROGMEM = "Turn On Lights Off";

const char *Wires_Init[] =       // Instructions for Initializing Puzzle
{   
  Wires_Name,
  To_Init,
  Wires_Init_1,
  Blank_Line
};

const char *Wires_Play[] =       // Instructions for Playing Puzzle
{   
  Wires_Name,
  To_Play,
  Slider_Init_Play,
  Slider_Play_2
};


