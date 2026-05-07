#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

class CameraNode : public rclcpp::Node
{
public:
    CameraNode() : Node("camera_node")
    {
        publisher_ = this->create_publisher<geometry_msgs::msg::Point>("object/position", 10);
        cap_.open(0);
        if (!cap_.isOpened()) {
            RCLCPP_ERROR(this->get_logger(), "Cannot open camera");
            return;
        }

        // Declare HSV parameters
        this->declare_parameter("h_low",  20);
        this->declare_parameter("h_high", 35);
        this->declare_parameter("s_low",  50);
        this->declare_parameter("s_high", 255);
        this->declare_parameter("v_low",  50);
        this->declare_parameter("v_high", 255);

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(33),
            std::bind(&CameraNode::timer_callback, this));

        RCLCPP_INFO(this->get_logger(), "Camera node started");
    }

private:
    void timer_callback()
    {
        cv::Mat frame;
        if (!cap_.read(frame)) return;

        // Read HSV params từ ROS2 parameters
        int h_low  = this->get_parameter("h_low").as_int();
        int h_high = this->get_parameter("h_high").as_int();
        int s_low  = this->get_parameter("s_low").as_int();
        int s_high = this->get_parameter("s_high").as_int();
        int v_low  = this->get_parameter("v_low").as_int();
        int v_high = this->get_parameter("v_high").as_int();

        cv::Scalar hsv_low(h_low, s_low, v_low);
        cv::Scalar hsv_high(h_high, s_high, v_high);

        auto squares = detect_squares(frame, hsv_low, hsv_high);

        for (auto& [cx, cy, x, y, w, h] : squares) {
            auto msg = geometry_msgs::msg::Point();
            msg.x = cx;
            msg.y = cy;
            msg.z = 0.0;
            publisher_->publish(msg);
            RCLCPP_INFO(this->get_logger(), "Detected: x=%d, y=%d", cx, cy);
        }
    }

    std::vector<std::tuple<int,int,int,int,int,int>> detect_squares(
        cv::Mat& frame, cv::Scalar hsv_low, cv::Scalar hsv_high)
    {
        cv::Mat hsv, mask, kernel;
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        cv::inRange(hsv, hsv_low, hsv_high, mask);

        kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5,5));
        cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
        cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        std::vector<std::tuple<int,int,int,int,int,int>> squares;
        for (auto& cnt : contours) {
            double area = cv::contourArea(cnt);
            if (area < 500) continue;

            std::vector<cv::Point> approx;
            cv::approxPolyDP(cnt, approx, 0.04 * cv::arcLength(cnt, true), true);

            if (approx.size() == 4) {
                cv::Rect rect = cv::boundingRect(approx);
                float aspect = (float)rect.width / rect.height;
                if (aspect >= 0.7f && aspect <= 1.3f) {
                    int cx = rect.x + rect.width / 2;
                    int cy = rect.y + rect.height / 2;
                    squares.emplace_back(cx, cy, rect.x, rect.y, rect.width, rect.height);
                }
            }
        }
        return squares;
    }

    rclcpp::Publisher<geometry_msgs::msg::Point>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    cv::VideoCapture cap_;
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CameraNode>());
    rclcpp::shutdown();
    return 0;
}
