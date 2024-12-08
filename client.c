#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "shared.h"

// signal_status를 LOW(OFF) 상태로 초기화
int signal_status = LOW; 
// signal_status 접근 동기화를 위한 mutex 초기화
pthread_mutex_t signal_mutex = PTHREAD_MUTEX_INITIALIZER; 

void* led_thread(void* arg);
void* buzzer_thread(void* arg);

// signal_status를 갱신하는 함수 (shared.h에 선언됨)
void update_signal_status(int new_status) {
    signal_status = new_status;
}

int main(int argc, char* argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    char msg[2];// 서버로부터 'on' 또는 'off' 신호 수신용 버퍼
    pthread_t led_tid, buzzer_tid;

    // 실행 시 인자로 서버 IP와 포트를 받는다. 인자 확인
    if (argc != 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return -1;
    }
    // 1. 클라이언트 소켓 생성
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("[Client] socket() error");
        return -1;
    }
    // 2. 서버 주소 구조체 설정
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;// IPv4 사용
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);// 명령행 인자로 받은 서버 IP
    serv_addr.sin_port = htons(atoi(argv[2]));// 명령행 인자로 받은 포트 번호(정수->네트워크 바이트 오더)

    // 3. 서버에 연결 요청
    // connect() 함수를 통해 지정한 IP와 포트로 TCP 연결 시도
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("[Client] connect() error");
        close(sock);
        return -1;
    }

    printf("[Client] Connected to server\n");

    // 4. LED 스레드와 Buzzer 스레드 생성
    //   각각 led_thread, buzzer_thread 함수에서 LED와 부저 상태를 계속 모니터링하고,
    //   signal_status에 따라 ON/OFF를 제어한다.
    pthread_create(&led_tid, NULL, led_thread, NULL);
    pthread_create(&buzzer_tid, NULL, buzzer_thread, NULL);

    // 5. 서버로부터 신호 수신 루프
    //   서버는 'on\n' 이나 'off\n' 같은 신호를 보내온다.
    //   클라이언트는 해당 신호를 읽어서 signal_status를 갱신하고,
    //   LED나 부저 스레드가 이 상태를 보고 동작을 수행한다.
    while (1) {
        int str_len = read(sock, msg, sizeof(msg) - 1);
        if (str_len == -1) {
            perror("[Client] read() error");
            close(sock);
            return -1;
        }
        // 수신한 문자열 끝에 널 문자를 추가해 C 문자열로 완성
        msg[str_len] = '\0';

        // signal_status 갱신을 위해 mutex 잠금
        pthread_mutex_lock(&signal_mutex);
        if (strcmp(msg,"off") == 0)
        {
            // 서버로부터 'off' 신호 수신 시 상태를 LOW로
            update_signal_status(LOW);
        }
        else
        {
            // 그 외(기본적으로 'on'일 경우) 상태를 HIGH로
            update_signal_status(HIGH);
        }
        pthread_mutex_unlock(&signal_mutex);

        printf("[Client] Received signal: %s\n", msg);
    }
    // 종료 시 스레드 정리 및 소켓 닫기
    pthread_join(led_tid, NULL);
    pthread_join(buzzer_tid, NULL);
    close(sock);
    return 0;
}
