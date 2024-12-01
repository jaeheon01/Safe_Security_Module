#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "shared.h"


int vibration_detected = 0;
int motion_detected = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int step = 0;


int main(){
    pthread_t vibration_thread, motion_thread;

    if(pthread_create(&vibration_thread, NULL, vibration_sensor, NULL)){
        fprintf(stderr, "Error creating vibration sensor thread\n");
        return 1;
    }
    if(pthread_create(&motion_thread, NULL, motion_sensor, NULL)){
        fprintf(stderr, "Error creating motion sensor thread\n");
        return 2;
    }

    while(1){
		pthread_mutex_lock(&lock);
		while(step!=2){
			pthread_cond_wait(&cond,&lock);
		}
        if(vibration_detected && motion_detected){
            printf("[Main] Invade detected by both sensor!\n"); 
        }
        else{
            printf("[Main] No Invade detected!\n");         
        }
        motion_detected = 0;
        vibration_detected = 0;
        step = 0;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);
        sleep(1);
    }

    pthread_join(vibration_thread,NULL); 
    pthread_join(motion_thread,NULL);

    return 0;
}

