/*
 * Instructure for playing the game
 */
#include <avr/pgmspace.h>

// All Definitions
//                                "                    "
const char To_Init[] PROGMEM    = "Initialize Puzzle";
const char To_Play[] PROGMEM    = "Play Puzzle";
const char Blank_Line[] PROGMEM = "";


// Controller
const char Controller_Name[] PROGMEM =   "Controller";

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


// Flip Definitions
const char Flip_Name[] PROGMEM =   "Flip the Bits";

//                                 "                    "
const char Flip_Init_1[] PROGMEM = "Flip All Switches";
const char Flip_Init_2[] PROGMEM = "Down to Off";

const char *const Flip_Init[] PROGMEM =       // Instructions for Initializing Puzzle
{   
  Flip_Init_1,
  Flip_Init_2,
  Blank_Line,
  Blank_Line
};

//                                 "                    "
const char Flip_Play_1[] PROGMEM = "Flip Switches to add";
const char Flip_Play_2[] PROGMEM = "or remove value/bit";
const char Flip_Play_3[] PROGMEM = "Hi = number too High";
const char Flip_Play_4[] PROGMEM = "Lo = number too Low";

const char *const Flip_Play[] PROGMEM =       // Instructions for Playing Puzzle
{   
  Flip_Play_1,
  Flip_Play_2,
  Flip_Play_3,
  Flip_Play_4
};



// Slider Definitions
const char Slider_Name[] PROGMEM =   "Slide into Home";

//                                   "                    "
const char Slider_Init_1[] PROGMEM = "Move All Sliders";
const char Slider_Init_2[] PROGMEM = "Down to Bottom";

const char *const Slider_Init[] PROGMEM =       // Instructions for Initializing Puzzle
{   
  Slider_Init_1,
  Slider_Init_2,
  Blank_Line,
  Blank_Line
};

//                                   "                    "
const char Slider_Play_1[] PROGMEM = "Adjust Sliders until";
const char Slider_Play_2[] PROGMEM = "All LEDS are ON";
const char Slider_Play_3[] PROGMEM = "The Number will the";
const char Slider_Play_4[] PROGMEM = "Value on the Meter";

const char *const Slider_Play[] PROGMEM =       // Instructions for Playing Puzzle
{   
  Slider_Play_1,
  Slider_Play_2,
  Slider_Play_3,
  Slider_Play_4
};


// Wires Definitions
const char Wires_Name[] PROGMEM =   "Hook Me Up";

//                                  "                    "
const char Wires_Init_1[] PROGMEM = "Disconnect All Wires";

const char *const Wires_Init[] PROGMEM =       // Instructions for Initializing Puzzle
{   
  Wires_Init_1,
  Blank_Line,
  Blank_Line,
  Blank_Line
};

//                                  "                    "
const char Wires_Play_1[] PROGMEM = "Connect Wires across";
const char Wires_Play_2[] PROGMEM = "Top & Bottom rows";
const char Wires_Play_3[] PROGMEM = "Turn On Lights Off";
const char Wires_Play_4[] PROGMEM = "Turn Off Lights On";

const char *const Wires_Play[] PROGMEM =       // Instructions for Playing Puzzle
{   
  Wires_Play_1,
  Wires_Play_2,
  Wires_Play_3,
  Wires_Play_4
};


// Spin Digits Definitions
const char Spin_Name[] PROGMEM =    "Spin to Win";

//                                  "                    "
const char Spin_Init_1[] PROGMEM =  "Turn all Dials";
const char Spin_Init_2[] PROGMEM =  "to Zero";

const char *const Spin_Init[] PROGMEM =       // Instructions for Initializing Puzzle
{   
  Spin_Init_1,
  Spin_Init_2,
  Blank_Line,
  Blank_Line
};

//                                 "                    "
const char Spin_Play_1[] PROGMEM = "Make the bars";
const char Spin_Play_2[] PROGMEM = "dissappear by";
const char Spin_Play_3[] PROGMEM = "by spinning each";
const char Spin_Play_4[] PROGMEM = "dial";

const char *const Spin_Play[] PROGMEM =       // Instructions for Playing Puzzle
{   
  Spin_Play_1,
  Spin_Play_2,
  Spin_Play_3,
  Spin_Play_4
};


// Phone Definitions
const char Phone_Name[] PROGMEM =    "Phone Home";

//                                   "                    "
const char Phone_Init_1[] PROGMEM =  "Take Phone offline";
const char Phone_Init_2[] PROGMEM =  "by lifting the";
const char Phone_Init_3[] PROGMEM =  "Knife Switch";

const char *const Phone_Init[] PROGMEM =       // Instructions for Initializing Puzzle
{   
  Phone_Init_1,
  Phone_Init_2,
  Phone_Init_3,
  Blank_Line
};

//                                  "                    "
const char Phone_Play_1[] PROGMEM = "Connect/Reset Phone";
const char Phone_Play_2[] PROGMEM = "using Knife Switch.";
const char Phone_Play_3[] PROGMEM = "Dial 7 digit number";
const char Phone_Play_4[] PROGMEM = "from other puzzles.";

const char *const Phone_Play[] PROGMEM =       // Instructions for Playing Puzzle
{   
  Phone_Play_1,
  Phone_Play_2,
  Phone_Play_3,
  Phone_Play_4
};

