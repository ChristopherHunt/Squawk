#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "GPIO.h"

using namespace std;

static const char *directions[] = {"in", "out"};
static const char *values[] = {"0", "1"};

// Checks to see if a directory exists for the given pin number. Returns true if
// the directory exists.
static bool dirExists(int pinNum) {
   bool dirExists = false;

   char gpioDirPath[STR_LEN] = {0};
   sprintf(gpioDirPath, GPIO_PATH "gpio%d", pinNum);

   int pinFd = open(gpioDirPath, O_DIRECTORY);

   if (pinFd > 0) {
      dirExists = true;
   }

   close(pinFd);

   return dirExists;
}

// Opens the file specified in path using the specified flags. Returns the file
// descriptor which references the file.
static int openFile(char *path, int flags) {
   int fd = open(path, flags);

   if (fd < 0) {
      fprintf(stderr, "Could not open file %s\n", path);
   }

   return fd;
}

// Reads numBytes from the front of the file specified by the file descriptor fd
// and stores that contents in buf. Returns the number of bytes read.
static int readFile(int fd, char *buf, int numBytes) {
   lseek(fd, 0, SEEK_SET);
   int numRead = read(fd, buf, numBytes);

   if (numRead <= 0) {
      fprintf(stderr, "Could not read from file\n");
   }

   return numRead;
}

// Writes numBytes from the content referenced by  data to the file referenced
// by the file descriptor fd. Returns the number of bytes written.
static int writeFile(int fd, const void *data, int numBytes) {
   lseek(fd, 0, SEEK_SET);
   int written = write(fd, data, numBytes);

   if (written <= 0) {
      fprintf(stderr, "Could not write to file\n");
   }

   return written;
}

// Instantiates a GPIO object referencing a specified GPIO pin.
GPIO::GPIO(int gpioPin) {
   pin = gpioPin;
   sprintf(path, GPIO_PATH "gpio%d", pin);
   exportPin();
}

// GPIO destructor (empty and just added for completeness).
GPIO::~GPIO() {
}

// Exports the gpio pin specified by pin by writing the number of the pin to the
// file /sys/class/gpio/export. Returns true if the directory is created/exists.
bool GPIO::exportPin() {
   bool dirCreated = false;

   // If the gpio is not currently exported, export it
   if (!dirExists(pin)) {
      char num[STR_LEN] = {0};
      sprintf(num, "%d", pin);
      char exportPath[] = {GPIO_PATH "export"};

      int exportFd = openFile(exportPath, O_WRONLY);
      if (exportFd) {
         writeFile(exportFd, &num, strlen(num)); 
         close(exportFd);
      }
   }

   dirCreated = dirExists(pin);

   // Populate GPIO 
   if (dirCreated) {
      char direction[STR_LEN] = {0};
      strncpy(direction, path, strlen(path));
      strcat(direction, "/direction");

      char value[STR_LEN] = {0};
      strncpy(value, path, strlen(path));
      strcat(value, "/value");

      dirFd = openFile(direction, O_RDWR);
      valFd = openFile(value, O_RDWR);
      setDirection("out");
      getDirection();      
      setValue(0);
      getValue();      
   } 

   return dirCreated;
}

// Unexports a specified gpio pin.  
bool GPIO::unexportPin() {
   if (dirExists(pin)) {
      char num[STR_LEN] = {0};
      sprintf(num, "%d", pin);
      char unexportPath[] = {GPIO_PATH "unexport"};

      int unexportFd = openFile(unexportPath, O_WRONLY);
      if (unexportFd) {
         writeFile(unexportFd, &num, strlen(num)); 
         close(unexportFd);
      }
   }

   close(dirFd);
   close(valFd);

   return !dirExists(pin);
}

// Retrieves the current direction of the specified gpio pin.
const char * GPIO::getDirection() {
   dir = NULL;
   char buffer[STR_LEN] = {0};

   if (readFile(dirFd, buffer, STR_LEN)) { 

      // Drop random newline added to end of buffer during read
      buffer[strlen(buffer) - 1] = '\0';

      if (strcmp(buffer, directions[0]) == 0) {
         dir = directions[0];
      } else if (strcmp(buffer, directions[1]) == 0) {
         dir = directions[1];
      }
   }

   return dir;
}

// Retrieves the current value of the specified gpio pin.
int GPIO::getValue() {
   val = -1;
   char buffer[STR_LEN] = {0};
   if (readFile(valFd, buffer, STR_LEN)) {
      val = atoi(buffer);
   }
   return val;
}

// Sets the current direction of the specified gpio pin to the value of          
// newDirection. 
int GPIO::setDirection(const char *newDirection) {
   int written = writeFile(dirFd, newDirection, strlen(newDirection));
   // Using fsync to ensure that the buffer is flushed.
   fsync(dirFd);
   return written;
}

// Sets the current value of the specified gpio pin to either 1 (high) or 0
// (low). Uses the global value's array as static values to use for writeFile.
int GPIO::setValue(int newValue) {
   const char *value = newValue == 0 ? values[0] : values[1];
   int written = writeFile(valFd, value, strlen(value));
   // Using fsync to ensure that the buffer is flushed.
   fsync(valFd);
   return written;
}
