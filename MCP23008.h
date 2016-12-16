/***************************************************
  This is a library for the MCP23017 i2c port expander

  These displays use I2C to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#ifndef _MCP23008_H_
#define _MCP23008_H_

// Don't forget the Wire library
class MCP23008 {
public:
  void begin(uint8_t addr);
  void begin(void);

  void pinMode(uint8_t p, uint8_t d);
  void digitalWrite(uint8_t p, uint8_t d);
  void pullUp(uint8_t p, uint8_t d);
  uint8_t digitalRead(uint8_t p);

  void writeGPIO(uint8_t);
  uint8_t readGPIO();

  void setupInterrupts(uint8_t mirroring, uint8_t open, uint8_t polarity);
  void setupInterruptPin(uint8_t p, uint8_t mode);
  uint8_t getLastInterruptPin();
  uint8_t getLastInterruptPinValue();

 private:
  uint8_t i2caddr;

  uint8_t bitForPin(uint8_t pin);
  uint8_t regForPin(uint8_t pin, uint8_t portAddr);

  uint8_t readRegister(uint8_t addr);
  void writeRegister(uint8_t addr, uint8_t value);

  /**
   * Utility private method to update a register associated with a pin
   * reads its value, updates the particular bit, and writes its value.
   */
  void updateRegisterBit(uint8_t p, uint8_t pValue, uint8_t portAddr);

};

#define MCP23008_ADDRESS 0x20

// registers
#define MCP23008_IODIR 0x00
#define MCP23008_IPOL 0x01
#define MCP23008_GPINTEN 0x02
#define MCP23008_DEFVAL 0x03
#define MCP23008_INTCON 0x04
#define MCP23008_IOCON 0x05
#define MCP23008_GPPU 0x06
#define MCP23008_INTF 0x07
#define MCP23008_INTCAP 0x08
#define MCP23008_GPIO 0x09
#define MCP23008_OLAT 0x0A

#define MCP23008_INT_ERR 255

#endif
