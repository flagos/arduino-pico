#include "OneWire.h"
#include "DallasTemperature.h"
#include "TimerOne.h"

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 7
#define TEMPERATURE_PRECISION 12

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress heatThermometer, pipeThermometer,boilThermometer;

void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");

  // Start up the library
  sensors.begin();

  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

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
  
  Timer1.initialize(10000000); // set a timer of length 20000 microseconds (or 0.02 sec - or 50Hz => frequency of electricity in europe)
  Timer1.attachInterrupt( timerIsr ); // attach the service routine here

}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
}

// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress)
{
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();    
}

// main function to print information about a device
void printData(DeviceAddress deviceAddress)
{
  Serial.print("Device Address: ");
  printAddress(deviceAddress);
  Serial.print(" ");
  printTemperature(deviceAddress);
  Serial.println();
}

void timerISR(void)
{ 
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  Serial.println("DONE");

  // print the device information
  printData(heatThermometer);
  printData(pipeThermometer);  
  printData(boil
  Thermometer);
}

void loop(void){
 // do nothing 
}
