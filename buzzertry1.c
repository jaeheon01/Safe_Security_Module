#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
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
    time_t buzzer_on_time = 0; // 부저 켜진 시점 기록
    int is_buzzer_on = 0;      // 현재 부저 상태 (0: OFF, 1: ON)

    while (1) {
        pthread_mutex_lock(&signal_mutex);
        if (signal_status == HIGH) {
            if (!is_buzzer_on) {
                // 부저가 꺼져 있으면 켠다
                printf("[BUZZER Thread] Turning BUZZER ON.\n");
                buzzer_on();
                buzzer_on_time = time(NULL);
                is_buzzer_on = 1;
            } else {
                // 부저가 켜져 있는 경우, 5초 경과했는지 확인
                time_t current_time = time(NULL);
                if (current_time - buzzer_on_time >= 5) {
                    printf("[BUZZER Thread] Turning BUZZER OFF after 5 seconds.\n");
                    buzzer_off();
                    is_buzzer_on = 0; // 상태 갱신
                }
            }
        } else {
            if (is_buzzer_on) {
                // signal_status가 LOW이면 부저를 즉시 끔
                printf("[BUZZER Thread] Turning BUZZER OFF.\n");
                buzzer_off();
                is_buzzer_on = 0; // 상태 갱신
            }
        }
        pthread_mutex_unlock(&signal_mutex);

        usleep(500 * 1000); // 500ms 대기
    }

    buzzer_cleanup();
    pthread_exit(NULL);
}

