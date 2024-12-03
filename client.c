#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "shared.h"

int signal_status = LOW; // 신호 상태 변수
pthread_mutex_t signal_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex 선언

void* led_thread(void* arg);
void* buzzer_thread(void* arg);

void update_signal_status(int new_status) {
    pthread_mutex_lock(&signal_mutex); // Mutex 잠금
    signal_status = new_status;
    pthread_mutex_unlock(&signal_mutex); // Mutex 해제
}

int main(int argc, char* argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    char msg[2];
    pthread_t led_tid, buzzer_tid;

    // 서버와 연결 설정
    if (argc != 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return -1;
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("[Client] socket() error");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("[Client] connect() error");
        close(sock);
        return -1;
    }

    printf("[Client] Connected to server\n");

    // LED 및 BUZZER 쓰레드 생성
    pthread_create(&led_tid, NULL, led_thread, NULL);
    pthread_create(&buzzer_tid, NULL, buzzer_thread, NULL);

    while (1) {
        int str_len = read(sock, msg, sizeof(msg) - 1);
        if (str_len == -1) {
            perror("[Client] read() error");
            close(sock);
            return -1;
        }

        msg[str_len] = '\0';

        // 신호 상태 업데이트
        pthread_mutex_lock(&signal_mutex);
        if (strcmp(msg, "on") == 0) {
            update_signal_status(HIGH); // LED 및 BUZZER ON 신호
        } else if (strcmp(msg, "off") == 0) {
            update_signal_status(LOW); // LED 및 BUZZER OFF 신호
        }
        pthread_mutex_unlock(&signal_mutex);

        printf("[Client] Received signal: %s\n", msg);
    }

    pthread_join(led_tid, NULL);
    pthread_join(buzzer_tid, NULL);
    close(sock);
    return 0;
}
