#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "LSM303.h"

#define ACCEL_VALUES 6
#define BITS_PER_BYTE 8 
#define ACCEL_CALIBRATION 8192 

using namespace std;

// Constructor.
LSM303::LSM303(UNSIGNED_BYTE bus) {
   assert(bus >= 0);
   i2cBus = bus;
}

// Destructor.
LSM303::~LSM303() {
}

// Initializes the accelerometer by establishing a connection between it
// and the microcontroller. This method also enables the accelerometer.
// The initMask is the mask to write to register CTRL_REG1_A of the LSM303
// in order to enable its axes. To enable all 3, use the #define INIT_MASK.
// Additionally, this initialization routine assumes that the i2cBus is located
// at /dev/i2c-#, where the number is the i2cBus designation. This will not
// likely be the same on other devices (this is true on the BeagleBoneBlack
// running Debian). If this library is to be used with other microcontrollers,
// this setting needs to be checked/adjusted.
bool LSM303::init(UNSIGNED_BYTE initMask) {

   bool rtn = true;;
   char filename[16];
   snprintf(filename, sizeof(filename), "/dev/i2c-%d", i2cBus);
   fd = open(filename, O_RDWR); 

   if (fd < 0) {
      fprintf(stderr, "Could not open file/dev/i2c-%d.\n", i2cBus);
      rtn = false;
   }

   if (ioctl(fd, I2C_SLAVE, ACCEL_ADDRESS) < 0) {
      fprintf(stderr, "Could not establish i2c connection on i2cBus %d.\n", 
            i2cBus);
      rtn = false;
   }

   rtn = enable(initMask) ? rtn : false;

   return rtn;
}

// Enabls the accelerometer using the specified value of initMask. The value of
// the mask is written to register CTRL_REG1_A of the LSM303. To enable data
// collection on all 3 axes, use the #define INIT_MASK.
bool LSM303::enable(UNSIGNED_BYTE initMask) {

   bool status = writeReg(CTRL_REG1_A, initMask) > 0 ? true : false;
   status = writeReg(CTRL_REG4_A, HIGH_SENSITIVITY) > 0 ? status : false;

   return status; 
}

// Writes the contents of mask to register reg on the LSM303.
// Returns the number of bytesWritten, Will print an error message if the number
// of bytes written is not the intended number of bytes as specified by count,
// but will not kill the program's execution (to prevent realtime failures).
UNSIGNED_BYTE LSM303::writeReg(UNSIGNED_BYTE reg, UNSIGNED_BYTE mask) {

   UNSIGNED_BYTE bytesWritten = 0;
   if (validRegBounds(reg, 1)) {
      UNSIGNED_BYTE buffer[2] = {reg | READ_PAD_BYTES, mask};
      bytesWritten = write(fd, buffer, 2);

      if (!bytesWritten) {
         fprintf(stderr, "Unsuccessful write of %08X into regster %08X.\n", 
               reg, mask);
         char* error = strerror(errno);
         fprintf(stderr, error);
      }

   } else {
      fprintf(stderr, "Invalid register bounds, write aborted.\n");
   }

   return bytesWritten;
}

// Reads count number of consecutive registers starting at register startReg
// into the array pointed to by buf. Will print an error message if the number
// of bytes read is not the intended number of bytes as specified by count, but
// will not kill the program's execution (to prevent realtime failures).
UNSIGNED_BYTE LSM303::readReg(UNSIGNED_BYTE startReg, UNSIGNED_BYTE *buf, 
      UNSIGNED_BYTE count) {

   UNSIGNED_BYTE bytesRead = 0;

   if (validRegBounds(startReg, count)) {
      UNSIGNED_BYTE buffer = startReg | READ_PAD_BYTES;
      if (write(fd, &buffer, 1)) {
         bytesRead = read(fd, buf, count);
      }

      if (bytesRead != count) {
         fprintf(stderr, "Unsuccessful read of registers %08X - %08X.\n", 
               startReg, startReg + count);
         char* error = strerror(errno);
         fprintf(stderr, error);
      }

   } else {
      fprintf(stderr, "Invalid bounds, read aborted on registers %08X - %08x.\n",
            startReg, startReg + count);
   }

   return bytesRead;
}

// Ensures that the read / write operation will not overrun the bounds of the
// register space. From the datasheet, CTRL_REG1_A is the beginning of the valid
// register block and TIME_WINDOW_A is the end of the valid register block.
bool LSM303::validRegBounds(UNSIGNED_BYTE startReg, UNSIGNED_BYTE count) {

   if (startReg < CTRL_REG1_A || startReg + count > TIME_WINDOW_A) {
      return false; 
   }

   return true;
}

// Reads the contents of the accelerometer and writes the appropriate values to
// the Acceleration struct accl. Since the acceleration values are broken up
// over 2 registers per axis on the accelerometer there is some bitshifting
// required to combine their values to create an accurate final acceleration.
void LSM303::readAcceleration(Acceleration *accl) {

   UNSIGNED_BYTE data[ACCEL_VALUES];
   readReg(OUT_X_L_A, data, sizeof(data));
   accl->xVal = data[1] << BITS_PER_BYTE | data[0];
   accl->yVal = data[3] << BITS_PER_BYTE | data[2];
   accl->zVal = data[5] << BITS_PER_BYTE | data[4];
}

// Disables the accelerometer by writing the disable mask to CTRL_REG1_A.
bool LSM303::disable() {

   if (writeReg(CTRL_REG1_A, DISABLE_ACCEL) > 0) {
      return true;
   }

   return false;
}
