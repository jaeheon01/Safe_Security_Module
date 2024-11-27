#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "shared.h"

//thread 공유 전역 변수
int vibrationDetected = 0;
int motionDetected = 0;

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
    vibrationDetected = vibration_detected;
    motionDetected = motion_detected;

    while(1){
        if(vibrationDetected && motionDetected){
            printf("Motion detected!\n"); 
        }
        else{
            printf("No detected!\n");
        }
    }

    pthread_join(vibration_thread,NULL); //thread가 종료될때까지 메인 thread 대기
    pthread_join(motion_thread,NULL);

    return 0;
}