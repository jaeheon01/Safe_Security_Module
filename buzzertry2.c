
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shared.h"

#define OUT 1
#define HIGH 1
#define LOW 0
#define VALUE_MAX 40
#define DIRECTION_MAX 128
#define BUZZER_PIN 16

// buzzer initialize
int buzzer_init() {
    if (GPIOExport(BUZZER_PIN) == -1) {
        fprintf(stderr, "[BUZZER] Failed to export GPIO pin.\n");
        return -1;
    }
    if (GPIODirection(BUZZER_PIN, OUT) == -1) {
        fprintf(stderr, "[BUZZER] Failed to set GPIO direction.\n");
        return -1;
    }
    return 0;
}

int buzzer_on() {
    return GPIOWrite(BUZZER_PIN, HIGH);
}
int buzzer_off() {
    return GPIOWrite(BUZZER_PIN, LOW);
}
int buzzer_cleanup() {
    return GPIOUnexport(BUZZER_PIN);
}

// Buzzer thread
void* buzzer_thread(void* arg) {
    buzzer_init();
    while (1) {
        pthread_mutex_lock(&signal_mutex);

        if (signal_status == HIGH) {
            printf("[BUZZER Thread] Turning BUZZER ON.\n");
            buzzer_on();

            // 7초 동안 대기
            pthread_mutex_unlock(&signal_mutex);
            sleep(7); // 7초 대기
            
            // 7초 후 부저 끄기
            printf("[BUZZER Thread] 7 seconds passed. Turning BUZZER OFF.\n");
            buzzer_off();

            // 부저가 꺼진 후 signal_status를 LOW로 설정
            pthread_mutex_lock(&signal_mutex);
            signal_status = LOW;
        } else if (signal_status == LOW) {
            // 부저가 꺼진 상태 유지
            printf("[BUZZER Thread] BUZZER is OFF.\n");
            buzzer_off();
        }

        pthread_mutex_unlock(&signal_mutex);
        usleep(500 * 1000); // 500ms 대기
    }

    buzzer_cleanup();
    pthread_exit(NULL);
}
