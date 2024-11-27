#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

// GPIO 핀 방향 및 값 정의
#define OUT 1          // GPIO 핀 방향을 출력(OUT)으로 설정
#define HIGH 1         // GPIO 핀 값을 HIGH(1)으로 설정 (ON)
#define LED_PIN 27     // LED가 연결된 GPIO 핀 번호
#define VALUE_MAX 40   // GPIO 값 파일 경로 최대 크기
#define DIRECTION_MAX 128 // GPIO 방향 파일 경로 최대 크기

// GPIO 핀을 활성화하는 함수
static int GPIOExport(int pin) {
#define BUFFER_MAX 3   // GPIO 핀 번호를 저장할 버퍼 크기
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    // GPIO 핀을 활성화하기 위해 /sys/class/gpio/export 파일을 열기
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (-1 == fd) { // 파일 열기에 실패한 경우
        fprintf(stderr, "GPIO 활성화 파일 열기 실패!\n");
        return -1;  // 에러 코드 반환
    }

    // GPIO 핀 번호를 문자열로 변환하여 export 파일에 쓰기
    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written); // GPIO 핀 활성화
    close(fd); // 파일 닫기
    return 0;  // 성공적으로 완료
}

// GPIO 핀의 방향(입력/출력)을 설정하는 함수
static int GPIODirection(int pin, int dir) {
    static const char s_directions_str[] = "in\0out"; // 방향 설정 문자열 (in 또는 out)

    char path[DIRECTION_MAX]; // 방향 설정 파일 경로를 저장할 버퍼
    int fd;

    // /sys/class/gpio/gpio<번호>/direction 경로 생성
    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY); // 방향 설정 파일 열기
    if (-1 == fd) { // 파일 열기에 실패한 경우
        fprintf(stderr, "GPIO 방향 설정 파일 열기 실패!\n");
        return -1;  // 에러 코드 반환
    }

    // 방향 설정 (dir이 OUT이면 "out", 그렇지 않으면 "in")
    if (-1 == write(fd, &s_directions_str[OUT == dir ? 3 : 0], OUT == dir ? 3 : 2)) {
        fprintf(stderr, "GPIO 방향 설정 실패!\n");
        return -1;  // 에러 코드 반환
    }

    close(fd); // 파일 닫기
    return 0;  // 성공적으로 완료
}

// GPIO 핀의 값을 설정하는 함수 (HIGH 또는 LOW)
static int GPIOWrite(int pin, int value) {
    static const char s_values_str[] = "01"; // 값 설정 문자열 (0 또는 1)

    char path[VALUE_MAX]; // 값 설정 파일 경로를 저장할 버퍼
    int fd;

    // /sys/class/gpio/gpio<번호>/value 경로 생성
    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY); // 값 설정 파일 열기
    if (-1 == fd) { // 파일 열기에 실패한 경우
        fprintf(stderr, "GPIO 값 설정 파일 열기 실패!\n");
        return -1;  // 에러 코드 반환
    }

    // 값 설정 (value가 HIGH이면 "1", LOW이면 "0")
    if (1 != write(fd, &s_values_str[HIGH == value ? 1 : 0], 1)) {
        fprintf(stderr, "GPIO 값 쓰기 실패!\n");
        return -1;  // 에러 코드 반환
    }

    close(fd); // 파일 닫기
    return 0;  // 성공적으로 완료
}

// GPIO 핀을 비활성화하는 함수
static int GPIOUnexport(int pin) {
    char buffer[BUFFER_MAX]; // GPIO 핀 번호를 저장할 버퍼
    ssize_t bytes_written;
    int fd;

    // GPIO 핀을 비활성화하기 위해 /sys/class/gpio/unexport 파일 열기
    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (-1 == fd) { // 파일 열기에 실패한 경우
        fprintf(stderr, "GPIO 비활성화 파일 열기 실패!\n");
        return -1;  // 에러 코드 반환
    }

    // GPIO 핀 번호를 문자열로 변환하여 unexport 파일에 쓰기
    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written); // GPIO 핀 비활성화
    close(fd); // 파일 닫기
    return 0;  // 성공적으로 완료
}

// 메인 함수: LED를 켜고 10초 동안 유지한 뒤 종료
int main() {
    // 1. LED_PIN 활성화
    if (GPIOExport(LED_PIN) == -1) return 1;

    // 2. LED_PIN 방향을 출력(OUT)으로 설정
    if (GPIODirection(LED_PIN, OUT) == -1) return 2;

    // 3. LED를 켜기 (GPIO 핀 값을 HIGH로 설정)
    if (GPIOWrite(LED_PIN, HIGH) == -1) return 3;

    printf("LED가 켜졌습니다.\n");

    // 4. 10초 동안 LED를 유지
    sleep(10);

    // 5. LED_PIN 비활성화
    if (GPIOUnexport(LED_PIN) == -1) return 4;

    return 0; // 프로그램 정상 종료
}
