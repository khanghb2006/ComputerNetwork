// Hiệu ứng chữ chạy kiểu Hacker
const letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()";

function hackEffect(element) {
    let iteration = 0;
    const originalText = element.dataset.value; // Lấy chữ gốc từ data-value
    
    clearInterval(element.interval);
    
    element.interval = setInterval(() => {
        element.innerText = originalText
            .split("")
            .map((letter, index) => {
                if(index < iteration) {
                    return originalText[index];
                }
                return letters[Math.floor(Math.random() * 26)];
            })
            .join("");
        
        if(iteration >= originalText.length){ 
            clearInterval(element.interval);
        }
        
        iteration += 1 / 3; // Tốc độ chạy (càng nhỏ càng chậm)
    }, 30);
}

// Tự động chạy khi di chuột vào tiêu đề
document.querySelectorAll(".hacker-text").forEach(h1 => {
    h1.addEventListener("mouseover", () => hackEffect(h1));
    // Chạy luôn 1 lần khi load trang
    hackEffect(h1);
});