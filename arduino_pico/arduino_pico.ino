#include "OneWire.h"
#include "DallasTemperature.h"
#include "TimerOne.h"

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS A0
#define TEMPERATURE_PRECISION 12

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Device addresses
DeviceAddress heatThermometer, pipeThermometer, boilThermometer;
float         heatTemp,        pipeTemp,        boilTemp;


void setup(void)
{
  // start serial port
  Serial.begin(57600);


  // Start up the library
  sensors.begin();

  // assign address manually.  the addresses below will beed to be changed
  // to valid device addresses on your bus.  device address can be retrieved
  // by using either oneWire.search(deviceAddress) or individually via
  // sensors.getAddress(deviceAddress, index)

  heatThermometer   = { 0x28, 0x82, 0x23, 0x28, 0x06, 0x00, 0x00, 0x91};
  pipeThermometer   = { 0x28, 0x8D, 0x90, 0x26, 0x06, 0x00, 0x00, 0xA1};
  boilThermometer   = { 0x28, 0xD3, 0x76, 0x28, 0x06, 0x00, 0x00, 0xCF};
 
  // set the resolution to 12 bit
  sensors.setResolution(heatThermometer, TEMPERATURE_PRECISION);
  sensors.setResolution(pipeThermometer, TEMPERATURE_PRECISION);
  sensors.setResolution(boilThermometer, TEMPERATURE_PRECISION);
  
  sensors.setWaitForConversion(false);
  
  Timer1.initialize(750000); 
  Timer1.attachInterrupt( timerIsr ); // attach the service routine here

}

void interruptTemperature (void) 
{

  heatTemp = sensors.getTempC(heatThermometer);
  pipeTemp = sensors.getTempC(pipeThermometer);  
  boilTemp = sensors.getTempC(boilThermometer);
  
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  sensors.requestTemperatures();
}

void timerIsr()
{ 
  interruptTemperature(); // get temperature data and request measure

}

void loop(void){


}
