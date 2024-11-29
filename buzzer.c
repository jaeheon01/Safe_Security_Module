#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define OUT 1       // GPIO 핀 방향을 출력(OUT)으로 설정
#define HIGH 1      // GPIO 핀의 출력값을 HIGH(1)로 설정
#define BUZZER_PIN 18 // 부저가 연결된 GPIO 핀 번호
#define VALUE_MAX 40 // 파일 경로 버퍼 최대 크기
#define DIRECTION_MAX 128 // 방향 설정 파일 경로 버퍼 최대 크기

// GPIO 핀을 활성화하는 함수
static int GPIOExport(int pin) {
#define BUFFER_MAX 3 // GPIO 번호를 저장할 버퍼 크기
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    // /sys/class/gpio/export 파일을 열어 GPIO 핀을 활성화
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "GPIO 활성화(export) 파일 열기 실패!\n");
        return -1;
    }

    // GPIO 번호를 문자열로 변환하여 export 파일에 씀
    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd); // 파일 닫기
    return 0;  // 성공적으로 완료
}

// GPIO 핀의 방향(입력/출력)을 설정하는 함수
static int GPIODirection(int pin, int dir) {
    static const char s_directions_str[] = "in\0out"; // 방향 설정 문자열 (in 또는 out)

    char path[DIRECTION_MAX]; // 방향 설정 파일 경로를 저장할 버퍼
    int fd;

    // 방향 설정 파일 경로 생성: /sys/class/gpio/gpio<번호>/direction
    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY); // 방향 설정 파일 열기
    if (-1 == fd) {
        fprintf(stderr, "GPIO 방향 설정 파일 열기 실패!\n");
        return -1;
    }

    // 방향(in 또는 out)을 설정
    if (-1 == write(fd, &s_directions_str[OUT == dir ? 3 : 0], OUT == dir ? 3 : 2)) {
        fprintf(stderr, "GPIO 방향 설정 실패!\n");
        return -1;
    }

    close(fd); // 파일 닫기
    return 0;  // 성공적으로 완료
}

// GPIO 핀의 값을 설정하는 함수 (HIGH 또는 LOW)
static int GPIOWrite(int pin, int value) {
    static const char s_values_str[] = "01"; // 값 설정 문자열 (0 또는 1)

    char path[VALUE_MAX]; // 값 설정 파일 경로를 저장할 버퍼
    int fd;

    // 값 설정 파일 경로 생성: /sys/class/gpio/gpio<번호>/value
    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY); // 값 설정 파일 열기
    if (-1 == fd) {
        fprintf(stderr, "GPIO 값 설정 파일 열기 실패!\n");
        return -1;
    }

    // 값(HIGH 또는 LOW)을 설정
    if (1 != write(fd, &s_values_str[HIGH == value ? 1 : 0], 1)) {
        fprintf(stderr, "GPIO 값 쓰기 실패!\n");
        return -1;
    }

    close(fd); // 파일 닫기
    return 0;  // 성공적으로 완료
}

// GPIO 핀을 비활성화하는 함수
static int GPIOUnexport(int pin) {
    char buffer[BUFFER_MAX]; // GPIO 번호를 저장할 버퍼
    ssize_t bytes_written;
    int fd;

    // /sys/class/gpio/unexport 파일을 열어 GPIO 핀을 비활성화
    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "GPIO 비활성화(unexport) 파일 열기 실패!\n");
        return -1;
    }

    // GPIO 번호를 문자열로 변환하여 unexport 파일에 씀
    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd); // 파일 닫기
    return 0;  // 성공적으로 완료
}

// 메인 함수: 부저를 켜는 프로그램
int main() {
    // 1. BUZZER_PIN 활성화
    if (GPIOExport(BUZZER_PIN) == -1) return 1;

    // 2. BUZZER_PIN을 출력(OUT)으로 설정
    if (GPIODirection(BUZZER_PIN, OUT) == -1) return 2;

    // 3. 부저를 켜기 (GPIO 핀 값을 HIGH로 설정)
    if (GPIOWrite(BUZZER_PIN, HIGH) == -1) return 3;

    printf("부저가 울리고 있습니다.\n");

    // 4. 5초 동안 부저를 유지
    sleep(5);

    // 5. 부저를 끄기 (GPIO 핀 값을 LOW로 설정)
    if (GPIOWrite(BUZZER_PIN, 0) == -1) return 4;

    // 6. BUZZER_PIN 비활성화
    if (GPIOUnexport(BUZZER_PIN) == -1) return 5;

    return 0; // 프로그램 정상 종료
}
