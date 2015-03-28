#ifndef TYPEDEFS_H
#define TYPEDEFS_H

typedef struct {
           byte          sensorInterrupt;
           byte          sensorPin;
  volatile byte          pulseCount;
           float         flowRate;
           unsigned int  flowMilliLitres;
           unsigned long totalMilliLitres;
           unsigned long oldTime;
           void          (*pulseCounter)();
} flow_sensor_t;

#endif
