#ifndef TYPEDEFS_H
#define TYPEDEFS_H

typedef struct {
           byte          sensorInterrupt;
           byte          sensorPin;
  volatile byte          pulseCount;
           float         flowRate;
           unsigned int  flowMilliLitres;
           unsigned long totalMilliLitres;
           unsigned long orderMilliLitres; // current status of order
           unsigned long oldTime;
           void          (*pulseCounter)();
           int           targetMilliLitres;
           int           targetEV;
} flow_sensor_t;

#endif
