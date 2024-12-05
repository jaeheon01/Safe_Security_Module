//로그아웃
document.addEventListener('DOMContentLoaded', function() {
    const logoutButton = document.querySelector('.dashboard-header__date');
    
    if (logoutButton) {
        logoutButton.addEventListener('click', function(e) {
            e.preventDefault();
            window.location.href = 'login.html';
        });
    }
});

function updateSafeStatus(systemState) {
    const safeStatus = document.getElementById("safe-status");
    const safeIcon = document.getElementById("safe-icon");
    const safeText = document.getElementById("safe-text");

    if (systemState === "on") {
        safeStatus.classList.add("status_success");
        safeStatus.classList.remove("status_danger");
        safeIcon.classList.add("fa-check");
        safeIcon.classList.remove("fa-times");
        safeText.textContent = "Activated";
    } else if (systemState === "off") {
        safeStatus.classList.add("status_danger");
        safeStatus.classList.remove("status_success");
        safeIcon.classList.add("fa-times");
        safeIcon.classList.remove("fa-check");
        safeText.textContent = "Disabled";
    }
}
const switch1 = document.getElementById('switch1');

document.getElementById("switch1").addEventListener("change", (event) => {
    const isChecked = event.target.checked;
    const systemState = isChecked ? "on" : "off";

    updateSafeStatus(systemState);

    fetch('/toggle_sensor', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({ state: systemState }),
    })
    .then(response => response.json())
    .then(data => {
        console.log(data.message || data.error);
    })
    .catch(error => {
        console.error('Error:', error);
    });
});

document.addEventListener("DOMContentLoaded", () => {
    const initialState = document.getElementById("switch1").checked ? "on" : "off";
    updateSafeStatus(initialState); // 초기 상태\

    fetchAlarmStatus(); // 초기 상태
});

function updateAlarmStatus(alarmState) {
    const alarmStatus = document.getElementById("alarm-status");
    const alarmIcon = document.getElementById("alarm-icon");
    const alarmText = document.getElementById("alarm-text");

    if (alarmState === "alert") {
        alarmStatus.classList.add("status_danger");
        alarmStatus.classList.remove("status_success");
        alarmIcon.classList.add("fa-times");
        alarmIcon.classList.remove("fa-check");
        alarmText.textContent = "경보가 발생하였습니다";
    } else if (alarmState === "safe") {
        alarmStatus.classList.add("status_success");
        alarmStatus.classList.remove("status_danger");
        alarmIcon.classList.add("fa-check");
        alarmIcon.classList.remove("fa-times");
        alarmText.textContent = "안전한 상태입니다";
    }
}

function fetchAlarmStatus() {
    fetch('/get_alarm_status')
        .then(response => response.json())
        .then(data => {
            const alarmState = data.state; 
            updateAlarmStatus(alarmState);
        })
        .catch(error => {
            console.error('Error fetching alarm status:', error);
        });
}


document.getElementById("switch1").addEventListener("change", (event) => {
    const isChecked = event.target.checked;
    const systemState = isChecked ? "on" : "off";

    fetch('/toggle_sensor', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({ state: systemState }),
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Failed to toggle sensor.');
        }
        return response.json();
    })
    .then(data => {
        if (data.success) {
            updateSafeStatus(systemState); // Safe System 상태 업데이트
        } else {
            console.error('Server error:', data.error);
            alert(data.error || 'Failed to toggle sensor. Please try again.');
        }
    })
    .catch(error => {
        console.error('Error:', error);
        alert('Failed to toggle sensor. Please try again.');
    });
});

// 알람 상태는 별도로 서버에서 주기적으로 가져오도록 유지
document.addEventListener("DOMContentLoaded", () => {
    fetchAlarmStatus(); // 초기 알람 상태를 서버에서 가져옴
});

// 실시간 사진 업데이트
function updatePhotoList(photoData) {
    const photoContainer = document.querySelector(".weather-days");
    // const photoContainer = document.querySelector("#photo-container");

    // 기존 사진 리스트 초기화
    photoContainer.innerHTML = "";

    // 새로 받은 데이터로 사진 리스트 생성
    photoData.forEach(photo => {
        const photoItem = document.createElement("div");
        photoItem.className = "weather-days__item";

        photoItem.innerHTML = `
            <div class="weather-days__title">${photo.date}</div>
            <div class="weather-days__icon">
                <img src="${photo.filePath}" alt="Captured Photo" class="weather-icon-image">
            </div>
            <div class="weather-days__humidity">${photo.time}</div> <!-- 시간 추가 -->
        `;

        photoContainer.appendChild(photoItem);
    });
}


// 서버에서 사진 정보 가져오기
async function fetchPhotoData() {
    try {
        const response = await fetch("/api/photos");
        if (!response.ok) {
            throw new Error(`HTTP error! Status: ${response.status}`);
        }
        const photoData = await response.json();
        
        if (photoData.length > 0) {
            updatePhotoList(photoData);
        }
    } catch (error) {
        console.error("Failed to fetch photo data:", error);
    }
}


// 사진 데이터가 변경되면 fetchPhotoData 호출
// 예: 서버에서 사진이 업로드되었을 때 이 함수를 호출
function onNewPhotoDataReceived(photoData) {
    updatePhotoList(photoData);
}
