let index = {
    init: function() {
        $("#register-form").submit(function(event) {
            event.preventDefault();
            index.register();
        });
    },
    register: function() {
        let data = {
            studentid: $("#studentid").val(),
            name: $("#name").val(),
            password: $("#password").val(),
        };
        console.log(data);
        $.ajax({
            type: "POST",
            url: "/register",
            data: JSON.stringify(data),
            contentType: "application/json",
            dataType: "json",
            success: function(response) {
                console.log(response);
                alert("회원가입 성공: " + response.name);
                window.location.href = '/login.html';
            },
            error: function(xhr) {
                alert("회원가입 실패: " + xhr.responseJSON.message);
            }
        });
    }
}

$(document).ready(function() {
    index.init();
});