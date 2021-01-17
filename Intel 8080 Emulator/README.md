# Intel 8080
Written in C++20, the goal of this project is to completely emulate every aspect of the Intel 8080 CPU, with the ultimate goal of using this chip to emulate one of the highest grossing and most influential video games of all time: 
Space Invaders

### Test ROM results
The emulator passes the following tests:
- [x] 8080PRE.COM 
- Preliminary test for 8080/8085 CPU Exerciser by Ian Bartholomew and Frank Cringles

![](screenshots/8080PRE.PNG)
- [x] TST8080.COM 
- 8080/8085 CPU Diagnostic, version 1.0, by Microcosm Associates
![]()
- [x] CPUTEST.COM
- 8080/8085 CPU Exerciser by Ian Bartholomew and Frank Cringles. This is a very thorough test that generates a CRC code for each group of tests. The test compares the reported CRC with results from tests against real silicon. This test takes several hours to complete, if ran at the i8080's standard clock of 2MHz, but this test like others where run on a AMD Ryzen 3900x 4.4 GHz CPU, making the test only take 4 minutes. 
![]()
- [x] 8080EXM.COM
- SuperSoft Associates CPU test from the Diagnostic II suite. When it displays "ABCDEF..." those are actually indications of test that have passed. Additional testing occurs during the "Begin Timing Test" and "End Timing Test" period. 
![]()

### Excel File
Custom file I made to keep track of all the instructions that the chip supports, the file shows the break down of the instructions and how the bits are organized and correlate with one another, this allowed me to  break up instructions to their significant bits and organize my code in such a way that is unique and no other emulator, I found, does it this way.

### Next Steps
Begin researching Space Invaders and the custom hardware made specifically for the game and begin emulating it, as well as flesh out the Intel 8080 emulator to support interrupts and its I/O capabilities.
