#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include <sqlite3.h> // 1. 引入 SQLite3 头文件

class MyStorageNode : public rclcpp::Node {
public:
    MyStorageNode() : Node("cpp_storage_node"), db_(nullptr) {
        RCLCPP_INFO(this->get_logger(), "💾 数据库持久化节点启动中...");

        // 2. 初始化数据库：在本地创建一个名为 robot_data.db 的文件
        int rc = sqlite3_open("robot_data.db", &db_);
        if (rc) {
            RCLCPP_ERROR(this->get_logger(), "无法打开数据库: %s", sqlite3_errmsg(db_));
            return;
        }

        // 3. 建表：如果表不存在，创建一张名为 security_logs 的表，包含 id, time, message 三列
        const char *sql_create_table = 
            "CREATE TABLE IF NOT EXISTS security_logs ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "log_time TEXT,"
            "message TEXT);";
        
        char *err_msg = nullptr;
        rc = sqlite3_exec(db_, sql_create_table, nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            RCLCPP_ERROR(this->get_logger(), "建表失败: %s", err_msg);
            sqlite3_free(err_msg);
        } else {
            RCLCPP_INFO(this->get_logger(), "✅ 数据库及数据表就绪，开始持久化监听...");
        }

        // 4. 开启 ROS 2 订阅者
        auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10)).transient_local();
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "security_status", 
            qos_profile, 
            std::bind(&MyStorageNode::topic_callback, this, std::placeholders::_1)
        );
    }

    // 析构函数：节点关闭时，必须安全关闭数据库连接
    ~MyStorageNode() {
        if (db_) {
            sqlite3_close(db_);
            RCLCPP_INFO(this->get_logger(), "🔒 数据库已安全关闭");
        }
    }

private:
    void topic_callback(const std_msgs::msg::String::SharedPtr msg) {
        RCLCPP_INFO(this->get_logger(), "【落盘捕获】: '%s'", msg->data.c_str());

        // 5. 将接收到的数据安全地插入到数据库中
        const char *sql_insert = "INSERT INTO security_logs (log_time, message) VALUES (datetime('now', 'localtime'), ?);";
        sqlite3_stmt *stmt;
        
        // 使用预编译语句（Prepare），防止字符串里带特殊符号导致 SQL 注入或崩溃
        if (sqlite3_prepare_v2(db_, sql_insert, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, msg->data.c_str(), -1, SQLITE_TRANSIENT);
            
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                RCLCPP_ERROR(this->get_logger(), "数据落盘失败！");
            }
            sqlite3_finalize(stmt);
        }
    }

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
    sqlite3 *db_; // SQLite3 数据库句柄指针
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MyStorageNode>());
    rclcpp::shutdown();
    return 0;
}