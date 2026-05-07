#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <mutex>
#include <vector>

class TcpServerNode : public rclcpp::Node
{
public:
    TcpServerNode() : Node("tcp_server_node")
    {
        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        int opt = 1;
        setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(9000);

        bind(server_fd_, (sockaddr*)&addr, sizeof(addr));
        listen(server_fd_, 5);

        RCLCPP_INFO(this->get_logger(), "TCP server listening on port 9000");

        accept_thread_ = std::thread(&TcpServerNode::accept_loop, this);

        subscription_ = this->create_subscription<geometry_msgs::msg::Point>(
            "object/position", 10,
            std::bind(&TcpServerNode::topic_callback, this, std::placeholders::_1));
    }

    ~TcpServerNode()
    {
        close(server_fd_);
    }

private:
    void accept_loop()
    {
        while (true) {
            int client_fd = accept(server_fd_, nullptr, nullptr);
            if (client_fd < 0) continue;
            RCLCPP_INFO(this->get_logger(), "TCP client connected");
            std::lock_guard<std::mutex> lock(clients_mutex_);
            clients_.push_back(client_fd);
        }
    }

    void topic_callback(const geometry_msgs::msg::Point::SharedPtr msg)
    {
        std::string data = "x=" + std::to_string((int)msg->x) +
                           ",y=" + std::to_string((int)msg->y) + "\n";

        std::lock_guard<std::mutex> lock(clients_mutex_);
        std::vector<int> alive;
        for (int fd : clients_) {
            if (send(fd, data.c_str(), data.size(), MSG_NOSIGNAL) >= 0) {
                alive.push_back(fd);
            } else {
                close(fd);
                RCLCPP_INFO(this->get_logger(), "TCP client disconnected");
            }
        }
        clients_ = alive;
    }

    rclcpp::Subscription<geometry_msgs::msg::Point>::SharedPtr subscription_;
    int server_fd_;
    std::thread accept_thread_;
    std::mutex clients_mutex_;
    std::vector<int> clients_;
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TcpServerNode>());
    rclcpp::shutdown();
    return 0;
}
