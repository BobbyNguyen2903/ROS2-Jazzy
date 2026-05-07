ROS2 Multi Nodes Project
1. Project Structure

Tạo cấu trúc thư mục như sau:

```bash
~/ros2_ws/src/my_cpp_pkg
├── CMakeLists.txt
├── launch
│   └── all_nodes.launch.py
├── package.xml
└── src
    ├── camera_node.cpp
    ├── tcp_server_node.cpp
    └── uart_node.cpp
```
2. Install Required System Packages

Trước khi bắt đầu, cài đặt các package cần thiết cho OpenCV và ROS2 image processing:
```bash
sudo apt update

sudo apt install -y \
ros-jazzy-cv-bridge \
ros-jazzy-image-transport \
python3-opencv \
libopencv-dev
```
3. Verify Installation

Kiểm tra package cv_bridge đã được cài thành công:
```bash
ros2 pkg list | grep cv_bridge
```
Kết quả mong đợi:
```bash
cv_bridge
```
4. Rebuild Workspace

Sau khi cài đặt dependencies:
```bash
cd ~/ros2_ws

source /opt/ros/jazzy/setup.bash

colcon build
```
5. Build And Run Project

Sau khi hoàn tất việc thêm source code:
```bash
cd ~/ros2_ws

colcon build --packages-select my_cpp_pkg

source install/setup.bash

ros2 launch my_cpp_pkg all_nodes.launch.py
```
6. Important Notes

Sau mỗi lần chỉnh sửa source code, cần rebuild và source lại workspace để đảm bảo chương trình hoạt động ổn định.
```bash
cd ~/ros2_ws

colcon build --packages-select my_cpp_pkg

.install/setup.bash
```
Khi chạy `all_nodes.launch.py`, hệ thống sẽ khởi tạo đồng thời cả 3 ROS2 nodes. `camera_node` thực hiện việc thu nhận và xử lý hình ảnh để detect vật thể, sau đó tính toán tọa độ `(x, y)` và publish dữ liệu lên ROS2 topic. `uart_node` subscribe dữ liệu này và truyền tọa độ về laptop thông qua giao tiếp UART, trong khi `tcp_server_node` phát dữ liệu tọa độ theo thời gian thực tới các TCP client kết nối vào server thông qua địa chỉ IP của Raspberry Pi và port `9000`.

