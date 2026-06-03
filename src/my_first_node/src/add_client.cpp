#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/srv/add_two_ints.hpp"

#include <chrono>
#include <cstdlib>
#include <memory>

using namespace std::chrono_literals;

class MyAddClientNode : public rclcpp::Node {
public:
    MyAddClientNode() : Node("cpp_service_client_test") {
        RCLCPP_INFO(this->get_logger(), "📞 算术计算客户端（Client）已启动...");

        // 1. 创建服务客户端，指定寻找的话题名称："add_two_ints_service"
        client_ = this->create_client<example_interfaces::srv::AddTwoInts>("add_two_ints_service");
    }

    // 2. 核心发送请求函数
    void send_request(int64_t a, int64_t b) {
        // 先检查服务在不在，如果不在，每隔 1 秒死等它上线
        while (!client_->wait_for_service(1s)) {
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(this->get_logger(), "在等待服务时被强行中断退出");
                return;
            }
            RCLCPP_WARN(this->get_logger(), "📡 服务端未上线，正在持续呼叫中...");
        }

        // 3. 组装请求包裹
        auto request = std::make_shared<example_interfaces::srv::AddTwoInts::Request>();
        request->a = a;
        request->b = b;

        RCLCPP_INFO(this->get_logger(), "🚀 正在向服务端发射请求: a = %ld, b = %ld", a, b);

        // 4. 【异步黑科技】：发送请求，并绑定一个“等答案”的回调函数（C++ Lambda 表达式）
        // async_send_request 发完立马转身离开，绝不卡死
        client_->async_send_request(request, 
            [this](rclcpp::Client<example_interfaces::srv::AddTwoInts>::SharedFuture future) {
                // 当服务端算完把答案送回时，这个内部小函数会被自动触发
                auto response = future.get();
                RCLCPP_INFO(this->get_logger(), "🎉 【请求成功】答案安全送回！结果 sum = %ld", response->sum);
            }
        );
    }

private:
    rclcpp::Client<example_interfaces::srv::AddTwoInts>::SharedPtr client_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    
    // 实例化客户端节点
    auto node = std::make_shared<MyAddClientNode>();

    // 5. 调用函数发送一笔请求（比如给服务器算 88 + 12）
    node->send_request(88, 12);

    // 让节点保持清醒，专门负责在后台接收异步送回的答案
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}