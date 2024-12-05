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
#define BUZZER_PIN 24

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
        } else {
            printf("[BUZZER Thread] Turning BUZZER OFF.\n");
            buzzer_off();
        }
        pthread_mutex_unlock(&signal_mutex);
        usleep(500 * 1000); // 500ms 대기
    }
    buzzer_cleanup();
    pthread_exit(NULL);
}
