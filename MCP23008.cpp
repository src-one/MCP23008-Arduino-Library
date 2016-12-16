/***************************************************
 This is a library for the MCP23008 i2c port expander

 These displays use I2C to communicate, 2 pins are required to
 interface
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries.
 BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Wire.h>
#ifdef __AVR
  #include <avr/pgmspace.h>
#elif defined(ESP8266)
  #include <pgmspace.h>
#endif
#include "MCP23008.h"

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// minihelper to keep Arduino backward compatibility
static inline void wiresend(uint8_t x) {
#if ARDUINO >= 100
	Wire.write((uint8_t) x);
#else
	Wire.send(x);
#endif
}

static inline uint8_t wirerecv(void) {
#if ARDUINO >= 100
	return Wire.read();
#else
	return Wire.receive();
#endif
}

/**
 * Bit number associated to a give Pin
 */
uint8_t MCP23008::bitForPin(uint8_t pin){
	return pin%8;
}

/**
 * Register address, port dependent, for a given PIN
 */
uint8_t MCP23008::regForPin(uint8_t pin, uint8_t portAddr){
	return portAddr;
}

/**
 * Reads a given register
 */
uint8_t MCP23008::readRegister(uint8_t addr){
	// read the current GPINTEN
	Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
	wiresend(addr);
	Wire.endTransmission();
	Wire.requestFrom(MCP23008_ADDRESS | i2caddr, 1);
	return wirerecv();
}


/**
 * Writes a given register
 */
void MCP23008::writeRegister(uint8_t regAddr, uint8_t regValue){
	// Write the register
	Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
	wiresend(regAddr);
	wiresend(regValue);
	Wire.endTransmission();
}


/**
 * Helper to update a single bit of a register.
 * - Reads the current register value
 * - Writes the new register value
 */
void MCP23008::updateRegisterBit(uint8_t pin, uint8_t pValue, uint8_t portAddr) {
	uint8_t regValue;
	uint8_t regAddr = regForPin(pin, portAddr);
	uint8_t bit=bitForPin(pin);
	regValue = readRegister(regAddr);

	// set the value for the particular bit
	bitWrite(regValue,bit,pValue);

	writeRegister(regAddr,regValue);
}

////////////////////////////////////////////////////////////////////////////////

/**
 * Initializes the MCP23008 given its HW selected address, see datasheet for Address selection.
 */
void MCP23008::begin(uint8_t addr) {
	if (addr > 7) {
		addr = 7;
	}
	i2caddr = addr;

	// set defaults!
	// all inputs on port
	writeRegister(MCP23008_IODIR, 0xff);
}

/**
 * Initializes the default MCP23008, with 000 for the configurable part of the address
 */
void MCP23008::begin(void) {
	begin(0);
}

/**
 * Sets the pin mode to either INPUT or OUTPUT
 */
void MCP23008::pinMode(uint8_t p, uint8_t d) {
	updateRegisterBit(p, (d==INPUT), MCP23008_IODIR);
}

/**
 * Read the GPIOs, and return its current 8 bit value.
 */
uint8_t MCP23008::readGPIO() {
	// read the current GPIO output latches
	Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
	wiresend(MCP23008_GPIO);
	Wire.endTransmission();

	Wire.requestFrom(MCP23008_ADDRESS | i2caddr, 1);
	return wirerecv();
}

/**
 * Writes all the pins in one go. This method is very useful if you are implementing a multiplexed matrix and want to get a decent refresh rate.
 */
void MCP23008::writeGPIO(uint8_t ba) {
	Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
	wiresend(MCP23008_GPIO);
	wiresend(ba & 0xFF);
	wiresend(ba >> 8);
	Wire.endTransmission();
}

void MCP23008::digitalWrite(uint8_t pin, uint8_t d) {
	uint8_t gpio;
	uint8_t bit = bitForPin(pin);

	// read the current GPIO output latches
	uint8_t regAddr = regForPin(pin, MCP23008_OLAT);
	gpio = readRegister(regAddr);

	// set the pin and direction
	bitWrite(gpio, bit, d);

	// write the new GPIO
	regAddr = regForPin(pin, MCP23008_GPIO);
	writeRegister(regAddr, gpio);
}

void MCP23008::pullUp(uint8_t p, uint8_t d) {
	updateRegisterBit(p, d, MCP23008_GPPU);
}

uint8_t MCP23008::digitalRead(uint8_t pin) {
	uint8_t bit = bitForPin(pin);
	uint8_t regAddr = regForPin(pin, MCP23008_GPIO);

	return (readRegister(regAddr) >> bit) & 0x1;
}

/**
 * Configures the interrupt system.
 * Mirroring will OR both INT pin.
 * OpenDrain will set the INT pin to value or open drain.
 * polarity will set LOW or HIGH on interrupt.
 * Default values after Power On Reset are: (false, flase, LOW)
 * If you are connecting the INT pin to arduino 2/3, you should configure the interrupt handling as FALLING with
 * the default configuration.
 */
void MCP23008::setupInterrupts(uint8_t mirroring, uint8_t openDrain, uint8_t polarity){
	uint8_t ioconfValue=readRegister(MCP23008_IOCON);
	bitWrite(ioconfValue, 6, mirroring);
	bitWrite(ioconfValue, 2, openDrain);
	bitWrite(ioconfValue, 1, polarity);
	writeRegister(MCP23008_IOCON, ioconfValue);
}

/**
 * Set's up a pin for interrupt. uses arduino MODEs: CHANGE, FALLING, RISING.
 *
 * Note that the interrupt condition finishes when you read the information about the port / value
 * that caused the interrupt or you read the port itself. Check the datasheet can be confusing.
 *
 */
void MCP23008::setupInterruptPin(uint8_t pin, uint8_t mode) {

	// set the pin interrupt control (0 means change, 1 means compare against given value);
	updateRegisterBit(pin, (mode!=CHANGE), MCP23008_INTCON);
	// if the mode is not CHANGE, we need to set up a default value, different value triggers interrupt

	// In a RISING interrupt the default value is 0, interrupt is triggered when the pin goes to 1.
	// In a FALLING interrupt the default value is 1, interrupt is triggered when pin goes to 0.
	updateRegisterBit(pin, (mode==FALLING), MCP23008_DEFVAL);

	// enable the pin for interrupt
	updateRegisterBit(pin, HIGH, MCP23008_GPINTEN);

}

uint8_t MCP23008::getLastInterruptPin(){
	uint8_t intf;

	intf = readRegister(MCP23008_INTF);

	for(int i=0; i<8; i++) if (bitRead(intf, i)) return i;

	return MCP23008_INT_ERR;

}
uint8_t MCP23008::getLastInterruptPinValue(){
	uint8_t intPin=getLastInterruptPin();

	if(intPin != MCP23008_INT_ERR){
		uint8_t intcapreg = regForPin(intPin, MCP23008_INTCAP);
		uint8_t bit = bitForPin(intPin);

		return (readRegister(intcapreg) >> bit) & (0x01);
	}

	return MCP23008_INT_ERR;
}
