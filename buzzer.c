#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "shared.h"

#define OUT 1
#define HIGH 1
#define LOW 0
#define BUZZER_PIN 24 // 부저가 연결된 GPIO 핀 번호
#define VALUE_MAX 40
#define DIRECTION_MAX 128

static int GPIOExport(int pin);
static int GPIODirection(int pin, int dir);
static int GPIOWrite(int pin, int value);
static int GPIOUnexport(int pin);

void* buzzer_control(void* arg) {
    if (GPIOExport(BUZZER_PIN) == -1) pthread_exit(NULL);
    if (GPIODirection(BUZZER_PIN, OUT) == -1) pthread_exit(NULL);

    while (1) {
        pthread_mutex_lock(&lock);
        while (step != 2) { // 데이터가 수신될 때까지 대기
            pthread_cond_wait(&cond, &lock);
        }

        // 두 센서가 모두 감지된 경우 Buzzer ON, 그렇지 않으면 OFF
        if (motion_detected && vibration_detected) {
            printf("[Buzzer] Intrusion detected by both sensors! Turning Buzzer ON.\n");
            GPIOWrite(BUZZER_PIN, HIGH);
        } else {
            printf("[Buzzer] No intrusion detected. Turning Buzzer OFF.\n");
            GPIOWrite(BUZZER_PIN, LOW);
        }

        step = 0; // 작업 완료 후 단계 초기화
        pthread_mutex_unlock(&lock);
        usleep(500 * 1000); // 500ms 대기
    }

    if (GPIOUnexport(BUZZER_PIN) == -1) {
        fprintf(stderr, "[Buzzer] Failed to unexport GPIO pin.\n");
    }
    pthread_exit(NULL);
}

static int GPIOExport(int pin) {
    char buffer[3];
    ssize_t bytes_written;
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1) return -1;

    bytes_written = snprintf(buffer, sizeof(buffer), "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return 0;
}

static int GPIODirection(int pin, int dir) {
    char path[DIRECTION_MAX];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
    int fd = open(path, O_WRONLY);
    if (fd == -1) return -1;

    const char* direction = (dir == OUT) ? "out" : "in";
    write(fd, direction, strlen(direction));
    close(fd);
    return 0;
}

static int GPIOWrite(int pin, int value) {
    char path[VALUE_MAX];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
    int fd = open(path, O_WRONLY);
    if (fd == -1) return -1;

    const char* val_str = (value == HIGH) ? "1" : "0";
    write(fd, val_str, strlen(val_str));
    close(fd);
    return 0;
}

static int GPIOUnexport(int pin) {
    char buffer[3];
    ssize_t bytes_written;
    int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd == -1) return -1;

    bytes_written = snprintf(buffer, sizeof(buffer), "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return 0;
}
