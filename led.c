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
#define LED_PIN 23

// LED GPIO pin initialize
int led_init() {
    if (GPIOExport(LED_PIN) == -1) {
        fprintf(stderr, "[LED] Failed to export GPIO pin.\n");
        return -1;
    }
    if (GPIODirection(LED_PIN, OUT) == -1) {
        fprintf(stderr, "[LED] Failed to set GPIO direction.\n");
        return -1;
    }
    return 0;
}

int led_on() {
    return GPIOWrite(LED_PIN, HIGH);
}
int led_off() {
    return GPIOWrite(LED_PIN, LOW);
}
int led_cleanup() {
    return GPIOUnexport(LED_PIN);
}

// LED Thread
void* led_thread(void* arg) {
    led_init();
    while (1) {
        pthread_mutex_lock(&signal_mutex);
        if (signal_status == HIGH) {
            printf("[LED Thread] Turning LED ON.\n");
            led_on();
        } else {
            printf("[LED Thread] Turning LED OFF.\n");
            led_off();
        }
        pthread_mutex_unlock(&signal_mutex);
        usleep(500 * 1000); // 500ms 대기
    }
    led_cleanup();
    pthread_exit(NULL);
}
