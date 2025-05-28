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

// LED GPIO 핀 초기화 함수
// 1) GPIOExport를 통해 LED_PIN을 sysfs에서 사용 가능하게 하고
// 2) GPIODirection을 통해 해당 핀을 출력(OUT) 모드로 설정한다.
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
// LED 켜기 함수 (해당 핀에 HIGH 값 쓰기)
int led_on() {
    return GPIOWrite(LED_PIN, HIGH);
}
// LED 끄기 함수 (해당 핀에 LOW 값 쓰기)
int led_off() {
    return GPIOWrite(LED_PIN, LOW);
}
// 사용한 LED 핀에 대한 GPIO unexport 함수 (리소스 정리)
int led_cleanup() {
    return GPIOUnexport(LED_PIN);
}

// LED를 제어하는 스레드 함수
void* led_thread(void* arg) {
    led_init();
    while (1) {
        /*
        pthread_mutex_lock(&signal_mutex) 함수를 호출하면 signal_mutex 라는 뮤텍스 획득
        만약 다른 스레드가 이미 뮤텍스를 소유하고 있다면, 해당 뮤텍스가 해제될 때까지 대기함.
        이를 통해 한 번에 한 스레드만 공유 변수(signal_status)에 접근하도록 함
        */
        pthread_mutex_lock(&signal_mutex);
        // signal_mutex를 획득해 signal_status를 확인
        // signal_status가 HIGH(LED 켜야 하는 상황)인지 확인
        if (signal_status == HIGH) {
            printf("[LED Thread] Turning LED ON.\n");
            led_on();// LED 켜기

            // 잠금 해제 후 5초동안 LED 켠 상태 유지
            pthread_mutex_unlock(&signal_mutex);
            sleep(5); // 5초 동안 LED 유지
            // 다시 mutex 잠금 후 LED를 끄고 signal_status 리셋
            pthread_mutex_lock(&signal_mutex);
            printf("[LED Thread] Turning LED OFF.\n");
            led_off();
            signal_status = LOW; // LED 상태 초기화
        }
        // 100ms 대기 후 다시 signal_status 확인을 위해 루프 반복
        pthread_mutex_unlock(&signal_mutex);
        usleep(100 * 1000); // 100ms 대기
    }
    led_cleanup();// 스레드 종료 전 LED 리소스 정리
    pthread_exit(NULL);
}
