# Phone Home Library

## Overview

This Library is included by all the modules to provide standard definitions and common code.

There are 3 key include files and a test sketch.

- [**PhoneHome_Definitions.h**](PhoneHome_Definitions.h) defines the common pins to be used by all of the modules.
- [**PhoneHome_Command.h**](PhoneHome_Command.h) handles all command communication into or out of the module. Communication can happen over PJON or the USB Serial connection.
- [**PhoneHome_PuzzleState.h**](PhoneHome_PuzzleState.h) manages the puzzle states for the puzzle modules. It also handles displaying the state using colors on the neopixel.
- [**PhoneHomeLib.ino**](PhoneHomeLib.inos) is a test sketch to test if the serial handling and state management is functioning.


## Architecture

Each module uses a single Arduino Nano to handle its part of the puzzle. There are 3 pins reserved for every module. Pin *A7* is left floating and used to help with generating a seed for randomization. Pin *A0* is used by the puzzle modules to display the state of the puzzle. In the control module this pin is not used. Communication happens over Pin *A1* for all modules.


### Communication

Communication happens using the [PJON \(Padded Jitterning Operative Network\)](https://github.com/gioblu/PJON) protocol over a single wire using [SoftwareBitBang](https://github.com/gioblu/PJON/tree/master/src/strategies/SoftwareBitBang). A common library, [PhoneHome_Command.h](PhoneHome_Command.h), is used to establish a common communication language between the **Control Module** and the puzzle modules.
The *PhoneHome_Command.h* library can accept command over both the PJON wire as well as over the USB Serial connection. Responses are always sent over both PJON and USB Serial. This flexibility allows a module to be tested in isolation from the rest of the briefcase.
Here is a list of the possible commands:

| Command | Format | Description |
| ------- | ------ | ----------- |
| **W**ake | `W` | From Control to Puzzle to Wake Up |
| **A**cknowledge | `A[short name]`<BR>`[short name]`=module | From Puzzle to Control acknowledging the Wake Up |
| **S**tart | `SD###`<BR>`D`=Difficulty<BR>`###`=number | From Control to Puzzle to starting playing at the requested Difficulty using the ###s provided. |
| **I**nitialize | `I[long name]`<BR>`[long name]`=module | From Puzzle to Control to ask player to initialize the puzzle using instructions provided. |
| **P**laying | `P[long name]`<BR>`[long name]`=module | From Puzzle to share state with Control and display playing instructions. |
| **D**one | `D` or `DY`<BR>`Y`=instructions | From Puzzle to Control to indicate puzzle is solved and to optionally display decoding instructions. |
| **L**ine | `L[display line]`<BR>`[display line]`=20 char for LCD | From Puzzle to Control to share 1 line of instuction text to display on LCD. |
| **N**ext | `N` | From Control to Puzzle requesting the next line of text to display on the LCD. |
| **T**une | `T` | Request to put the puzzle in Tune state. |


One of the risks of a single wire protocol is collisions with multiple devices trying to communicate at once. This is handled by following a request-response pattern and only having one puzzle active at a time. The **Control Module** activates each puzzle when it is that puzzle's time to be played which removes communication conflicts. The state is synchronized between the puzzle and **Control Module** using the following sequence diagram and state machine:

#### Sequence Diagram
```mermaid
---
title: Control Module and Puzzle Interactions
---
sequenceDiagram
  participant control as Control Module
  participant puzzle as Puzzle

  Note right of puzzle: On Power, Set Puzzle State to "Startup"
  control->>puzzle: 'W'ake Up
  activate puzzle
  puzzle->>puzzle: Check and Flash display
  puzzle->>puzzle: Set State to "Ready"
  puzzle->>control: 'A'cknowledge and Ready to Play
  deactivate puzzle

  control->>puzzle: 'S'tart playing with these number(s)
  activate puzzle

  alt Puzzle needs Initialized
    puzzle->>puzzle: Set State to "Initialize"
    puzzle-->>control: 'I'nitialization needed
    activate control
    Note right of puzzle: Display how to Initialize Puzzle
    loop Up to 4 display lines
      control-->>puzzle: 'N'ext line of text
      activate puzzle
      puzzle-->>control: 'L'ine of text
      deactivate puzzle
    end
    deactivate control
    puzzle->>puzzle: Wait for Player to Initialize Puzzle
  end

  puzzle->>puzzle: Set State to "Playing"
  puzzle-->>control: 'P'laying the game
  activate control
  Note right of puzzle: Display how to Play Puzzle
  loop Up to 4 display lines
    control-->>puzzle: 'N'ext line of text
    activate puzzle
    puzzle-->>control: 'L'ine of text
    deactivate puzzle
  end
  deactivate control

  puzzle->>puzzle: Wait for Player to Solve Puzzle

  puzzle->>puzzle: Set State to "Solved"
  puzzle->>control: 'D'one playing
  activate control
  Note right of puzzle: Display how to Interpret Number
  loop Up to 4 display lines
    control-->>puzzle: 'N'ext line of text
    activate puzzle
    puzzle-->>control: 'L'ine of text
    deactivate puzzle
  end
  deactivate control

  deactivate puzzle

```


### State

Each puzzle manages their state and shares that state with the **Control Module**. The following describes each of the puzzle module states:

- **Starting** state indicates the module has performed the setup functions and is waiting for the **Control Module** to acknowledge its presence.
- **Ready** state indicates the puzzle is ready to play. This state occurs after the **Control Module** has acknowledged the module's presence and the module performed the self checks.
- **Initialize** state indicates the puzzle needs the player to reset or initialize the puzzle. When the **Control Module** request the puzzle to play, there may be a need for player interaction to reset the puzzle so play can start.
- **Playing** state indicates the player is playing the puzzle. This state is entered after the Initialization steps are complete.
- **Solved** state indicates the player has solved the puzzle. The answer remains on the display.
- **Tuning** state indicates the puzzle has been placed in tuning mode.


#### State Diagram
```mermaid
---
title: Puzzle State Diagram
---
stateDiagram-v2
  direction TB

  state needsInitialized <<choice>>

  [*] --> Startup: Power Up
  Startup --> Ready: Control Wakes Up Puzzle
  Ready --> needsInitialized: Control Starts Puzzle
  needsInitialized --> Initialize: Puzzle<BR>needs to be<BR>Initialized
  needsInitialized --> Playing: Puzzle<BR>is ready to<BR>Play
  Initialize --> Playing: Puzzle Initialized
  Playing --> Done: Puzzle is Solved
  Done --> Ready: Control Wakes Puzzle<br>for another game

  classDef purple fill:#f0f,font-weight:bold
  classDef blue fill:#00F,color:white,font-weight:bold
  classDef red fill:#F00,color:white,font-weight:bold
  classDef yellow fill:#FF0,font-weight:bold
  classDef green fill:#0F0,font-weight:bold
  classDef cyan fill:#0FF,font-weight:bold

  class Startup purple
  class Ready blue
  class Initialize red
  class Playing yellow
  class Done green

```



## Attribution

I would like to thank [Playful Technology](https://www.youtube.com/@PlayfulTechnology) on YouTube for [introducing me to PJON as a cross module communication tool](https://www.youtube.com/watch?v=u8giZveqlxs&list=PLogiUurtMYtSxku2Itst0msCv8MC2w14P&index=1). 
