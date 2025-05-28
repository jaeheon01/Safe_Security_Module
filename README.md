# 금고 보안 시스템 프로젝스

## OS
- Raspberry Pi OS (Raspbian)
## GPIO
- GPIO 핀을 이용하여 진동 센서, 모션 센서, LED, 부저와 통신
- 사용된 핀 :
  - 진동 센서 : GPIO 17
  - 모션 센서 : GPIO 22
  - LED : GPIO 23
  - 부저 : GPIO 16
## 코드 구성
### 주요 파일 설명
- **web**
  - 방범 시스템을 원격으로 조종하며 활성화/비활성화 설정 가능
  - 실시간으로 캡쳐사진 업데이트
- **server.c**
  - 진동 센서와 모션 센서를 통해 침입 여부를 감지하고, 감지 결과를 클라이언트 Pi로 전송
  - 카메라를 통해 침입 시 이미지 캡처
- **client.c**
  - 서버로부터 침입 신호를 받아 부저와 LED 제어
- **vibration.c**
  - 진동 센서에서 신호를 읽고 서버에 전달
- **motion.c**
  - 모션 센서에서 신호를 읽고 서버에 전달
- **led.c**
  - LED를 켜고 끄는 동작을 제어
- **buzzer.c**
  - 부저를 켜고 끄는 동작을 제어
- **gpio.c**
  - GPIO 핀의 입력/출력 설정 및 제어 관련 함수 정의
- **shared.h**
  - 쓰레드 간 공유되는 변수와 함수의 선언
### 디렉토리 구조
```bash
system_programming_project/
├── web
├── buzzer.c
├── camera.c
├── camera.h
├── client.c
├── gpio.c
├── led.c
├── motion.c
├── README.md
├── server.c
├── shared.h
└── vibration.c
system_programming_project/web
├── app.py
├── templates/
├── dashboard.html
├── login.html
├── register.html
├── static/
├── auth.css
├── dashboard.css
├── dashboard.js
├── login_client.js
└── register_client.js
```

## 시스템 구성
1. 진동 센서 : 금고에 물리적 충격이 감지되면 신호를 생성
2. 모션 센서 : 금고 주변의 움직임을 감지
3. LED 및 부저 : 침입이 감지되었을 경우 경고 표시, 원활한 촬영을 위해 LED 점등
4. Server Pi : 센서를 모니터링하고 신호를 처리
5. Client Pi : 서버로부터 신호를 받아 부저와 LED 제어

## Activity diagram

<img width="224" alt="플로우차트" src="https://github.com/user-attachments/assets/b14095a0-18e5-4b9b-a3fe-64216a017222">

