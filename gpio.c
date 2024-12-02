#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shared.h"

#define VALUE_MAX 40
#define DIRECTION_MAX 128

int GPIOExport(int pin) {
    char buffer[3];
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1) return -1;

    snprintf(buffer, sizeof(buffer), "%d", pin);
    write(fd, buffer, strlen(buffer));
    close(fd);
    return 0;
}

int GPIODirection(int pin, int dir) {
    char path[DIRECTION_MAX];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
    int fd = open(path, O_WRONLY);
    if (fd == -1) return -1;

    const char* direction = (dir == OUT) ? "out" : "in";
    write(fd, direction, strlen(direction));
    close(fd);
    return 0;
}

int GPIOWrite(int pin, int value) {
    char path[VALUE_MAX];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
    int fd = open(path, O_WRONLY);
    if (fd == -1) return -1;

    const char* val_str = (value == HIGH) ? "1" : "0";
    write(fd, val_str, strlen(val_str));
    close(fd);
    return 0;
}

int GPIOUnexport(int pin) {
    char buffer[3];
    int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd == -1) return -1;

    snprintf(buffer, sizeof(buffer), "%d", pin);
    write(fd, buffer, strlen(buffer));
    close(fd);
    return 0;
}
