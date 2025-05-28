#ifndef SHARED_H
#define SHARED_H

#include <pthread.h>

// 모션 감지 상태를 저장하는 외부 변수
extern int motion_detected;
// 진동 감지 상태를 저장하는 외부 변수
extern int vibration_detected;
// 시템에서 특정 신호 상태를 관리하는 외부 변수
extern int signal_status;

// 동기화를 위해 사용되는 뮤텍스(lock)는 공유 자원 접근 시 상호 배제를 보장하기 위해 사용
extern pthread_mutex_t lock;
// signal_status 변수를 안전하게 접근하기 위한 별도의 뮤텍스
extern pthread_mutex_t signal_mutex;
// 특정 조건을 만족할 때 대기 중인 스레드를 깨우기 위해 사용되는 조건 변수
extern pthread_cond_t cond;
// 현재 시스템 단계, 상태
extern int step;

// GPIO 핀 출력을 위한 상수 정의
#define HIGH 1
#define LOW 0
#define OUT 1

// GPIO 관리 함수
// 해당 핀 번호를 export하여 접근 가능하게 함
int GPIOExport(int pin);
// 해당 핀의 입출력 방향 설정
int GPIODirection(int pin, int dir);
// 특정 핀에 HIGH 또는 LOW 신호 출력
int GPIOWrite(int pin, int value);
// 사용이 끝난 핀을 해제
int GPIOUnexport(int pin);

// sensor function
void* motion_sensor(void* arg);
void* vibration_sensor(void* arg);

// actuator function
void* buzzer_thread(void* arg);
void* led_thread(void* arg);

void update_signal_status(int new_status);
// LED control function
int led_init();
int led_on();
int led_off();
int led_cleanup();

// BUZZER control function
int buzzer_init();
int buzzer_on();
int buzzer_off();
int buzzer_cleanup();

#endif
