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
#define LED_PIN 12

// pthread_mutex_t signal_mutex = PTHREAD_MUTEX_INITIALIZER;
// int signal_status = LOW;
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

// int GPIOExport(int pin) {
//     char buffer[3];
//     int fd = open("/sys/class/gpio/export", O_WRONLY);
//     if (fd == -1) return -1;

//     snprintf(buffer, sizeof(buffer), "%d", pin);
//     write(fd, buffer, strlen(buffer));
//     close(fd);
//     return 0;
// }

// int GPIODirection(int pin, int dir) {
//     char path[DIRECTION_MAX];
//     snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
//     int fd = open(path, O_WRONLY);
//     if (fd == -1) return -1;

//     const char* direction = (dir == OUT) ? "out" : "in";
//     write(fd, direction, strlen(direction));
//     close(fd);
//     return 0;
// }

// int GPIOWrite(int pin, int value) {
//     char path[VALUE_MAX];
//     snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
//     int fd = open(path, O_WRONLY);
//     if (fd == -1) return -1;

//     const char* val_str = (value == HIGH) ? "1" : "0";
//     write(fd, val_str, strlen(val_str));
//     close(fd);
//     return 0;
// }

// int GPIOUnexport(int pin) {
//     char buffer[3];
//     int fd = open("/sys/class/gpio/unexport", O_WRONLY);
//     if (fd == -1) return -1;

//     snprintf(buffer, sizeof(buffer), "%d", pin);
//     write(fd, buffer, strlen(buffer));
//     close(fd);
//     return 0;
// }

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

// void update_signal_status(int new_status)
// {
//     pthread_mutex_lock(&signal_mutex);
//     signal_status = new_status;
//     pthread_mutex_unlock(&signal_mutex);
// }
