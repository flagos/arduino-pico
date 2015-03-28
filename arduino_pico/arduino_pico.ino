#include "OneWire.h"
#include "DallasTemperature.h"
#include "TimerOne.h"
#include "typedefs.h"

// Data wire is plugged into port ONE_WIRE_BUS on the Arduino
#define ONE_WIRE_BUS A0
#define TEMPERATURE_PRECISION 12

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Device addresses
DeviceAddress heatThermometer, pipeThermometer, boilThermometer;
float         heatTemp,        pipeTemp,        boilTemp;


flow_sensor_t input_heat;
flow_sensor_t input_pipe;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;


void setup(void)
{
  // start serial port
  Serial.begin(57600);

  // 1-wire devices temperature

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
  
  
  // Hall effect flow meter
  input_heat = {
    0,  // sensorInterrupt
    2,  // sensorPin
    0,  // pulseCount
    0,  // flowRate
    0,  // flowMilliLitres
    0,  // totalMilliLitres
    0,  // oldTime
    pulseCounter_heat // pulseCounter
  };
  
  input_pipe = {
    1,  // sensorInterrupt
    3,  // sensorPin
    0,  // pulseCount
    0,  // flowRate
    0,  // flowMilliLitres
    0,  // totalMilliLitres
    0,  // oldTime
    pulseCounter_pipe // pulseCounter
   };
  
  
  pinMode(input_heat.sensorPin, INPUT);
  digitalWrite(input_heat.sensorPin, HIGH);
  
  pinMode(input_pipe.sensorPin, INPUT);
  digitalWrite(input_pipe.sensorPin, HIGH);

  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(input_heat.sensorInterrupt, input_heat.pulseCounter, FALLING);
  attachInterrupt(input_pipe.sensorInterrupt, input_pipe.pulseCounter, FALLING);


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
  
  loop_water_flow(&input_heat);
  loop_water_flow(&input_pipe);
}

void loop_water_flow(flow_sensor_t* sensor_t)
{ 
   if((millis() - sensor_t->oldTime) > 1000)    // Only process counters once per second
  { 
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensor_t->sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    sensor_t->flowRate = ((1000.0 / (millis() - sensor_t->oldTime)) * sensor_t->pulseCount) / calibrationFactor;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    sensor_t->oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    sensor_t->flowMilliLitres = (sensor_t->flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    sensor_t->totalMilliLitres += sensor_t->flowMilliLitres;
      
    unsigned int frac;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(sensor_t->flowRate));  // Print the integer part of the variable
    Serial.print(".");             // Print the decimal point
    // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
    frac = (sensor_t->flowRate - int(sensor_t->flowRate)) * 10;
    Serial.print(frac, DEC) ;      // Print the fractional part of the variable
    Serial.print("L/min");
    // Print the number of litres flowed in this second
    Serial.print("  Current Liquid Flowing: ");             // Output separator
    Serial.print(sensor_t->flowMilliLitres);
    Serial.print("mL/Sec");

    // Print the cumulative total of litres flowed since starting
    Serial.print("  Output Liquid Quantity: ");             // Output separator
    Serial.print(sensor_t->totalMilliLitres);
    Serial.println("mL"); 

    // Reset the pulse counter so we can start incrementing again
    sensor_t->pulseCount = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt( sensor_t->sensorInterrupt, sensor_t->pulseCounter, FALLING);
  }
}

void loop(void){
  

}

/*
Interrupt Service Routine
 */
void pulseCounter_heat()
{
  // Increment the pulse counter
  input_heat.pulseCount++;
}

void pulseCounter_pipe()
{
  // Increment the pulse counter
  input_pipe.pulseCount++;
}
