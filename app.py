from flask import Flask, jsonify, render_template, request
import ctypes
import os
from shutil import copyfile

# # C 라이브러리 불러오기
# example = ctypes.CDLL('./example.so')

# # C 함수 호출
# result = example.add(10, 20)
# print("C 함수 결과:", result)

app = Flask(__name__)

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

# 사진 데이터를 제공하는 API
@app.route("/api/photos")
def get_photos():
    
    photos = []
    photo_directory = "/home/pi/Pictures/"
    for filename in os.listdir(photo_directory):
        if filename.endswith(".jpg"):
            try:
                # 파일 이름에서 날짜와 시간 추출
                # 예: capture_20241205_173443.jpg
                parts = filename.split("_")  # ['capture', '20241205', '173443.jpg']
                date_part = parts[1]  # '20241205'
                time_part = parts[2].split(".")[0]  # '173443'

                # 날짜 및 시간 포맷
                date_str = f"{date_part[:4]}-{date_part[4:6]}-{date_part[6:]}"  # '2024-12-05'
                time_str = f"{time_part[:2]}:{time_part[2:4]}:{time_part[4:]}"  # '17:34:43'

                photos.append({
                    "filePath": f"/static/{filename}",
                    "date": date_str,
                    "time": time_str
                })
            except (IndexError, ValueError) as e:
                # 파일명 파싱 실패 시 로그 출력
                print(f"Failed to parse filename: {filename}. Error: {e}")
    
    return jsonify(photos)

# Safe System 상태 관리
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

    # 상태에 따라 state.txt 파일 초기화 및 기록
    state_value = "1" if safe_system_state == "on" else "0"
    with open("state.txt", "w") as state_file:
        state_file.truncate(0)  # 기존 내용 초기화
        state_file.write(state_value)

    return jsonify({"success": True, "message": f"Safe system toggled to {safe_system_state}"})

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
    app.run(debug=True, port=80, host='0.0.0.0')
