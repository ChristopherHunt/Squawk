#ifndef LSM303_H
#define LSM303_H

#define CTRL_REG1_A 0x20
#define CTRL_REG2_A 0x21
#define CTRL_REG3_A 0x22
#define CTRL_REG4_A 0x23
#define CTRL_REG5_A 0x24
#define CTRL_REG6_A 0x25
#define REFERENCE_A 0x26
#define STATUS_REG_A 0x27 
#define OUT_X_L_A 0x28
#define OUT_X_H_A 0x29
#define OUT_Y_L_A 0x2A
#define OUT_Y_H_A 0x2B
#define OUT_Z_L_A 0x2C
#define OUT_Z_H_A 0x2D
#define FIFO_CTRL_REG_A 0x2E
#define FIFO_SRC_REG_A 0x2F
#define INT1_CFG_A 0x30
#define INT1_SOURCE_A 0x31
#define INT1_THS_A 0x32
#define INT1_DURATION_A 0x33
#define INT2_CFG_A 0x34
#define INT2_SOURCE_A 0x35
#define INT2_THS_A 0x36
#define INT2_DURATION_A 0x37
#define CLICK_CFG_A 0x38
#define CLICK_SRC_A 0x39
#define CLICK_THS_A 0x3A
#define TIME_LIMIT_A 0x3B
#define TIME_LATENCY_A 0x3C
#define TIME_WINDOW_A 0x3D

#define DISABLE_ACCEL 0x07
#define ACCEL_ADDRESS 0x19
#define INIT_MASK 0x57
#define HIGH_SENSITIVITY 0x18
#define READ_PAD_BYTES 0x80

#define UNSIGNED_BYTE unsigned char

// Struct to hold all 3 axis acceleration values.
typedef struct Acceleration {
   short xVal;
   short yVal;
   short zVal;
} Acceleration;

// Class to wrap the major functionality of the LSM303 accelerometer. The
// accelerometer has a data and a clock line, and both need to be connected
// correctly in order to use the device. Additionally, data line is
// communicating with the device i2c, and it is important that the user
// specify which i2cBus (number) they are attempting to communicate over. On the
// BeagleBoneBlack there are 3 possible i2cBuses (0, 1 & 2). To use the device,
// first construct an LSM303 object and then initialize it. The initialization
// routine will enable the accelerometer which will begin transferring x,y,z
// acceleration data. The data stream can be stopped by calling disable(), and
// restarted with a call to enable().
class LSM303 {
   public:
      // Constructor
      LSM303(UNSIGNED_BYTE i2cBus);

      // Destructor
      ~LSM303();

      // Initializes the accelerometer by establishing a connection between it         
      // and the microcontroller. This method also enables the accelerometer.          
      // To enable data collection on all 3 axes, use the #define INIT_MASK.
      // Returns true if the initialization was successful.
      bool init(UNSIGNED_BYTE initMask);

      // Enabls the accelerometer using the value of initMask. To enable all 3
      // axes, use the #define INIT_MASK. Returns true if the accelerometer was
      // enabled.
      bool enable(UNSIGNED_BYTE initMask);

      // Reads count number of consecutive registers starting at register            
      // startReg into the array pointed to by buf. Returns the number of
      // bytes read.
      UNSIGNED_BYTE readReg(UNSIGNED_BYTE startReg, UNSIGNED_BYTE *buf, 
       UNSIGNED_BYTE count);

      // Writes the contents of mask to register reg on the LSM303. Returns the
      // number of bytesWritten.
      UNSIGNED_BYTE writeReg(UNSIGNED_BYTE reg, UNSIGNED_BYTE mask);

      // Reads the contents of the accelerometer, combines the appropriate
      // register values and places the updated values into the Acceleration
      // struct pointed to by accl. 
      void readAcceleration(Acceleration *accl);

      // Disables the accelerometer.
      bool disable();

   private:
      int fd;
      UNSIGNED_BYTE i2cBus;

      // Ensures that the read / write operation will not overrun the bounds of
      // the register space. Returns true if the supplied register range is
      // valid.
      bool validRegBounds(UNSIGNED_BYTE startReg, UNSIGNED_BYTE count);

};

#endif
