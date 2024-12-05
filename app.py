from flask import Flask, jsonify, render_template, request, send_from_directory
import os
import shutil

app = Flask(__name__)
SOURCE_FOLDER = '/home/pi/Desktop/web_app/system_programming_project-6v6DS/'  # C 코드가 이미지를 저장하는 폴더
TARGET_FOLDER = 'static/photos/'  # Flask가 웹에서 사용하는 폴더

safe_system_state = "off"  # 초기 Safe System 상태
alarm_state = "safe"  # 초기 Alarm 상태

# Ensure the target folder exists
if not os.path.exists(TARGET_FOLDER):
    os.makedirs(TARGET_FOLDER)

@app.route('/')
def login():
    return render_template('login.html')

@app.route('/dashboard.html')
def dashboard():
    return render_template('dashboard.html')

@app.route('/register.html')
def register():
    return render_template('register.html')

@app.route("/api/photos")
def get_photos():
    photos = []

    # Ensure SOURCE_FOLDER exists
    if not os.path.exists(SOURCE_FOLDER):
        return jsonify(photos)  # No photos available

    # Copy new images from SOURCE_FOLDER to TARGET_FOLDER
    for filename in os.listdir(SOURCE_FOLDER):
        if filename.endswith(".jpg"):
            source_path = os.path.join(SOURCE_FOLDER, filename)
            target_path = os.path.join(TARGET_FOLDER, filename)

            # Copy the file if it doesn't already exist in TARGET_FOLDER
            if not os.path.exists(target_path):
                shutil.copy2(source_path, target_path)

            # Extract date and time from filename
            parts = filename.split("_")
            date_part = parts[1]
            time_part = parts[2].split(".")[0]

            # Format date and time
            date_str = f"{date_part[:4]}-{date_part[4:6]}-{date_part[6:]}"
            time_str = f"{time_part[:2]}:{time_part[2:4]}:{time_part[4:]}"

            photos.append({
                "filePath": f"/static/photos/{filename}",
                "date": date_str,
                "time": time_str
            })

    return jsonify(photos)

@app.route('/toggle_sensor', methods=['POST'])
def toggle_sensor():
    global safe_system_state
    data = request.json
    new_state = data.get("state")

    if new_state not in ["on", "off"]:
        return jsonify({"success": False, "error": "Invalid state"}), 400

    safe_system_state = new_state
    return jsonify({"success": True, "message": f"Safe system toggled to {safe_system_state}"})

@app.route('/get_alarm_status', methods=['GET'])
def get_alarm_status():
    return jsonify({"state": alarm_state})

@app.route('/toggle_alarm', methods=['POST'])
def toggle_alarm():
    global alarm_state
    data = request.json
    new_state = data.get("state")

    if new_state not in ["safe", "alert"]:
        return jsonify({"success": False, "error": "Invalid state"}), 400

    alarm_state = new_state
    return jsonify({"success": True, "message": f"Alarm state changed to {alarm_state}"})

@app.route('/static/<path:filename>')
def serve_static(filename):
    return send_from_directory('static', filename)

if __name__ == "__main__":
    app.run(debug=True, port=80, host='0.0.0.0')

