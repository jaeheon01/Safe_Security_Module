#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "camera.h"

void captureImage() {
    char command[128]; // libcamera-still 명령어를 저장할 문자열 버퍼
    char filename[64]; // 캡처한 이미지 파일 이름을 저장할 문자열 버퍼

    // 현재 시간을 얻어와서 파일 이름을 생성하기 위해 time_t 타입 변수 생성
    time_t now = time(NULL);
    // time_t를 로컬 시간 구조체 tm 포인터로 변환
    struct tm *t = localtime(&now);
    // 날짜와 시간 정보를 이용해 파일명을 "/home/pi/Pictures/capture_YYYYMMDD_HHMMSS.jpg" 형태로 생성
    snprintf(filename, sizeof(filename), "/home/pi/Pictures/capture_%04d%02d%02d_%02d%02d%02d.jpg",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);

     // libcamera-still 명령어를 command 배열에 저장
    // -o 옵션을 통해 파일명을 지정, -t 1000은 1초 대기 후 촬영
    /*
    명령어 저장을 한 후에 system() 커맨드로 명령어 수행
    */
    snprintf(command, sizeof(command), "libcamera-still -o %s -t 1000", filename);

    printf("[Camera] Capturing image...\n");
    // system() 함수를 사용해 shell에서 libcamera-still 명령 실행
    int ret = system(command);
    // system() 호출 결과를 통해 성공/실패 메시지 출력
    if (ret == 0) {
        printf("[Camera] Image saved as '%s'\n", filename);
    } else {
        fprintf(stderr, "[Camera] Failed to capture image. Check the camera connection.\n");
    }
}
