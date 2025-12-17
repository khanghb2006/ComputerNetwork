let ws;

function connectWS() {
    ws = new WebSocket("ws://127.0.0.1:8081");

    ws.onopen = () => {
        console.log("[WS] Connected");
        setStatus(true);
    };

    ws.onmessage = (e) => {
        console.log("RAW FROM SERVER:", e.data);
        handleServerMessage(e.data);
    };

    ws.onerror = (e) => {
        console.error("[WS ERROR]", e);
        setStatus(false);
    };

    ws.onclose = () => {
        console.warn("[WS] Disconnected");
        setStatus(false);
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

// auto connect
connectWS();
