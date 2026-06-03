# my_first_node - ROS 2 示例项目

一个展示 ROS 2 核心概念的学习项目，实现了**发布-订阅模式**和**服务通信模式**，并集成了 SQLite3 数据库用于数据持久化。

## 📦 项目概述

### 项目类型
- **框架**: ROS 2（机器人操作系统）
- **编程语言**: C++
- **构建系统**: CMake + ament_cmake
- **数据存储**: SQLite3

### 核心功能
该项目包含 4 个 ROS 2 节点程序，分别演示了不同的通信模式和数据持久化方案：

## 🏗️ 项目架构

### 程序模块

#### 1. **发布端节点** (`cpp_node_exec`)
- **源文件**: `src/first_node.cpp`
- **功能**: 定时发布安全状态消息
- **特性**:
  - 使用 `std_msgs::String` 消息类型
  - 1 秒间隔定时发布数据
  - 支持 **断点续传** - 重启后自动恢复到上次发送位置
  - 每条消息都记录到 `pub_manifest.db` 数据库中

**核心流程**:
```
启动 → 查询历史最大序列号 → 设置续传起点 → 定时发布 → 落盘记录 → 序列号递增
```

#### 2. **订阅端节点** (`cpp_sub_exec`)
- **源文件**: `src/second_node.cpp`
- **功能**: 接收发布端消息并持久化存储
- **特性**:
  - 订阅 `security_status` 话题
  - 所有接收的消息都存储到 `robot_data.db` 数据库
  - 使用预编译语句防止 SQL 注入
  - QoS 配置：保留最后 10 条消息 + 本地持久化

**核心流程**:
```
订阅话题 → 接收消息 → SQL 参数绑定 → 数据库落盘
```

#### 3. **算术服务端** (`cpp_server_exec`)
- **源文件**: `src/add_server.cpp`
- **功能**: 提供算术运算服务（两数相加）
- **通信方式**: ROS 2 Service（同步请求-应答）

#### 4. **算术客户端** (`cpp_client_exec`)
- **源文件**: `src/add_client.cpp`
- **功能**: 调用算术服务，发送两个数，获取求和结果
- **通信方式**: ROS 2 Service 客户端

## 📋 话题和接口

### 发布/订阅话题
| 话题名 | 消息类型 | 发布端 | 订阅端 | 用途 |
|--------|---------|--------|--------|------|
| `security_status` | `std_msgs::String` | cpp_node_exec | cpp_sub_exec | 安全状态消息 |

### 服务接口
| 服务名 | 类型 | 服务端 | 客户端 |
|--------|------|--------|--------|
| `add_two_ints` | `example_interfaces::AddTwoInts` | cpp_server_exec | cpp_client_exec |

### QoS 配置
- **策略**: KeepLast(10) + transient_local
  - 保留最后 10 条消息
  - 支持后加入的订阅者获取历史消息
  - 确保消息可靠传输

## 💾 数据持久化

### 数据库设计

#### 1. pub_manifest.db（发布端账本）
```sql
CREATE TABLE pub_history (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  seq_num INTEGER,              -- 消息序列号
  pub_time TEXT,                -- 发布时间戳
  content TEXT                  -- 消息内容
);
```
**用途**: 记录所有发布的消息，支持断点续传恢复

#### 2. robot_data.db（订阅端日志）
```sql
CREATE TABLE security_logs (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  log_time TEXT,                -- 落盘时间戳
  message TEXT                  -- 接收到的消息
);
```
**用途**: 持久化所有接收到的消息，用于审计和日志查询

### 断点续传机制
发布端启动时执行以下逻辑：
1. 查询 `pub_manifest.db` 中的最大序列号
2. 若存在历史数据，从 `max_seq_num + 1` 开始继续发布
3. 若无历史数据，从 0 开始

**优点**: 
- ✅ 节点重启时不丢失数据
- ✅ 自动故障恢复
- ✅ 业务流程连贯

## 🚀 快速开始

### 前置条件
- ROS 2（已安装）
- C++11 或更高版本
- SQLite3 开发库

### 构建项目
```bash
# 在工作空间根目录执行
cd /home/mac/dev_ws
colcon build
```

### 运行程序

**方案 1: 发布-订阅模式演示**
```bash
# 终端 1：启动发布端
ros2 run my_first_node cpp_node_exec

# 终端 2：启动订阅端
ros2 run my_first_node cpp_sub_exec
```

**方案 2: 服务通信演示**
```bash
# 终端 1：启动服务端
ros2 run my_first_node cpp_server_exec

# 终端 2：启动客户端
ros2 run my_first_node cpp_client_exec
```

### 查看运行日志
```bash
# 查看发布端数据库
sqlite3 pub_manifest.db "SELECT * FROM pub_history;"

# 查看订阅端数据库
sqlite3 robot_data.db "SELECT * FROM security_logs;"
```

## 📁 项目结构

```
dev_ws/
├── src/
│   └── my_first_node/
│       ├── src/
│       │   ├── first_node.cpp          # 发布端：持久化消息发送
│       │   ├── second_node.cpp         # 订阅端：消息接收与落盘
│       │   ├── add_server.cpp          # 服务端：算术运算
│       │   └── add_client.cpp          # 客户端：服务调用
│       ├── CMakeLists.txt              # 构建配置
│       └── package.xml                 # ROS 包元信息
├── build/                              # 构建输出目录
├── install/                            # 安装目录
├── pub_manifest.db                     # 发布端数据库
├── robot_data.db                       # 订阅端数据库
└── README.md                           # 项目说明文档
```

## 🔧 构建配置

### 依赖库
| 依赖 | 类型 | 用途 |
|------|------|------|
| `rclcpp` | buildtool | ROS 2 C++ 库 |
| `std_msgs` | build | 标准消息类型 |
| `example_interfaces` | build | 服务接口定义 |
| `ament_cmake` | buildtool | 构建工具 |
| `sqlite3` | system | 数据库库 |

### 编译选项
```cmake
# 启用所有警告
add_compile_options(-Wall -Wextra -Wpedantic)

# 链接 sqlite3 动态库
target_link_libraries(cpp_node_exec sqlite3)
target_link_libraries(cpp_sub_exec sqlite3)
```

## 💡 设计亮点

### 1. 数据可靠性
- ✅ 每条消息都持久化存储
- ✅ 使用 SQL 预编译语句，防止注入攻击
- ✅ 时间戳记录，便于审计

### 2. 系统鲁棒性
- ✅ 断点续传机制确保数据连续性
- ✅ 安全的数据库连接管理（析构函数关闭）
- ✅ 错误处理和日志输出

### 3. 通信灵活性
- ✅ 发布-订阅模式：异步、解耦、扩展性强
- ✅ 服务模式：同步、可靠、适合请求-应答
- ✅ QoS 配置：保证消息传输质量

## 📝 学习要点

本项目适合用于学习以下 ROS 2 概念：

1. **节点创建和生命周期** - 继承 `rclcpp::Node`
2. **发布-订阅通信** - Publisher 和 Subscription
3. **服务通信** - Service Server 和 Client
4. **定时器回调** - `create_wall_timer`
5. **消息序列化** - 标准消息类型的使用
6. **QoS 策略** - 消息投递保证
7. **数据库集成** - SQLite3 在 ROS 中的应用
8. **SQL 安全编程** - 预编译语句防注入

## 🐛 故障排除

### 问题 1：编译失败找不到 sqlite3
```bash
# 安装 SQLite3 开发库
sudo apt install libsqlite3-dev
```

### 问题 2：数据库权限错误
```bash
# 检查文件权限
ls -la *.db
# 若权限不正确，修改为可读写
chmod 644 *.db
```

### 问题 3：订阅端收不到消息
```bash
# 确保两个节点都在运行
ros2 node list

# 检查话题是否存在
ros2 topic list

# 查看话题信息
ros2 topic info security_status
```

## 📄 许可证
TODO: License declaration

## 👨‍💻 维护者
mac (mac@todo.todo)

---

**最后更新**: 2026-06-03

**项目状态**: ✅ 可用于学习和演示

