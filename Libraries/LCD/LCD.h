#if !defined(LCD_H)
#define LCD_H

#include "../GPIO/GPIO.h"

// Struct to hold all of the four main pins for the LCD panel.
typedef struct {
   unsigned char rs;
   unsigned char rw;
   unsigned char e;
   unsigned char *ctrlPins;
} LCDpins;

// Encapsulates all of the functionality of an LCD display. Supports both 4 bit
// and 8 bit operation. To adjust for different sizes LCD displays, edit the
// global variable ROW_SHIFT in the LCD.cpp source file.
class LCD {
   public:
      // Constructor.
      LCD(LCDpins *pins, unsigned char mode, unsigned char lineCount);

      // Destructor.
      ~LCD();

      // Anchors the cursor at its current location such that it will always
      // print starting from that location, pass true to anchor, false to 
      // unanchor.
      void anchorCursor(bool enable);

      // Enables cursor blinking, pass true to enable, false to disable.
      void blink(bool enable);

      // Enables the cursor, pass true to enable, false to disable.
      void cursor(bool enable);

      // Clears the LCD screen.
      void clear();

      // Sends the command specified to LCD. A command is a set of pin states
      // that the LCD interprets as an instruction. Many of these instructions
      // have been wrapped by functions in this library, but for the ones that
      // have not been implemented, this function allows the user to still use
      // the functionality.
      void command(LCDpins *cmd);

      // Enables the display, pass true to enable, false to disable.
      void display(bool enable);

      // Moves the cursor to the set display address.
      void moveCursor(unsigned short address);

      // Prints the input string to the LCD, the printing begins where the
      // cursor is currently anchored or located (if unanchored).
      void print(const char *message);

      // Prints the current state of the LCD pins.
      void printLCDPins();

      // Returns the current address of the cursor
      unsigned char readCurrentAddress();

      // Sends the cursor home on the designated line (line 0 is the first row).
      void returnHome(unsigned char line);

      // Enables reverse printing, pass true to enable, false otherwise.
      void reversePrint(bool enable);

   private:
      // Checks the current state of the busy flag, returns true if the LCD
      // controller is busy (must wait for it to be idle before issuing more
      // commands).
      bool busyFlag();

      // Clears all of the LCD pins.
      void clearPins();

      // Converts the input character into GPIO pin signals and writes the
      // command to the LCD controller.
      void convertToPins(unsigned char character);

      // Method for updating display state.
      void displayOptions();

      // Eight bit initialization routine.
      void eightBitInit();

      // Four bit initialization routine.
      void fourBitInit();

      // Writes the specified character to the LCD screen at the current cursor
      // location.
      void printChar(unsigned char character);

      // Writes the current state of the object's pins to the LCD.
      void writePins();

      // Writes the current state of the object's pins to the LCD with a delay.
      void writePins(unsigned char delay);

      unsigned char mode;
      unsigned char lineCount;
      unsigned char anchorAddress;
      bool anchorFlag;
      bool cursorFlag;
      bool blinkFlag;
      bool displayFlag;
      bool normalPrint;
      bool scrollFlag;
      GPIO *rs;
      GPIO *rw;
      GPIO *e;
      GPIO **ctrlPins;
};

#endif
