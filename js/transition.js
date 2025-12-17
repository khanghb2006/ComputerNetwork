document.addEventListener("DOMContentLoaded", () => {
    // 1. Kích hoạt hiệu ứng Fade In khi trang vừa tải xong
    // Sử dụng setTimeout nhỏ để đảm bảo CSS animation bắt được sự thay đổi
    setTimeout(() => {
        document.body.classList.add("loaded");
    }, 50);

    // 2. Xử lý sự kiện click vào các thẻ <a> (Link)
    const links = document.querySelectorAll("a");

    links.forEach(link => {
        link.addEventListener("click", (e) => {
            const href = link.getAttribute("href");
            const target = link.getAttribute("target");

            // Chỉ áp dụng hiệu ứng nếu:
            // - Link có href
            // - Không phải link nội bộ (dấu #)
            // - Không phải mở tab mới (_blank)
            // - Không phải gọi mailto: hoặc tel:
            if (href && !href.startsWith("#") && target !== "_blank" && !href.startsWith("mailto:")) {
                e.preventDefault(); // Ngăn trình duyệt chuyển trang ngay lập tức

                // Kích hoạt hiệu ứng Fade Out (bằng cách bỏ class loaded)
                document.body.classList.remove("loaded");

                // Đợi 400ms (bằng thời gian transition trong CSS) rồi mới chuyển trang
                setTimeout(() => {
                    window.location.href = href;
                }, 400);
            }
        });
    });
});