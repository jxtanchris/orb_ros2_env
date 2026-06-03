#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/srv/add_two_ints.hpp" // 1. 引入两数相加的服务接口头文件

#include <memory>

class MyAddServerNode : public rclcpp::Node {
public:
    MyAddServerNode() : Node("cpp_service_server_test") {
        RCLCPP_INFO(this->get_logger(), "🧮 算术计算服务端（Server）已启动，等待客户下单...");

        // 2. 创建服务服务端
        // 服务名称叫 "add_two_ints_service"
        // 收到请求后，自动触发 handle_add_request 函数进行计算
        server_ = this->create_service<example_interfaces::srv::AddTwoInts>(
            "add_two_ints_service",
            std::bind(&MyAddServerNode::handle_add_request, this, std::placeholders::_1, std::placeholders::_2)
        );
    }

private:
    // 3. 核心回调函数
    // request 指针包含传进来的 a 和 b；response 指针负责装我们要返回的 sum
    void handle_add_request(
        const std::shared_ptr<example_interfaces::srv::AddTwoInts::Request> request,
        std::shared_ptr<example_interfaces::srv::AddTwoInts::Response> response) 
    {
        // 收到客户端传来的参数，开始算术
        response->sum = request->a + request->b;
        
        RCLCPP_INFO(this->get_logger(), "Incoming request: a = %ld, b = %ld", request->a, request->b);
        RCLCPP_INFO(this->get_logger(), "Sending back response: sum = %ld", response->sum);
    }

    rclcpp::Service<example_interfaces::srv::AddTwoInts>::SharedPtr server_; // 服务端智能指针
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MyAddServerNode>());
    rclcpp::shutdown();
    return 0;
}