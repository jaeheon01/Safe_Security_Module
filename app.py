from flask import Flask, jsonify, render_template, request, redirect, url_for
import os
from datetime import datetime

##########
# c파일 불러오기
import ctypes

# 라이브러리 불러오기
example = ctypes.CDLL('./example.so')

# C 함수 호출
result = example.add(10, 20)

# 결과 출력
print("결과:", result)
############

app = Flask(__name__)

# 로그인 페이지
@app.route('/')
def login():
    return render_template('login.html')

# 대시보드 페이지
@app.route('/dashboard.html')
def dashboard():
    return render_template('dashboard.html')

#회원가입
@app.route('/register.html')
def register():
    return render_template('register.html')

#로그아웃
@app.route('/login.html')
def logout():
    return render_template('login.html')


# 사진 데이터를 제공하는 API
@app.route("/api/photos")
def get_photos():
    #예시 사진 3개
    photos = [
        {
            "filePath": "/static/photo_01.jpg",
            "date": "2024-11-27",
            "time": "23:57:01"
        },
        {
            "filePath": "/static/photo_02.jpg",
            "date": "2024-11-20",
            "time": "23:57:01"
        },
        {
            "filePath": "/static/photo_03.jpg",
            "date": "2024-11-15",
            "time": "23:57:01"
        }
    ]

    # photos = []
    # photo_directory = "/home/pi/Pictures"
    # for filename in os.listdir(photo_directory):
    #     if filename.endswith(".jpg"):
    #         # 파일 이름에서 날짜와 시간 추출
    #         timestamp = filename.split("_")[1].split(".")[0]
    #         date_str = f"{timestamp[:4]}-{timestamp[4:6]}-{timestamp[6:8]}"
    #         time_str = f"{timestamp[9:11]}:{timestamp[11:13]}:{timestamp[13:15]}"
    #         photos.append({
    #             "filePath": f"/static/{filename}",
    #             "date": date_str,
    #             "time": time_str
    #         })

    return jsonify(photos)
# Safe System 상태
safe_system_state = "off"  # 초기 상태: off

@app.route('/toggle_sensor', methods=['POST'])
def toggle_sensor():
    global safe_system_state
    data = request.json
    new_state = data.get("state")

    if new_state not in ["on", "off"]:
        return jsonify({"success": False, "error": "Invalid state"}), 400

    # 상태 변경
    safe_system_state = new_state
    return jsonify({"success": True, "message": f"Safe system toggled to {safe_system_state}"})


# 초기 Alarm 상태
alarm_state = "safe"  # "safe" or "alert"

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

if __name__ == "__main__":
    app.run(debug=True, port=80, host='0.0.0.0')
