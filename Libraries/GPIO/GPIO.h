#if !defined(GPIO_H)
#define GPIO_H

#define GPIO_PATH "/sys/class/gpio/"
#define STR_LEN 64 

// Class to encapsulate all of the functionality of a GPIO pin on a
// microcontroller. This source file is configured to work with a
// BeagleBoneBlack and as such exports and unexports pins by writing to the
// /sys/class/gpio/ directory. If this class is to be ported to other devices,
// the export path should be confirmed/altered for the new hardware.
class GPIO {
   private:
      int pin;
      char path[STR_LEN];
      int dirFd;
      int valFd;
      const char *dir;
      int val;

   public:
      // Constructor
      GPIO(int gpioPin);

      // Destructor
      ~GPIO();

      // Exports the gpio pin specified by pin by writing the number of the pin
      // to the file /sys/class/gpio/export. Returns true if the directory is
      // created/exists.
      bool exportPin();

      // Unexports a specified gpio pin. Returns true if the directory is
      // unexported or if the directory did not exists to begin with.
      bool unexportPin();

      // Retrieves the current direction of the specified gpio pin. Returns "in"
      // if the pin is set for input, "out" if the pin is set for output, or
      // NULL if the internal readFile call is unsuccessful.
      const char *getDirection();

      // Retrieves the current value of the specified gpio pin. Returns 1 if the 
      // pin is high, 0 if the pin is low, or -1 if the readFile call is 
      // unsuccessful.
      int getValue();

      // Sets the current direction of the specified gpio pin to the value of
      // newDirection. The value of newDirection can be "in" to set the pin as an
      // input, or "out" to set the pin as an output. Returns the number of bytes
      // written.
      int setDirection(const char *direction);

      // Sets the current value of the specified gpio pin to either 1 (high) or
      // 0 (low). Returns the number of bytes written.
      int setValue(int value);
};

#endif
