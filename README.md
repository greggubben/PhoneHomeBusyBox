# Phone Home Busy Box

This is a collection of repositories for a Busy Box placed into a briefcase. The purpose of the Busy Box is to determine a phone number by solving 4 puzzles. This phone number must be dialed in the 5<sup>th</sup> puzzle before time runs out.

There are 6 repositories for 6 of the modules. The 7<sup>th</sup> module is the power module and does not require any code.

- [**Control Module**](Controller) handles the flow of the game.
- [**Slide into Home**](Slider) puzzle is where sliders are moved to reveal the number on the analog meter.
- [**Hook Me Up**](ConnectWires) puzzle has the player connect wires between the top(Brown) and bottom(White) terminals to turn lights on if off or off if on revealing a binary number that must be converted to a base ten number.
- [**Flip the Bits**](FlipBits) puzzle uses switches to reveal a hexidecimal number that must be converted to a base ten number.
- [**Spin Digit**](SpinDigit) puzzle has the player make all the bars go out by spinning the number wheels to reveal the number.
- [**Dialer**](DialerPuzzle) puzzle is where the number must be dialed correctly in order to phone home.

The Phone Home Busy Box Briefcase layout looks like this:

![Picture of the Phone Home Busy Box Briefcase with all the Modules installed](images/Briefcase_Modules.jpg)

| :---: | :---: | :---: | :---: |
| **Slide into Home** | **Control Module** | **Flip the Bits** | Cable |
| **Hook me Up** | **Dialer** | **Spin Digit** | Power |

