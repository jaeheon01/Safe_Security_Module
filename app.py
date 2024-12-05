from flask import Flask, jsonify, render_template, request
import ctypes
import os
import shutil
from shutil import copyfile
import RPi.GPIO as GPIO

# # C 라이브러리 불러오기
# example = ctypes.CDLL('./example.so')

# # C 함수 호출
# result = example.add(10, 20)
# print("C 함수 결과:", result)

app = Flask(__name__)

# GPIO 핀 설정
SENSOR_PIN = 17
GPIO.setmode(GPIO.BCM)
GPIO.setup(SENSOR_PIN, GPIO.OUT)


# 로그인 페이지
@app.route('/')
def login():
    return render_template('login.html')

# 대시보드 페이지
@app.route('/dashboard.html')
def dashboard():
    return render_template('dashboard.html')

# 회원가입 페이지
@app.route('/register.html')
def register():
    return render_template('register.html')

# 로그아웃 페이지
@app.route('/login.html')
def logout():
    return render_template('login.html')

# 사진을 /static/photos/ 폴더로 복사하는 함수
def copy_photos():
    photo_directory = "/home/pi/Pictures"
    static_photo_directory = "static/photos"
    if not os.path.exists(static_photo_directory):
        os.makedirs(static_photo_directory)

    # 디렉토리 내의 파일들을 /static/photos/로 복사
    for filename in os.listdir(photo_directory):
        if filename.endswith(".jpg"):
            src_path = os.path.join(photo_directory, filename)
            dest_path = os.path.join(static_photo_directory, filename)
            shutil.copy(src_path, dest_path)

# 사진 데이터를 제공하는 API
@app.route("/api/photos")
def get_photos():
    photos = []
    copy_photos()  # 사진을 /static/photos/ 폴더로 복사

    # /static/photos/ 폴더에서 사진 파일 목록 가져오기
    static_photo_directory = "static/photos"
    for filename in os.listdir(static_photo_directory):
        if filename.endswith(".jpg"):
            # 파일 이름에서 날짜와 시간 추출
            timestamp = filename.split("_")[1].split(".")[0]
            date_str = f"{timestamp[:4]}-{timestamp[4:6]}-{timestamp[6:8]}"
            time_str = f"{timestamp[9:11]}:{timestamp[11:13]}:{timestamp[13:15]}"
            photos.append({
                "filePath": f"/static/photos/{filename}",  # static 폴더 경로
                "date": date_str,
                "time": time_str
            })

    return jsonify(photos)

# Safe System 상태 관리
safe_system_state = "off"  # 초기 상태: off

# @app.route('/toggle_sensor', methods=['POST'])
# def toggle_sensor():
#     global safe_system_state
#     data = request.json
#     new_state = data.get("state")

#     if new_state not in ["on", "off"]:
#         return jsonify({"success": False, "error": "Invalid state"}), 400

#     # 상태 변경
#     safe_system_state = new_state

#     # 상태에 따라 state.txt 파일 초기화 및 기록
#     state_value = "1" if safe_system_state == "on" else "0"
#     with open("state.txt", "w") as state_file:
#         state_file.truncate(0)  # 기존 내용 초기화
#         state_file.write(state_value)

#     return jsonify({"success": True, "message": f"Safe system toggled to {safe_system_state}"})


@app.route('/toggle_sensor', methods=['POST'])
def toggle_sensor():
    data = request.json
    state = data.get('state')
    if state == "on":
        GPIO.output(SENSOR_PIN, GPIO.HIGH)
        return jsonify({"message": "Sensor turned ON"}), 200
    elif state == "off":
        GPIO.output(SENSOR_PIN, GPIO.LOW)
        return jsonify({"message": "Sensor turned OFF"}), 200
    else:
        return jsonify({"error": "Invalid state"}), 400

# Alarm 상태 관리
alarm_state = "safe"  # 초기 상태: safe

@app.route('/get_alarm_status', methods=['GET'])
def get_alarm_status():
    return jsonify({"state": alarm_state})

@app.route('/toggle_alarm', methods=['POST'])
def toggle_alarm():
    global alarm_state
    data = request.json
    state = data.get("state", "off")

    if state == "on":
        alarm_state = "alert"  # 경보 활성화
    else:
        alarm_state = "safe"  # 경보 비활성화

    return jsonify({"alarmState": alarm_state})

# 상태 파일 읽기 API
@app.route('/get_state', methods=['GET'])
def get_state():
    try:
        with open("state.txt", "r") as state_file:
            state_value = state_file.read().strip()
        return jsonify({"state": state_value})
    except FileNotFoundError:
        return jsonify({"error": "State file not found"}), 404

if __name__ == "__main__":
    try:
        app.run(debug=True, port=80, host='0.0.0.0')
    except KeyboardInterrupt:
        GPIO.cleanup()
