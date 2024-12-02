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
