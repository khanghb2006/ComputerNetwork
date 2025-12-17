document.addEventListener("DOMContentLoaded", () => {
    
    // --- HIỆU ỨNG MATRIX RAIN (MÀU XANH CYAN) ---
    const canvas = document.getElementById("matrix-bg");
    if (canvas) {
        const ctx = canvas.getContext("2d");

        // Full màn hình
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;

        const letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@#$%^&*";
        const fontSize = 14;
        const columns = canvas.width / fontSize;
        
        // Mảng chứa vị trí y của các cột
        const drops = [];
        for (let i = 0; i < columns; i++) {
            drops[i] = 1;
        }

        function drawMatrix() {
            // Tạo lớp mờ đen để làm vết mờ đuôi chữ
            ctx.fillStyle = "rgba(2, 6, 23, 0.05)"; 
            ctx.fillRect(0, 0, canvas.width, canvas.height);

            ctx.fillStyle = "#38bdf8"; // Màu xanh Cyan
            ctx.font = fontSize + "px 'JetBrains Mono'";

            for (let i = 0; i < drops.length; i++) {
                const text = letters.charAt(Math.floor(Math.random() * letters.length));
                ctx.fillText(text, i * fontSize, drops[i] * fontSize);

                // Reset về đầu khi chạm đáy (ngẫu nhiên)
                if (drops[i] * fontSize > canvas.height && Math.random() > 0.975) {
                    drops[i] = 0;
                }
                drops[i]++;
            }
        }
        
        // Chạy animation 25fps (chậm lại xíu cho đỡ nhức mắt)
        setInterval(drawMatrix, 40);

        // Resize canvas khi chỉnh cửa sổ
        window.addEventListener("resize", () => {
            canvas.width = window.innerWidth;
            canvas.height = window.innerHeight;
        });
    }
});