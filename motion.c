#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "shared.h"

#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define PIR_PIN 22
#define VALUE_MAX 40
#define DIRECTION_MAX 128


static int GPIOexport(int pin) {
#define BUFFER_MAX 3
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;
	// sys/class/gpio/export 파일에 핀 번호를 써서 해당 핀 접근
	fd = open("/sys/class/gpio/export", O_WRONLY);
	if(-1 == fd) {
		fprintf(stderr, "Failed to open for writing!\n");
		return -1;
	}
	
	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return 0;
}
// GPIO 핀 방향(IN/OUT) 설정 함수
static int GPIOdirection (int pin, int dir){
	static const char s_directions_str[]="int\0out";
	char path[DIRECTION_MAX];
	int fd;
	// direction 파일에 'in' 또는 'out' 문자열을 써서 방향 설정
	snprintf(path,DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if(-1 == fd) {
		fprintf(stderr, "Failed to open gpio direction for writing!\n");
		return -1;
	}
	if (-1 == 
		write(fd, &s_directions_str[IN == dir ? 0 : 3], IN ==dir ? 2 : 3)) {
			fprintf(stderr, "Failed to set direction!\n");
			return -1;
	}
	
	close(fd);
	return 0;
}
// GPIO 핀 값 읽기 함수(현재 핀이 HIGH인지 LOW인지)
static int GPIOread(int pin){
	char path[DIRECTION_MAX];
	char value_str[3];
	int fd;
	
	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value",pin);
	fd = open(path, O_RDONLY);
	if(fd == -1){
		perror("Error: GPIO read");
		return -1;
	}
	
	read(fd, value_str,3);
	close(fd);

	return(value_str[0] == '1') ? HIGH : LOW;

}

// GPIO 핀에 값을 쓰는 함수(HIGH/LOW 출력)
static int GPIOwrite(int pin, int value){
	static const char s_values_str[] = "01";
	
	char path[VALUE_MAX];
	int fd;
	
	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path,O_WRONLY);
	if(-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for writing!\n");
		return -1;
	}
	
	if(1 != write(fd, &s_values_str[LOW == value ? 0: 1],1)) {
		fprintf(stderr, "Failed to write value!\n");
		return -1;
	}
	close(fd);
	return 0;
}
// 사용 후 GPIO pin을 unexport
static int GPIOunexport(int pin){
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;
	
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if(-1 == fd){
		fprintf(stderr, "Failed to open unexport for writing!\n");
		return -1;
	}
	
	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return 0;
}
// 모션 센서 스레드
void* motion_sensor(void* arg) {
	// PIR 센서 핀을 export 및 방향을 입력 모드로 설정
	if (GPIOexport(PIR_PIN) == -1) printf("Error_Exprot");
	if (GPIOdirection(PIR_PIN, IN) == -1) printf("Error_Direction");
	
	while(1) {
		// lock을 통해 공유 자원 접근 동기화
		/*
		공유 자원 (motion_detected, step)에 접근할 때 동시성 문제를 방지하기 위해 사용되는 mutex lock
		여러 스레드가 동시에 해당 변수에 접근하려 할 때, 뮤텍스를 통해 한 번에 하나의 스레드만 접근하도록 하여 데이터 불일치 방지
		*/
		pthread_mutex_lock(&lock);
		// step이 0이 아닐 경우 cond wait를 통해 조건 대기
		while(step!=0){
			/*
			조건변수(pthread_cond_t)를 사용하여 특정 조건이 충족될 때까지 스레드를 대기, 깨움
			예를 들어, step 변수가 특정 값이 될 때까지 모션 센서 스레드는 pthread_cond_wait로 대기 상태에 들어가고
			해당 조건이 충족되면 pthread_cond_broadcast를 통해 대기 중인 스레드를 깨워서 작업 진행.
			이를 통해 스레드 간 협업과 순서 제어 가능
			*/
			pthread_cond_wait(&cond,&lock);
		}
		printf("[Motion Sensor] Checking for motion...\n");
		usleep(200*1000); // 200ms 대기 후 센서값 읽기
		if(GPIOread(PIR_PIN) == HIGH){
			motion_detected = 1;
			printf("[Motion Sensor] Motion detected!\n");
		}else{
			motion_detected = 0;
			printf("[Motion Sensor] No Motion detected\n");
		}
		// 다음 단계로 진행하기 위해 step 조정 및 조건 변수 신호 전달
		step=1;
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&lock);
	}
	// 사용 종료 시 핀 Unexport
	if(GPIOunexport(PIR_PIN) == -1){ 
		printf("Error_Unexport");
	}
}

