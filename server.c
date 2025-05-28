#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "shared.h"
#include "camera.h"

// 침입 감지 관련 전역 변수 초기화
int vibration_detected = 0; 
int motion_detected = 0;
// 스레드 동기화를 위한 Mutex, cond 초기화
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
// 단계 제어 변수(센서 순서에 맞춰 단계별로 동작하기 위함)
int step = 0;
// 클라이언트에게 알림 메세지 전송 함수
void send_alert_to_client(int clnt_sock) {
    char alert_msg[] = "on\n";

    // 클라이언트 소켓에 알림 메세지 전송
    if (write(clnt_sock, alert_msg, strlen(alert_msg)) == -1) {
        perror("[Server] Failed to send alert");
    } else {
        printf("[Server] Alert sent to client: %s\n", alert_msg);
    }
}


int main(int argc, char *argv[]) {
    pthread_t vibration_thread, motion_thread;
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    // 포트 번호 확인
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return -1;
    }
    /*
    - socket() 함수를 통해 서버용 소켓 생성
    - bind() 함수를 통해 서버 소켓에 IP 주소와 포트 번호 할당
    - listen() 함수로 해당 소켓을 클라이언트 연결 대기 상태로 만듬
    - accept() 함수를 사용하여 클라이언트의 연결 요청을 수락함.
    - 연결 성공 시 클라이언트와 통신할 수 있는 전용 소켓(클라이언트 소켓)을 얻음.
    */
    // 1. 소켓 생성(서버 소켓)
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1) {
        perror("[Server] socket() error");
        return -1;
    }

    // 2. 서버 주소 구조체 초기화 및 설정
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; //IPv4 사용
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 모든 인터페이스(IP)에서 접속 수용
    serv_addr.sin_port = htons(atoi(argv[1])); // 입력받은 포트를 네트워크 바이트 오더로 변환

    // 3. 생성한 소켓을 서버 주소와 bind
    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("[Server] bind() error");
        close(serv_sock);
        return -1;
    }

    // 4. 클라이언트 접속 요청 대기 상태
    if (listen(serv_sock, 5) == -1) {
        perror("[Server] listen() error");
        close(serv_sock);
        return -1;
    }

    // 5. 클라이언트 접속 수락
    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1) {
        perror("[Server] accept() error");
        close(serv_sock);
        return -1;
    }
    printf("[Server] Client connected\n");

    // 6. 진동 센서와 모션 센서 스레드 생성
    if (pthread_create(&vibration_thread, NULL, vibration_sensor, NULL) != 0) {
        fprintf(stderr, "Error creating vibration sensor thread\n");
        close(serv_sock);
        close(clnt_sock);
        return -1;
    }

    if (pthread_create(&motion_thread, NULL, motion_sensor, NULL) != 0) {
        fprintf(stderr, "Error creating motion sensor thread\n");
        close(serv_sock);
        close(clnt_sock);
        return -1;
    }
    // 메인 루프 : 센서값 확인 후 클라이언트에 알림 전송 및 카메라 촬영 제어
    while (1) {
        pthread_mutex_lock(&lock);
        // step이 2가 될 때까지 대기 (모션 -> 진동 순서로 완료되어야 step=2)
        while (step != 2) {
            pthread_cond_wait(&cond, &lock);
        }
        // 두 센서 모두 침입 감지 시
        if (vibration_detected && motion_detected) {
            printf("[Main] Invade detected by both sensors!\n");
            send_alert_to_client(clnt_sock); // 클라이언트에 알림 전송
            captureImage(); // Image capturing
        } else {
            printf("[Main] No invade detected!\n");
        }
        // 다음 라운드 감지를 위해 값 초기화 및 step을 0으로 돌림
        vibration_detected = 0;
        motion_detected = 0;
        step = 0;
        pthread_cond_broadcast(&cond); // 초기화 후 대기중인 센서 스레드 깨움
        pthread_mutex_unlock(&lock);
    }
    // 스레드 종료 대기 및 리소스 정리
    pthread_join(vibration_thread, NULL);
    pthread_join(motion_thread, NULL);
    close(clnt_sock);
    close(serv_sock);
    return 0;
}
