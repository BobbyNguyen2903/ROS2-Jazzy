#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string>

class UartNode : public rclcpp::Node
{
public:
    UartNode() : Node("uart_node")
    {
        uart_fd_ = open("/dev/serial0", O_RDWR | O_NOCTTY);
        if (uart_fd_ < 0) {
            RCLCPP_ERROR(this->get_logger(), "Cannot open /dev/ttyAMA0");
            return;
        }

        struct termios tty;
        tcgetattr(uart_fd_, &tty);
        cfsetspeed(&tty, B115200);
        tty.c_cflag = CS8 | CREAD | CLOCAL;
        tty.c_iflag = 0;
        tty.c_oflag = 0;
        tty.c_lflag = 0;
        tcsetattr(uart_fd_, TCSANOW, &tty);

        subscription_ = this->create_subscription<geometry_msgs::msg::Point>(
            "object/position", 10,
            std::bind(&UartNode::topic_callback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "UART node started");
    }

    ~UartNode()
    {
        if (uart_fd_ >= 0) close(uart_fd_);
    }

private:
    void topic_callback(const geometry_msgs::msg::Point::SharedPtr msg)
    {
        std::string data = "x=" + std::to_string((int)msg->x) +
                           ",y=" + std::to_string((int)msg->y) + "\n";
        write(uart_fd_, data.c_str(), data.size());
        RCLCPP_INFO(this->get_logger(), "UART sent: %s", data.c_str());
    }

    rclcpp::Subscription<geometry_msgs::msg::Point>::SharedPtr subscription_;
    int uart_fd_;
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<UartNode>());
    rclcpp::shutdown();
    return 0;
}
