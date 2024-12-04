#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "camera.h"

void captureImage() {
    char command[128];
    char filename[64];

    //Create File name based on current time!!!
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    snprintf(filename, sizeof(filename), "/home/pi/Pictures/capture_%04d%02d%02d_%02d%02d%02d.jpg",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);

    //Create libcamera-still command line
    snprintf(command, sizeof(command), "libcamera-still -o %s -t 1000", filename);

    printf("[Camera] Capturing image...\n");
    int ret = system(command);
    if (ret == 0) {
        printf("[Camera] Image saved as '%s'\n", filename);
    } else {
        fprintf(stderr, "[Camera] Failed to capture image. Check the camera connection.\n");
    }
}
