#ifndef SHARED_H
#define SHARED_H

#include <pthread.h>

extern int motion_detected;
extern int vibration_detected;

extern pthread_mutex_t lock;
extern pthread_cond_t cond;
extern int step;

void* motion_sensor(void* arg);
void* vibration_sensor(void* arg);

#endif
