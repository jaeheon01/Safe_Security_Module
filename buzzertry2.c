void* buzzer_thread(void* arg) {
    buzzer_init();
    while (1) {
        pthread_mutex_lock(&signal_mutex);

        if (signal_status == HIGH) {
            printf("[BUZZER Thread] Turning BUZZER ON.\n");
            buzzer_on();

            // 7초 동안 대기
            pthread_mutex_unlock(&signal_mutex);
            sleep(7); // 7초 대기
            
            // 7초 후 부저 끄기
            printf("[BUZZER Thread] 7 seconds passed. Turning BUZZER OFF.\n");
            buzzer_off();

            // 부저가 꺼진 후 signal_status를 LOW로 설정
            pthread_mutex_lock(&signal_mutex);
            signal_status = LOW;
        } else if (signal_status == LOW) {
            // 부저가 꺼진 상태 유지
            printf("[BUZZER Thread] BUZZER is OFF.\n");
            buzzer_off();
        }

        pthread_mutex_unlock(&signal_mutex);
        usleep(500 * 1000); // 500ms 대기
    }

    buzzer_cleanup();
    pthread_exit(NULL);
}
