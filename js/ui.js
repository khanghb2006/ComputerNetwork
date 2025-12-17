function setStatus(connected) {
    const el = document.getElementById("status");
    if (!el) return;

    el.className = "status-badge " + (connected ? "connected" : "disconnected");

    const textSpan = el.querySelector(".status-text");
    if (textSpan) {
        textSpan.textContent = connected ? "Connected" : "Disconnected";
    } else {
        el.innerHTML = `<span class="status-dot"></span><span class="status-text">${connected ? "Connected" : "Disconnected"}</span>`;
    }
}

let imgBuffer = "";

function handleServerMessage(msg) {

    if (msg.startsWith("IMAGE_FRAME:") || imgBuffer.startsWith("IMAGE_FRAME:")) {
        imgBuffer += msg;

        if (imgBuffer.includes("__EOF__")) {
            let base64 = imgBuffer
                .replace("IMAGE_FRAME:", "")
                .replace("__EOF__", "")
                .replace(/\s+/g, ""); // remove whitespaces

            imgBuffer = "";
            renderImage(base64);
        }
        return;
    }

    if (msg.startsWith("KEYLOG_DATA:")) {
        const keylog = document.getElementById("keylog_output");
        if (keylog) keylog.textContent = msg.replace("KEYLOG_DATA:", "");
        return;
    }

    if (msg.startsWith("PROCESS_LIST:")) {
        const procList = document.getElementById("procList");
        if (procList) procList.textContent = msg.replace("PROCESS_LIST:", "");
        return;
    }

    if (msg.startsWith("RECORD_DONE:")) {
        const path = msg.replace("RECORD_DONE:", "").trim();

        lastVideoUrl = "http://127.0.0.1:8080/Video/record.mp4";

        const video = document.getElementById("videoPlayer");
        video.src = lastVideoUrl;
        video.load();
        video.play();

        return;
    }

    if (msg.startsWith("PROCESS_LIST:")) {
        const procList = document.getElementById("procList");
        if (!procList) return;

        const text = msg.slice("PROCESS_LIST:".length);
        procList.textContent = text.trim();
        return;
    }

    // ===== 3. APPLICATION LIST =====
    let obj;
    obj = JSON.parse(msg);
    if (obj.type === "APPLICATION_LIST") {
        const appList = document.getElementById("appList");
        const appLog  = document.getElementById("appLog");
        if (!appList || !appLog) return;

        let listOutput = "";
        let logOutput  = "";

        obj.data.forEach(app => {
            // LIST
            listOutput += `${app.pid} | ${app.name}\n`;

            // LOG
            logOutput += `[${new Date().toLocaleTimeString()}] PID: ${app.pid} | ${app.name}\n`;
            logOutput += `Path: ${app.path || "[unknown]"}\n\n`;
        });

        appList.textContent = listOutput.trim();
        appLog.textContent += logOutput;
        appLog.scrollTop = appLog.scrollHeight;
        return;
    }

    // ===== 4. JSON KHÁC =====
    const log = document.getElementById("log");
    if (log) log.textContent += JSON.stringify(obj) + "\n";

    console.log("RAW FROM SERVER:", e.data);
    handleServerMessage(e.data);
}


console.log("ui.js loaded");
function switchTab(id) {
    document.querySelectorAll(".content-view").forEach(v => {
        v.classList.remove("active");
    });

    const view = document.getElementById(id);
    if (view) view.classList.add("active");

    document.querySelectorAll(".nav-btn").forEach(b => {
        b.classList.remove("active");
    });

    const allButtons = Array.from(document.querySelectorAll(".nav-btn"));
    const activeButton = allButtons.find(b => {
        const onClickAttr = b.getAttribute("onclick");
        return onClickAttr && onClickAttr.includes(`'${id}'`);
    });

    if (activeButton) {
        activeButton.classList.add("active");
    }

    console.log("Switched tab to:", id);
}

function appendAppLog(text){
    const appLog = document.getElementById("appLog"); // ĐÚNG
    if (!appLog) return;

    const time = new Date().toLocaleTimeString();
    appLog.textContent += `[${time}] ${text}\n`;
    appLog.scrollTop = appLog.scrollHeight;
}

function renderImage(base64) {
    const img = document.getElementById("screenshotImg");
    if (!img) return;

    let mime = "image/bmp"; // ✅ BMP

    // auto detect (an toàn)
    if (base64.startsWith("iVBOR")) mime = "image/png";
    else if (base64.startsWith("/9j/")) mime = "image/jpeg";
    else if (base64.startsWith("Qk")) mime = "image/bmp";

    img.src = `data:${mime};base64,${base64}`;
    img.style.display = "block";
}