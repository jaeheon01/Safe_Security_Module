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

// 부저(GPIO PIN)을 사용하기 위한 초기 설정
// 1) GPIOExport로 부저핀을 export하여 sysfs 인터페이스로 접근 가능하게 만들고
// 2) GPIODirection을 통해 해당 핀을 출력 모드로 설정한다.
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
// 부저 켜기 (해당 핀에 HIGH 신호 출력)
int buzzer_on() {
    return GPIOWrite(BUZZER_PIN, HIGH);
}
// 부저 끄기 (해당 핀에 LOW 신호 출력)
int buzzer_off() {
    return GPIOWrite(BUZZER_PIN, LOW);
}
// 사용한 부저 핀을 unexport하여 GPIO 리소스를 반환
int buzzer_cleanup() {
    return GPIOUnexport(BUZZER_PIN);
}
// 부저를 제어하는 스레드 함수
void* buzzer_thread(void* arg) {
    buzzer_init();
    while (1) {
        // signal_status를 안전하게 확인하기 위해 뮤텍스 lock
        pthread_mutex_lock(&signal_mutex);
        // signal_status가 HIGH인 경우 부저를 켜서 경고음을 낸다.
        if (signal_status == HIGH) {
            printf("[BUZZER Thread] Turning BUZZER ON.\n");
            buzzer_on();

            pthread_mutex_unlock(&signal_mutex);
            sleep(5); // 5초 동안 BUZZER 유지

            pthread_mutex_lock(&signal_mutex);
            printf("[BUZZER Thread] Turning BUZZER OFF.\n");
            buzzer_off();
            signal_status = LOW; // 상태 초기화
        }
        pthread_mutex_unlock(&signal_mutex);
        usleep(100 * 1000); // 100ms 대기
    }
    buzzer_cleanup();
    pthread_exit(NULL);
}
