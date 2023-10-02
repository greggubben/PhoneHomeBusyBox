/*
 * Phone Dialer
 *
 * Dial the right phone number in time
 *
 * First the Dialer must be put online by throwing the knife switch
 * then the number must be dialed correctly
 * Audio will be used to generate phone sounds
 */
#include <Arduino.h>
#include <Bounce2.h>
#include <PhoneHome_Definitions.h>
#include <PhoneHome_Command.h>
#include <PhoneHome_PuzzleState.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// Puzzle definitions
const int   PuzzleId  = PHONE_ID;
const char* PuzzleName = "Dial Number";
bool puzzleInitialized = false;
char puzzleDifficulty = DIFFICULTY_EASY;

// Target Number
String targetNumber = "7950163";

// This pin indicates if the phone is online
const uint8_t onlinePin = 4;
// This pin indicates a number is being dialed
const uint8_t dialingPin = 5;
// This pin is the dialing pulses
const uint8_t pulsePin = 6; 

// Receive pin from DF Player
const uint8_t dfPlayerRxPin = 10;
// Transmit pin to DF Player
const uint8_t dfPlayerTxPin = 11;

// Maximum digits allowed in a phone number
const int maxPhoneNumberDigits = 7;
// Character representation of the number being dialed
char numberDialed[maxPhoneNumberDigits+1];
// The current digit dialed
int currentDigit;
// How many pulses have been detected for the current digit
int pulseCount;

// Define the States for the Dialer
typedef enum DialerStates {Idle, Online, Dialing, Connecting, Connected, Invalid} DialerStates;
// Character versions of State for Debug Printing
String dialerStatesString[] = {"Idle", "Online", "Dialing", "Connecting", "Connected", "Invalid"};
// Current state of the Dialer
DialerStates currentState = DialerStates::Idle;

// Debounce the Online transition
Bounce onlineSwitch = Bounce();
// Debounce the Dialing transition
Bounce dialingSwitch = Bounce();
// Debounce the Pulse transition
Bounce pulseSwitch = Bounce();

// Show the state of the switches
bool showSwitchState = true;

// DF Player settings
SoftwareSerial dfPlayerSerial(dfPlayerRxPin, dfPlayerTxPin); // RX, TX
DFRobotDFPlayerMini dfPlayer;

#define DFPLAYER_VOLUME 24
#define DIALTONE_MP3  1
#define RINGING_MP3   2

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  
}



// Turn on all visible lights and indicators for a little while as a test
void flashDisplays () {
  Serial.println(F("Flashing Displays"));
  // Turn on Lights and other indicators
  dfPlayer.play(DIALTONE_MP3);  //Play the Dialtone mp3
  delay(1000);
  // Turn off Lights and other indicators
  dfPlayer.stop();
}


// Check if the puzzle is in a ready state
bool puzzleReady() {
  onlineSwitch.update();
  return onlineSwitch.read() == HIGH;
}



// PUZZLE STATE FUNCTIONS

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

  puzzleDifficulty = (commandArgument.length() > 0) ? commandArgument[0] : DIFFICULTY_EASY;

  targetNumber = (commandArgument.length() > 1) ? commandArgument.substring(1) : "7950163";
}

// Perform the Initialization steps including any randomizations
void performInitialize() {

  // Perform first time tasks such as randomization of drivers
  if (!puzzleInitialized) {
    puzzleInitialized = true;

    if (!puzzleReady()) {
      // Need to let Command know we will be in this state for a while
      sendInitialize();
    }
  }

  // Wait until the puzzle has been reset by player
  if (puzzleReady()) {
    // Good to go - Let's play
    // Inform Command we are playing
    sendPlay();
    setPuzzleState(PuzzleStates::Playing);
    
    // Ensure the dialed phone number is empty
    zeroNumberDialed();
    showSwitchState = true; // First time through show the switch state
    setState(DialerStates::Idle);
  }
}


// Handle playing the puzzle
void performPlaying() {
  bool puzzleSolved = false;

  // Read state of all switches
  onlineSwitch.update();
  dialingSwitch.update();
  pulseSwitch.update();

  // If there is a change in state we want to print the update for debugging
  if (onlineSwitch.changed() || dialingSwitch.changed() || pulseSwitch.changed()) {
    showSwitchState = true;
  }

  if (showSwitchState) {
    Serial.print(F("Online Switch: "));
    Serial.print(onlineSwitch.read());
    Serial.print(F(" "));
    Serial.print(F("Dialing Switch: "));
    Serial.print(dialingSwitch.read());
    Serial.print(F(" "));
    Serial.print(F("Pulse Switch: "));
    Serial.print(pulseSwitch.read());
    Serial.println();
    showSwitchState = false;
  }

  // Taken offline. This overrides all other activity
  if (onlineSwitch.rose()) {
    Serial.println(F("Taken Offline - Knife Switch Opened"));
    setState(DialerStates::Idle);
    zeroNumberDialed();
    currentDigit = 0;
    pulseCount = 0;
  }

  switch (currentState) {
    // Idle waiting to be placed online
    case Idle:
      if (onlineSwitch.fell()) {
        Serial.println(F("Placed Online - Knife Switch Closed"));
        setState(DialerStates::Online);
        dfPlayer.loop(DIALTONE_MP3);
      }
      break;

    case Online:
      if (dialingSwitch.fell()) {
        dfPlayer.stop();
        Serial.println(F("Started Dialing"));
        setState(DialerStates::Dialing);
      }
      break;

    case Dialing:
      // Collect the pulse
      if (pulseSwitch.rose()) {
        pulseCount++;
      }

      // Done Dialing the Digit?
      if (dialingSwitch.rose()) {
        // Convert 10 pulses to 0
        if (pulseCount == 10) { pulseCount = 0;}
        Serial.print(F("Pulse Count = "));
        Serial.println(pulseCount);

        // Append the digit to the number being dialed
        numberDialed[currentDigit] = pulseCount + '0';
        currentDigit++;
        Serial.print(F("Number Dialed = "));
        Serial.println(numberDialed);

        if (targetNumber.equals(numberDialed)) {
          Serial.println(F("Connecting"));
          setState(DialerStates::Connecting);
        }
        else if (currentDigit >= maxPhoneNumberDigits) {
          Serial.println(F("Invalid Phone Number"));
          setState(DialerStates::Invalid);
        }
        pulseCount = 0;
      }
      break;

    case Connecting:
      // Play a ringing sound

      // Delay for a bit to allow for ringing effect
      dfPlayer.play(RINGING_MP3);
      delay(4000);

      // Play a sound file of someone answering

      setState(DialerStates::Connected);
      break;

    case Connected:
      // Nothing to do in this state, but wait to go offline via Knife Switch
      puzzleSolved = true;
      break;

    case Invalid:
      // Nothing to do in this state, but wait to go offline via Knife Switch
      // Could add a step before this to play the invalid number response
      break;

    default:
      break;
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

  // Set up the input pins and attach debounce
  pinMode(onlinePin, INPUT_PULLUP);
  onlineSwitch.attach(onlinePin);
  onlineSwitch.interval(10);
  onlineSwitch.update();
  pinMode(dialingPin, INPUT_PULLUP);
  dialingSwitch.attach(dialingPin);
  dialingSwitch.interval(5);
  dialingSwitch.update();
  pinMode(pulsePin, INPUT_PULLUP);
  pulseSwitch.attach(pulsePin);
  pulseSwitch.interval(5);
  pulseSwitch.update();

//dfPlayer.begin(dfPlayerSerial);
  dfPlayerSerial.begin(9600);

  while (!dfPlayer.begin(dfPlayerSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    if (dfPlayer.available()) {
      printDetail(dfPlayer.readType(), dfPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
    }
    delay(1000);
  }

  Serial.println(F("DFPlayer Mini online."));

  dfPlayer.volume(DFPLAYER_VOLUME);  //Set volume value. From 0 to 30

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

// Reset the number dialed back to 0s
void zeroNumberDialed() {
  memset(numberDialed, 0, sizeof numberDialed);
}

// Change the state
void setState(DialerStates newState) {
  Serial.print(F("Changing State from "));
  Serial.print(dialerStatesString[currentState]);
  Serial.print(F("("));
  Serial.print(currentState);
  Serial.print(F(") to "));
  Serial.print(dialerStatesString[newState]);
  Serial.print(F("("));
  Serial.print(newState);
  Serial.print(F(")"));
  Serial.println();
  currentState = newState;
}