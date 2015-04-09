#include "OneWire.h"
#include "DallasTemperature.h"
#include "TimerOne.h"
#include "SPI.h" // new include
#include "Ethernet.h"
#include "WebServer.h"
#include "typedefs.h"
#include "mapping.h"

/*
 *   Thermometer
 */
#define TEMPERATURE_PRECISION 12

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Device addresses
DeviceAddress heatThermometer, pipeThermometer, boilThermometer;
float         heatTemp,        pipeTemp,        boilTemp;

/*
 *   Flow meter
 */

flow_sensor_t input_heat;
flow_sensor_t input_pipe;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;


/*
 *   Valves
 */
 bool ev0, ev1, ev2, ev3, ev4, ev5;

/*
 *   Ethernet
 */
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
/* This creates an instance of the webserver.  By specifying a prefix
 * of "", all pages will be at the root of the server. */
#define PREFIX ""
WebServer webserver(PREFIX, 80);

#define NAMELEN 32
#define VALUELEN 32

P(Begin_JSON) = "{";
P(End_JSON)   = "}";
P(Separator_JSON)   = ",";
P(heat_temp)   = "\"heat\":";
P(pipe_temp)   = "\"pipe\":";
P(boil_temp)   = "\"boil\":";
P(EV0)         = "\"ev0\":";
P(EV1)         = "\"ev1\":";
P(EV2)         = "\"ev2\":";
P(EV3)         = "\"ev3\":";
P(EV4)         = "\"ev4\":";
P(EV5)         = "\"ev5\":";


void parsedCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  
  int cFlow_heat = 0;
  int cFlow_pipe = 0;     
  int cEVstate   = 0; 
  int cI   = 0; // count how EV are ordered: should max 1 
  int cEV  = 0; // EV ordered 

  /* this line sends the standard "we're all OK" headers back to the
     browser */
  server.httpSuccess();

  /* if we're handling a GET or POST, we can output our data here.
     For a HEAD request, we just stop after outputting headers. */
  if (type == WebServer::HEAD)
    return;
    
  if (strlen(url_tail))
    {
      while (strlen(url_tail))
      {
        rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
        if (rc != URLPARAM_EOS) {
           if(strcmp(name, "FLOW_INPUT_HEAT") == 0) {
             cFlow_heat = atoi(value);
           }
           
           if(strcmp(name, "FLOW_INPUT_PIPE") == 0) {
             cFlow_pipe = atoi(value);             
           }
           
           if(strcmp(name, "EV0") == 0) {
             cEVstate = atoi(value);                  
             cI++;
             cEV = 0;
           }
           if(strcmp(name, "EV1") == 0) {
             cEVstate = atoi(value);                  
             cI++;
             cEV = 1;
           }
           if(strcmp(name, "EV2") == 0) {
             cEVstate = atoi(value);                  
             cI++;
             cEV = 2;
           }
           if(strcmp(name, "EV3") == 0) {
             cEVstate = atoi(value);                  
             cI++;
             cEV = 3;
           }
           if(strcmp(name, "EV4") == 0) {
             cEVstate = atoi(value);                  
             cI++;
             cEV = 4;
           }
           if(strcmp(name, "EV5") == 0) {
             cEVstate = atoi(value);                  
             cI++;
             cEV = 5;
           }
          
        }
      }
      
      // lets process this command
      if (cI > 1) {
        server.print("Error: Please only one EV at a time");
        return;
      }
      
      if ((cFlow_heat != 0) &&  (cFlow_pipe != 0)) {
        server.print("Error: One flow sensor at a time");
        return;
      }
        
      if (cFlow_heat != 0) {
        if (input_heat.targetEV != -1){
          server.print("Error: Input heat flow sensor is already in use");
          return;        
        } else {
           input_heat.targetEV          = cEV;
           input_heat.targetMilliLitres = cFlow_heat;
           input_heat.orderMilliLitres  = 0;
         }
      }
      
      if (cFlow_pipe != 0) {
        if (input_pipe.targetEV != -1){
          server.print("Error: Input pipe flow sensor is already in use");
          return;   
        } else {
           input_pipe.targetEV          = cEV;
           input_pipe.targetMilliLitres = cFlow_pipe;
           input_pipe.orderMilliLitres  = 0;
        }     
      }
      
      // lets open the selected valve
      valve(cEV, cEVstate);
            
    }
          

  server.printP(Begin_JSON);
  server.printP(heat_temp);
  server.print(sensors.getTempC(heatThermometer));
  server.printP(Separator_JSON); 
  
  server.printP(pipe_temp);
  server.print(sensors.getTempC(pipeThermometer)); 
  server.printP(Separator_JSON); 
  
  server.printP(boil_temp);
  server.print(sensors.getTempC(boilThermometer)); 
  server.printP(Separator_JSON); 
  
  server.printP(EV0);
  server.print(ev0); 
  server.printP(Separator_JSON); 

  server.printP(EV1);
  server.print(ev1);  
  server.printP(Separator_JSON); 
  
  server.printP(EV2);
  server.print(ev2);  
  server.printP(Separator_JSON); 
  
  server.printP(EV3);
  server.print(ev3);  
  server.printP(Separator_JSON); 
  
  server.printP(EV4);
  server.print(ev4);   
  server.printP(Separator_JSON); 
  
  server.printP(EV5);
  server.print(ev5); 
  server.printP(End_JSON); 


} 

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
    0,  // orderMilliLitres
    0,  // oldTime
    pulseCounter_heat, // pulseCounter
    0,  // targetMilliLitres;
    -1  // targetEV;
  };
  
  input_pipe = {
    1,  // sensorInterrupt
    3,  // sensorPin
    0,  // pulseCount
    0,  // flowRate
    0,  // flowMilliLitres
    0,  // totalMilliLitres
    0,  // orderMilliLitres
    0,  // oldTime
    pulseCounter_pipe, // pulseCounter
    0,  // targetMilliLitres;
    -1  // targetEV;
   };
  
  
  pinMode(input_heat.sensorPin, INPUT);
  digitalWrite(input_heat.sensorPin, HIGH);
  
  pinMode(input_pipe.sensorPin, INPUT);
  digitalWrite(input_pipe.sensorPin, HIGH);

  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(input_heat.sensorInterrupt, input_heat.pulseCounter, FALLING);
  attachInterrupt(input_pipe.sensorInterrupt, input_pipe.pulseCounter, FALLING);


  // Setup for relay
  pinMode(RELAY0_1, OUTPUT);
  pinMode(RELAY0_2, OUTPUT);
  pinMode(RELAY0_3, OUTPUT);
  pinMode(RELAY0_4, OUTPUT);
  pinMode(RELAY0_5, OUTPUT);
  pinMode(RELAY0_6, OUTPUT);
  pinMode(RELAY0_7, OUTPUT);
  pinMode(RELAY0_8, OUTPUT);
  pinMode(RELAY1_1, OUTPUT);
  pinMode(RELAY1_2, OUTPUT);

  
  // keep valves closed at setup
  valve(0, LOW);
  valve(1, LOW);
  valve(2, LOW);
  valve(3, LOW);
  valve(4, LOW);
  valve(5, LOW);
  
  
  /* initialize the Ethernet adapter */
  Ethernet.begin(mac);

  /* setup our default command that will be run when the user accesses
   * the root page on the server */
  webserver.setDefaultCommand(&parsedCmd);
  //webserver.setFailureCommand(&my_failCmd);
  
  webserver.addCommand("index.html", &parsedCmd);
  
  /* start the webserver */
  webserver.begin();

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
    sensor_t->orderMilliLitres += sensor_t->flowMilliLitres;
      
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
    
    if (sensor_t->targetEV != -1 && sensor_t->orderMilliLitres >= sensor_t->targetMilliLitres) {
      valve(sensor_t->targetEV, LOW);
      sensor_t->targetEV = -1;         
      sensor_t->orderMilliLitres = 0;
    }
  }
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


void valve(int id, bool state)
{
  
  switch(id) {
   case 0:
     if (state) {
       digitalWrite(RELAY0_1, HIGH);
       digitalWrite(RELAY0_2, LOW ); 
     } else {
       digitalWrite(RELAY0_1, LOW );
       digitalWrite(RELAY0_2, HIGH);    
     } 
     ev0 = state;  
   break; 
   case 1:
     if (state) {
       digitalWrite(RELAY0_3, HIGH);
       digitalWrite(RELAY0_4, LOW ); 
     } else {
       digitalWrite(RELAY0_3, LOW );
       digitalWrite(RELAY0_4, HIGH);    
     }     
     ev1 = state;  
   break; 
   case 2:
     if (state) {
       digitalWrite(RELAY0_5, HIGH);
       digitalWrite(RELAY0_6, LOW ); 
     } else {
       digitalWrite(RELAY0_5, LOW );
       digitalWrite(RELAY0_6, HIGH);    
     }     
     ev2 = state;  
   break; 
   case 3: 
     if (state) {
       digitalWrite(RELAY0_7, HIGH);
       digitalWrite(RELAY0_8, LOW ); 
     } else {
       digitalWrite(RELAY0_7, LOW );
       digitalWrite(RELAY0_8, HIGH);    
     }     
     ev3 = state;  
   break; 
   case 4:
      digitalWrite(RELAY1_1 , state);
      ev4 = state;  
   break;
   case 5:
      digitalWrite(RELAY1_2 , state);
      ev5 = state;  
   break;
    
  }
}  
  

void loop(void){
  char buff[64];
  int len = 64;

  /* process incoming connections one at a time forever */
  webserver.processConnection(buff, &len);
}


