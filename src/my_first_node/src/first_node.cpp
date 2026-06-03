#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include <sqlite3.h> // 1. 引入 SQLite3

class MyPublishStorageNode : public rclcpp::Node {
public:
    MyPublishStorageNode() : Node("cpp_node_test"), count_(0), db_(nullptr) {
        RCLCPP_INFO(this->get_logger(), "💾 发布端持久化节点启动中...");

        // 2. 初始化发布端数据库（起个不同的名字，叫 pub_manifest.db）
        int rc = sqlite3_open("pub_manifest.db", &db_);
        if (rc != SQLITE_OK) {
            RCLCPP_ERROR(this->get_logger(), "发布端无法打开数据库: %s", sqlite3_errmsg(db_));
            return;
        }
        
        // 3. 建表：如果不存在，建一张名为 pub_history 的表
        const char *sql_create = 
            "CREATE TABLE IF NOT EXISTS pub_history ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "seq_num INTEGER,"
            "pub_time TEXT,"
            "content TEXT);";
        sqlite3_exec(db_, sql_create, nullptr, nullptr, nullptr);

        // 4. 【核心黑科技】：断点恢复！去数据库里查一查上次最后发到了多少
        const char *sql_get_max = "SELECT MAX(seq_num) FROM pub_history;";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db_, sql_get_max, -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                // 如果能查到历史最大序列号，就在它的基础上 + 1，继续往下发
                int max_seq = sqlite3_column_int(stmt, 0);
                if (max_seq > 0) {
                    count_ = max_seq + 1;
                    RCLCPP_WARN(this->get_logger(), "🔄 检测到历史数据！成功触发断点续传，将从序列号 %zu 开始发射...", count_);
                }
            }
            sqlite3_finalize(stmt);
        }

        // 5. 配置 QoS 并创建发布者
        auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10)).transient_local();
        publisher_ = this->create_publisher<std_msgs::msg::String>("security_status", qos_profile);
        
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(1000),
            std::bind(&MyPublishStorageNode::timer_callback, this)
        );
    }

    ~MyPublishStorageNode() {
        if (db_) sqlite3_close(db_);
    }

private:
    void timer_callback() {
        auto message = std_msgs::msg::String();
        message.data = "【核心配置】工业级指令，序列号: " + std::to_string(count_);
        
        RCLCPP_INFO(this->get_logger(), "正在发射数据: '%s'", message.data.c_str());
        publisher_->publish(message);

        // 6. 发布的同时，立刻把账本写进本地数据库
        const char *sql_insert = "INSERT INTO pub_history (seq_num, pub_time, content) VALUES (?, datetime('now', 'localtime'), ?);";
        sqlite3_stmt *insert_stmt;
        if (sqlite3_prepare_v2(db_, sql_insert, -1, &insert_stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(insert_stmt, 1, count_);
            sqlite3_bind_text(insert_stmt, 2, message.data.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(insert_stmt);
            sqlite3_finalize(insert_stmt);
        }

        count_++; // 顺利落盘后，序列号才递增
    }
    
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    size_t count_;
    sqlite3 *db_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MyPublishStorageNode>());
    rclcpp::shutdown();
    return 0;
}