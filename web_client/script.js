// // C:\server\web_client\script.js

// const WEB_SOCKET_URL = `ws://localhost:3000`;
// const logElement = document.getElementById('output-log');
// const statusElement = document.getElementById('connection-status');
// let ws;

// function initWebSocket() {
//     ws = new WebSocket(WEB_SOCKET_URL);

//     ws.onopen = () => {
//         logMessage('Web Client Connected');
//     };

//     ws.onmessage = (event) => {
//         try {
//             const data = JSON.parse(event.data);
            
//             if (data.type === 'STATUS') {
//                 updateCoreStatus(data.message);
//                 logMessage(`[STATUS] ${data.message}`);
//             } else if (data.type === 'CORE_RESPONSE') {
//                 logMessage(`[SERVER] ${data.data}`);
//             } else if (data.type === 'ERROR') {
//                 logMessage(`[ERROR] ${data.message}`, 'error');
//             }
//         } catch (e) {
//             logMessage(`[RAW DATA] ${event.data}`);
//         }
//     };

//     ws.onerror = (error) => {
//         console.error('WebSocket Error:', error);
//         logMessage(`[ERROR] WebSocket error occurred.`, 'error');
//     };

//     ws.onclose = () => {
//         updateCoreStatus('Core Server Disconnected'); 
//         console.log('WebSocket Disconnected. Reconnecting...');
//         setTimeout(initWebSocket, 2000); 
//     };
// }

// function updateCoreStatus(message) {
//     const coreStatus = (message.includes('Connected')) ? 'Connected' : 'Disconnected';
//     statusElement.textContent = `Core: ${coreStatus}`;
//     statusElement.className = (coreStatus === 'Connected') ? 'status-connected' : 'status-disconnected';
// }

// function logMessage(message, type = 'info') {
//     const now = new Date().toLocaleTimeString();
//     const formattedMessage = `[${now}] ${message}\n`;
//     logElement.textContent += formattedMessage;
//     logElement.scrollTop = logElement.scrollHeight;
// }

// function sendCommand(commandType, argument = '') {
//     if (!ws || ws.readyState !== WebSocket.OPEN) {
//         logMessage('[ERROR] Không có kết nối Web Socket.');
//         return;
//     }
    
//     const command = `${commandType} ${argument}`.trim();

//     logMessage(`[CLIENT] Gửi lệnh: ${command}`);
//     ws.send(command);
// }

// window.onload = initWebSocket;

// C:\server\web_client\script.js

const WEB_SOCKET_URL = `ws://localhost:3000`;
// Cập nhật các element ID theo cấu trúc HTML mới
const systemLogElement = document.getElementById('system-log-output');
const keyLogElement = document.getElementById('key-log');
const processTableBody = document.querySelector('#process-table tbody');
const statusElement = document.getElementById('connection-status');
let ws;

function initWebSocket() {
    ws = new WebSocket(WEB_SOCKET_URL);

    ws.onopen = () => {
        logMessage('Web Client Connected to Core', 'status');
    };

    ws.onmessage = (event) => {
        try {
            const data = JSON.parse(event.data);
            
            if (data.type === 'STATUS') {
                updateCoreStatus(data.message);
                logMessage(`[STATUS] ${data.message}`, 'status');
            } else if (data.type === 'CORE_RESPONSE') {
                logMessage(`[SERVER] ${data.data}`, 'response');
            } else if (data.type === 'ERROR') {
                logMessage(`[ERROR] ${data.message}`, 'error');
            } else if (data.type === 'PROCESS_LIST' && Array.isArray(data.data)) {
                // Xử lý danh sách Process
                displayProcessList(data.data);
                logMessage(`[PROCESS] Nhận được ${data.data.length} Process.`, 'process');
            } else if (data.type === 'KEY_DATA' && typeof data.data === 'string') {
                // Ghi dữ liệu Keylogger liên tục
                logMessage(data.data, 'key_data');
            } else {
                 logMessage(`[RAW JSON] ${event.data}`, 'debug');
            }
        } catch (e) {
            logMessage(`[RAW DATA] ${event.data}`, 'raw');
        }
    };

    ws.onerror = (error) => {
        console.error('WebSocket Error:', error);
        logMessage(`[ERROR] WebSocket error occurred.`, 'error');
    };

    ws.onclose = () => {
        updateCoreStatus('Core Server Disconnected'); 
        console.log('WebSocket Disconnected. Reconnecting...');
        // Thử kết nối lại sau 2 giây
        setTimeout(initWebSocket, 2000); 
    };
}

function updateCoreStatus(message) {
    const coreStatus = (message.includes('Connected')) ? 'Connected' : 'Disconnected';
    statusElement.textContent = `Core: ${coreStatus}`;
    statusElement.className = (coreStatus === 'Connected') ? 'status-connected' : 'status-disconnected';
}

function logMessage(message, type = 'info') {
    const now = new Date().toLocaleTimeString();
    const formattedMessage = `[${now}] ${message}\n`;
    
    // Ghi vào System Log
    systemLogElement.textContent += formattedMessage;
    systemLogElement.scrollTop = systemLogElement.scrollHeight;

    // Nếu là dữ liệu KEYLOGGER, ghi vào Key Log
    if (type === 'key_data') {
        keyLogElement.textContent += message; 
        keyLogElement.scrollTop = keyLogElement.scrollHeight;
    }
}

function displayProcessList(processes) {
    processTableBody.innerHTML = ''; // Xóa nội dung cũ

    processes.forEach(p => {
        const row = processTableBody.insertRow();
        
        // PID
        row.insertCell().textContent = p.pid || 'N/A';
        // Process Name
        row.insertCell().textContent = p.name || 'Unknown';
        // Memory (Giả định server gửi về p.memory_usage_mb)
        row.insertCell().textContent = p.memory_usage_mb ? `${p.memory_usage_mb.toFixed(2)} MB` : 'N/A';
        
        // Action (Nút Kill)
        const actionCell = row.insertCell();
        const killButton = document.createElement('button');
        killButton.innerHTML = '<i class="fas fa-trash"></i> Kill';
        killButton.className = 'danger';
        // Gán lệnh Kill Process với PID cụ thể
        killButton.onclick = () => sendCommand('process kill', p.pid);
        
        actionCell.appendChild(killButton);
    });
}


function sendCommand(commandType, argument = '') {
    if (!ws || ws.readyState !== WebSocket.OPEN) {
        logMessage('[ERROR] Không có kết nối Web Socket.', 'error');
        return;
    }
    
    // Đảm bảo argument là chuỗi rỗng nếu không được truyền
    const argString = (argument !== null && argument !== undefined) ? String(argument) : '';

    const command = `${commandType} ${argString}`.trim();

    logMessage(`[CLIENT] Gửi lệnh: ${command}`, 'info');
    ws.send(command);
}

window.onload = initWebSocket;