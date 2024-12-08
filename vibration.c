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
#define VIB_PIN 17
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
// GPIO 핀 값 읽기(HIGH or LOW)
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
	
	// if (value_str[0] != '1' && value_str[0] != '0')
	// {
	// 	return LOW;
	// }
	return(value_str[0] == '1') ? HIGH : LOW;
	
}

// GPIO 핀 값 쓰기(HIGH or LOW)
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
// 사용한 GPIO 핀을 unexport하여 시스템 리소스 정리
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
// 진동 센서를 모니터링하는 스레드 함수
void* vibration_sensor(void* arg) {
	if (GPIOexport(VIB_PIN) == -1) printf("Error_Exprot");
	if (GPIOdirection(VIB_PIN, IN) == -1) printf("Error_Direction");
	
	while(1) {
		// 공유 자원 접근 전 lock
		/*
		여러 스레드가 공유하는 자원(여기서는 vibration_detected나 step등의 변수)에 동시에 접근할 경우,
		동기화 문제(데이터 불일치)가 발생할 수 있음. 
		이를 예방하기 위해 뮤텍스를 사용하여 한 번에 한 스레드만 접근.
		pthread_mutex_lock을 통해 lock이 된 스레드는 unlock하기 전까지 독점적으로 해당 자원 사용
		사용이 끝나면 pthread_mutex_unlock을 호출하여 다른 스레드가 접근할 수 있도록 함.
		*/
		pthread_mutex_lock(&lock);
		// step 값이 1이 아닐 경우 조건 대기
		/*
		조건변수(step)을 통해 특정 조건(step값)이 만족될 때까지 스레드를 대기 상태로 만듬.
		이 코드에서 vibration_sensor 스레드는 step값이 1이 될 때까지 대기함.
		-> motion.c코드에서 step을 1로 설정하고 vibration.c 코드로 step1이 전달될 때까지 대기하는 것.
		조건이 만족되면 ptrhead_cond_broadcast를 통해 대기 중인 스레드를 깨워서 진행함.
		이를 통해 스레드 간의 실행 순서를 제어, 특정 로직의 단계적 진행 구현.
		*/
		while(step!=1){
			pthread_cond_wait(&cond,&lock);
		}
		
		printf("[Vibration Sensor] Checking for vibration...\n");
		usleep(200 * 1000);// 200ms 대기 후 센서값 읽기
		if(GPIOread(VIB_PIN) == HIGH){
			vibration_detected = 1;
			printf("[Vibration Sensor] Vibration detected!\n");
		}else{
			vibration_detected = 0;
			printf("[Vibration Sensor] No Vibration detected\n");
		}
		// 다음 단계로 진행(step을 2로)하고 조건 변수로 다른 스레드 깨움
		step=2;
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&lock);
	}
	// 사용한 핀 unexport 처리
	if(GPIOunexport(VIB_PIN) == -1){ 
		printf("Error_Unexport");
	}
}




