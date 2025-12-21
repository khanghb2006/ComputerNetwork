let ws;

async function scanLAN() {
    // 1. Lấy đúng ID từ giao diện console.html mới
    const prefix = document.getElementById("ipRangeInput").value.trim();
    const listArea = document.getElementById("targetSelect"); // Đổ IP vào dropdown
    const scanBtn = document.querySelector(".btn-scan"); 

    if (!prefix) return alert("Hãy nhập dải IP (VD: 192.168.1.)");

    listArea.innerHTML = `<option value="">Searching in ${prefix}...</option>`;
    scanBtn.disabled = true;

    // 2. Vòng lặp quét IP giữ nguyên logic
    for (let i = 1; i <= 254; i += 30) {
        let promises = [];
        for (let j = i; j < i + 30 && j <= 254; j++) {
            promises.push(checkIP(`${prefix}${j}`)); // Lưu ý: prefix đã có dấu chấm ở cuối
        }
        await Promise.all(promises);
    }

    scanBtn.disabled = false;
    if (listArea.innerHTML.includes("Searching")) {
        listArea.innerHTML = "<option value=''>No agents found</option>";
    }
}

function checkIP(ip) {
    return new Promise((resolve) => {
        let t = new WebSocket(`ws://${ip}:8081`);
        t.onopen = () => { addIPCard(ip); t.close(); resolve(); };
        t.onerror = t.onclose = () => resolve();
        setTimeout(() => { t.close(); resolve(); }, 1000); // Timeout 1s
    });
}

function addIPCard(ip) {
    const select = document.getElementById("targetSelect");
    if (select.innerHTML.includes("Searching") || select.innerHTML.includes("No agents")) {
        select.innerHTML = "";
    }

    const opt = document.createElement("option");
    opt.value = ip;
    opt.textContent = `${ip} [ACTIVE]`;
    select.appendChild(opt);
}

function connectWS(ip) {
    // ws = new WebSocket("ws://127.0.0.1:8081");
    if (ws) {
        ws.onclose = null;
        ws.close();
    }

    const target = ip || "127.0.0.1";
    console.log("[WS] Connecting to:", target);
    ws = new WebSocket(`ws://${target}:8081`);

    ws.onopen = () => {
        console.log("[WS] Connected");
        setStatus(true);
        if (typeof appendAppLog == "function")
            appendAppLog(`Connected to agent: ${target}`);
    };

    ws.onmessage = (e) => {
        console.log("RAW FROM SERVER:", e.data);
        handleServerMessage(e.data);
    };

    ws.onerror = (e) => {
        console.error("[WS ERROR]", e);
        alert(`Không thể bắt tay (handshake) với ${target}`);
    };

    ws.onclose = () => {
        setStatus(false);
        console.warn("[WS] Disconnected");
    };
}

function sendCommand(cmd) {
    if (!ws || ws.readyState !== WebSocket.OPEN) {
        alert("WebSocket chưa kết nối!");
        return;
    }
    console.log("[SEND]", cmd);
    ws.send(cmd);
}
    