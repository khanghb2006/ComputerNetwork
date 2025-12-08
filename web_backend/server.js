// C:\server\web_backend\server.js

try {
    const express = require('express');
    const http = require('http');
    const WebSocket = require('ws');
    const net = require('net'); 
    const path = require('path');

    const WEB_PORT = 3000;
    const CORE_SERVER_HOST = '192.168.1.201'; //Thay doi IP o cho nay
    const CORE_SERVER_PORT = 8888; 

    const app = express();
    const server = http.createServer(app);
    const wss = new WebSocket.Server({ server });

    // Phục vụ các file tĩnh
    app.use(express.static(path.join(__dirname, '..', 'web_client')));

    let coreSocket = null; 

    // ==========================================================
    // 1. Kết nối đến Server C++ (Core Server)
    // ==========================================================
    function connectToCoreServer() {
        if (coreSocket && coreSocket.readyState === 'open') return; 
        if (coreSocket && !coreSocket.destroyed) coreSocket.destroy(); 

        coreSocket = net.createConnection({ host: CORE_SERVER_HOST, port: CORE_SERVER_PORT }, () => {
            console.log('Connected to C++ Core Server.');
            wss.clients.forEach(client => {
                if (client.readyState === WebSocket.OPEN) {
                    client.send(JSON.stringify({ type: 'STATUS', message: 'Core Server Connected' }));
                }
            });
        });

        coreSocket.on('data', (data) => {
            const response = data.toString().trim();
            console.log(`Data from Core Server: ${response}`);
            
            wss.clients.forEach(client => {
                if (client.readyState === WebSocket.OPEN) {
                    client.send(JSON.stringify({ type: 'CORE_RESPONSE', data: response }));
                }
            });
        });

        coreSocket.on('error', (err) => {
            // Hiển thị lỗi kết nối nếu không tìm thấy C++ Server
            console.error('Core Server Connection Error:', err.message);
            if (coreSocket) coreSocket.destroy();
        });

        coreSocket.on('close', () => {
            console.log('C++ Core Server connection closed. Retrying in 5s...');
            coreSocket = null;
            wss.clients.forEach(client => {
                if (client.readyState === WebSocket.OPEN) {
                    client.send(JSON.stringify({ type: 'STATUS', message: 'Core Server Disconnected' }));
                }
            });
            setTimeout(connectToCoreServer, 5000);
        });
    }

    // ==========================================================
    // 2. WebSocket Server (Giao tiếp với Web Client)
    // ==========================================================
    wss.on('connection', (ws) => {
        console.log('Web Client Connected.');

        const connectionStatus = (coreSocket && !coreSocket.destroyed) ? 'Core Server Connected' : 'Core Server Disconnected';
        ws.send(JSON.stringify({ 
            type: 'STATUS', 
            message: connectionStatus
        }));

        ws.on('message', (message) => {
            const command = message.toString();
            console.log(`Received command from Web Client: ${command}`);

            if (coreSocket && coreSocket.writable && connectionStatus === 'Core Server Connected') {
                coreSocket.write(command + '\n');
            } else {
                ws.send(JSON.stringify({ type: 'ERROR', message: 'Core Server is not connected. Please start server.exe first.' }));
            }
        });
    });

    // Khởi động server
    server.listen(WEB_PORT, () => {
        console.log(`Web Server running on http://localhost:${WEB_PORT}`);
        connectToCoreServer();
    });
    
} catch (error) {
    // Nếu có lỗi ngay lập tức trong quá trình khởi tạo module/server
    console.error('----------------------------------------------------');
    console.error('❌ SERVER INITIALIZATION FAILED. CHECK MODULES/PATHS.');
    console.error(`Error: ${error.message}`);
    console.error('----------------------------------------------------');
}