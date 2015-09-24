#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "LCD.h"

#define FOUR_BIT_MODE 4
#define EIGHT_BIT_MODE 8
#define ROW_SHIFT 0x40

using namespace std;

// Constructor.
LCD::LCD(LCDpins *layout, unsigned char xferMode, unsigned char numLines) {
   lineCount = numLines;
   mode = xferMode == FOUR_BIT_MODE || xferMode == EIGHT_BIT_MODE ? xferMode : 0;
   anchorFlag = false;
   blinkFlag = false;
   cursorFlag = false; 
   displayFlag = true;
   anchorAddress = 0x00;

   if (mode == 0) {
      fprintf(stderr, "Improper mode, must enter a mode of 4 or 8!\n");
   } else {
      rs = new GPIO(layout->rs);
      rw = new GPIO(layout->rw);
      e = new GPIO(layout->e);

      ctrlPins = (GPIO **)calloc(mode, sizeof(GPIO *));
      int index;
      for (index = 0; index < mode; ++index) {
         ctrlPins[index] = new GPIO(layout->ctrlPins[index]);
      }

      if (mode == FOUR_BIT_MODE) {
         fourBitInit();
      } else {
         eightBitInit();
      }
   }
}

// Eight bit initialization routine.
void LCD::eightBitInit() {
   usleep(15 * 1000);
   ctrlPins[2]->setValue(1);
   ctrlPins[3]->setValue(1);

   signed char count = 3;
   // Initial setup bit sequence
   for (; count >= 0; --count) {
      writePins(10);
   }
   clearPins();

   // Function set
   while (busyFlag()) {
   }
   ctrlPins[2]->setValue(1);
   ctrlPins[3]->setValue(1);
   ctrlPins[4]->setValue(lineCount);
   writePins();
   clearPins();

   // Display on/off control
   while (busyFlag()) {
   }
   convertToPins(0x04);

   // Display on/off control
   while (busyFlag()) {
   }
   convertToPins(0x01);

   // Entry mode set
   while (busyFlag()) {
   }
   convertToPins(0x06);

   cursor(false);
}

// Eight bit initialization routine.
void LCD::fourBitInit() {
   usleep(15 * 1000);
   ctrlPins[2]->setValue(1);
   ctrlPins[3]->setValue(1);

   signed char count = 3;
   // Initial setup bit sequence
   for (; count >= 0; --count) {
      writePins(10);
   }
   clearPins();

   // Function set number of bits
   while (busyFlag()) {
   }
   ctrlPins[2]->setValue(1);
   writePins();
   clearPins();

   // Function set of lines
   while (busyFlag()) {
   }
   ctrlPins[2]->setValue(1);
   writePins();
   clearPins();
   while (busyFlag()) {
   }
   ctrlPins[0]->setValue(lineCount);
   writePins();
   clearPins();

   // Display on/off control
   while (busyFlag()) {
   }
   convertToPins(0x08);

   // Display on/off control
   while (busyFlag()) {
   }
   convertToPins(0x01);

   // Entry mode set
   while (busyFlag()) {
   }
   convertToPins(0x06);

   cursor(false);
}

// Destructor.
LCD::~LCD() {
   delete(rs);
   delete(rw);
   delete(e);

   int index;
   for (index = 0; index < mode; ++index) {
      delete(ctrlPins[index]);
   }
   free(ctrlPins);
}

// Enables the anchor cursor option.
void LCD::anchorCursor(bool enable) {
   anchorFlag = enable;
   anchorAddress = readCurrentAddress();
}

// Changes the blink state of the cursor.
void LCD::blink(bool enable) {
   blinkFlag = enable;
   displayOptions();
}

// Checks the current state of the busy flag.
bool LCD::busyFlag() {
   clearPins();
   rw->setValue(1);

   ctrlPins[0]->setDirection("in");

   unsigned char busyFlag = 0;
   e->setValue(1);
   busyFlag = ctrlPins[0]->getValue();
   e->setValue(0);

   ctrlPins[0]->setDirection("out");
   clearPins();

   return busyFlag == 1;
}

// Clears the display.
void LCD::clear() {
   unsigned char dataPins[] = {0, 0, 0, 0, 0, 0, 0, 1};
   LCDpins pins = {0, 0, 0, dataPins}; 
   command(&pins);
}

// Set all pins equal to 0.
void LCD::clearPins() {
   rs->setValue(0);
   rw->setValue(0);
   int index;
   for (index = 0; index < mode; ++index) {
      ctrlPins[index]->setValue(0);
   }
}

// Sends the command specified to the LCD.
void LCD::command(LCDpins *cmd) {
   signed char count = mode == FOUR_BIT_MODE ? 2 : 1;
   signed char index = 0;
   signed char pinIndex;

   for (; count > 0; --count) {
      while (busyFlag()) {
      }

      rs->setValue(cmd->rs);
      rw->setValue(cmd->rw);

      for (pinIndex = 0; pinIndex < mode; ++pinIndex) {
         ctrlPins[pinIndex]->setValue(cmd->ctrlPins[index++]);
      }
      writePins();
   }
   clearPins();
}

// Converts |character| into pin signals for output.
void LCD::convertToPins(unsigned char character) {
   unsigned char mask = 1;
   unsigned char shift = 8;
   unsigned char pin;
   unsigned char count = mode == FOUR_BIT_MODE ? 2 : 1;

   unsigned char rsValue = rs->getValue();
   unsigned char rwValue = rw->getValue();

   while (busyFlag()) {
   }

   for (; count > 0; --count) {

      rs->setValue(rsValue);
      rw->setValue(rwValue);

      for (pin = 0; pin < mode; ++pin) {
         ctrlPins[pin]->setValue(character >> --shift & mask); 
      }

      writePins();
   }

   clearPins();
}

// Changes the state of the cursor.
void LCD::cursor(bool enable) {
   cursorFlag = enable;
   displayOptions();
}

// Changes the state of the display.
void LCD::display(bool enable) {
   displayFlag = enable;
   displayOptions();
}

// Private helper method to update the display options.
void LCD::displayOptions() {
   unsigned char dataPins[] = {0, 0, 0, 0, 1, displayFlag, cursorFlag, blinkFlag};
   LCDpins pins = {0, 0, 0, dataPins}; 
   command(&pins);
}

// Moves the cursor to the set address on the LCD.
void LCD::moveCursor(unsigned short address) {
   anchorAddress = address;
   convertToPins(anchorAddress | 0x80);
}

// Prints the input string to the LCD on line |line|.
void LCD::print(const char *message) {
   signed char index;
   signed char length = strlen(message);

   for (index = 0; index < length; ++index) {
      printChar(message[index]);
   }

   if (anchorFlag) {
      moveCursor(anchorAddress);
   }
}

// Converts a character into pin signals.
void LCD::printChar(unsigned char character) {
   rs->setDirection("out");
   rs->setValue(1);
   rw->setDirection("out");
   rw->setValue(0);
   convertToPins(character);
}

// Prints the current state of the LCD pins.
void LCD::printLCDPins() {
   fprintf(stderr, "rs: %d\n", rs->getValue());
   fprintf(stderr, "rw: %d\n", rw->getValue());
   fprintf(stderr, "e: %d\n", e->getValue());

   int index;
   for (index = 0; index < mode; ++index) {
      fprintf(stderr, "db%d: %d\n", index, ctrlPins[index]->getValue());
   }
}

// Returns the current address of the cursor.
unsigned char LCD::readCurrentAddress() {
   signed char count = mode == FOUR_BIT_MODE ? 2 : 1;
   rw->setValue(1);

   int index;
   for (index = 0; index < mode; ++index) {
      ctrlPins[index]->setDirection("in");
   }

   unsigned char address = 0x00;
   for (; count > 0; --count) {
      e->setValue(1);

      for (index = 0; index < mode; ++index) {
         address = address << 1 | ctrlPins[index]->getValue(); 
      }

      e->setValue(0);
   }

   for (index = 0; index < mode; ++index) {
      ctrlPins[index]->setDirection("out");
   }

   clearPins();

   return address;
}

// Returns the cursor to the home location for the given line.
void LCD::returnHome(unsigned char line) {
   if (line <= lineCount) {
      moveCursor(line * ROW_SHIFT);
   } else {
      fprintf(stderr, "Line %d exceeds max LCD lines of %d (starting from 0).\n",
            line, lineCount);
   }
}

// Sets the screen to print in reverse if |enable| is true;
void LCD::reversePrint(bool enable) {
   unsigned char dataPins[] = {0, 0, 0, 0, 0, 1, !enable, 0};
   LCDpins pins = {0, 0, 0, dataPins}; 
   command(&pins);
}

// Writes the current state of pins to LCD.
void LCD::writePins() {
   e->setValue(1);
   usleep(50);
   e->setValue(0);
}

// Writes the current state of pins to LCD.
void LCD::writePins(unsigned char delay) {
   e->setValue(1);
   usleep(delay * 10);
   e->setValue(0);
}
