#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "shared.h"
#include "camera.h"


int vibration_detected = 0;
int motion_detected = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int step = 0;


int main(){
    pthread_t vibration_thread, motion_thread;
    // 진동 센서 스레드 생성
    if(pthread_create(&vibration_thread, NULL, vibration_sensor, NULL)){
        fprintf(stderr, "Error creating vibration sensor thread\n");
        return 1;

    // 모션 센서 스레드 생성
    }
    if(pthread_create(&motion_thread, NULL, motion_sensor, NULL)){
        fprintf(stderr, "Error creating motion sensor thread\n");
        return 2;
    }

    // 센서 감지 루프
    while(1){
		pthread_mutex_lock(&lock);
		while(step!=2){ // 두 센서가 모두 감지될 때까지 대기
			pthread_cond_wait(&cond,&lock);
		}
        if(vibration_detected && motion_detected){
            printf("[Main] Invade detected by both sensors!\n"); 
            // Image Capture
            captureImage();
        }
        else{
            printf("[Main] No Invade detected!\n");         
        }
        motion_detected = 0;
        vibration_detected = 0;
        step = 0;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);
        // 30초 대기
        sleep(30);
    }

    pthread_join(vibration_thread,NULL); 
    pthread_join(motion_thread,NULL);

    return 0;
}

