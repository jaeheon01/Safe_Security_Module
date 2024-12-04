#ifndef SHARED_H
#define SHARED_H

#include <pthread.h>

// shared flag, signal status
extern int motion_detected;
extern int vibration_detected;
extern int signal_status;

extern pthread_mutex_t lock;
extern pthread_mutex_t signal_mutex;
extern pthread_cond_t cond;
extern int step;

#define HIGH 1
#define LOW 0
#define OUT 1

int GPIOExport(int pin);
int GPIODirection(int pin, int dir);
int GPIOWrite(int pin, int value);
int GPIOUnexport(int pin);

// sensor function
void* motion_sensor(void* arg);
void* vibration_sensor(void* arg);

// actuator function
void* buzzer_thread(void* arg);
void* led_thread(void* arg);

void update_signal_status(int new_status);
// LED control function
int led_init();
int led_on();
int led_off();
int led_cleanup();

// BUZZER control function
int buzzer_init();
int buzzer_on();
int buzzer_off();
int buzzer_cleanup();

#endif
