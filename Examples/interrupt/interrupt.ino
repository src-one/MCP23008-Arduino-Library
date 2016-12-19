#include <Wire.h>
#include <MCP23008.h>

MCP23008 mcp;

byte interruptPin = 3;

byte mcpBtnPin = 0;
byte mcpLedPin = 1;

volatile boolean interruptCalled = false;

void interruptGateway() {
  interruptCalled = true;
}

void expanderInterrupt() {
  Serial.println("interrupt called");
  
  int state = mcp.digitalRead(mcpLedPin);
  mcp.digitalWrite(mcpLedPin, !state);
}


void setup() {
  Serial.begin(115200);
  
  attachInterrupt(interruptPin, interruptGateway, CHANGE);

  mcp.begin();
  mcp.setupInterrupts(true, false, LOW);
  
  mcp.setupInterruptPin(mcpBtnPin, CHANGE);
  mcp.pinMode(mcpBtnPin, INPUT);
  mcp.pullUp(mcpBtnPin, HIGH);
  
  mcp.pinMode(mcpLedPin, OUTPUT);
}

void loop() {
  if(interruptCalled == true) {
    expanderInterrupt();
    interruptCalled = false;
  }
}

