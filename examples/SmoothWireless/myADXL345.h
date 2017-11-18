#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>



void displaySensorDetails(Adafruit_ADXL345_Unified * accel);
void displayDataRate(Adafruit_ADXL345_Unified * accel);
void displayRange(Adafruit_ADXL345_Unified * accel);
void displayAccelValues(sensors_event_t * event);