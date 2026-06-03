cmake_minimum_required(VERSION 3.8)
project(my_first_node)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(example_interfaces REQUIRED)




# 1. 把 first_node.cpp 编译成名为 cpp_node_exec 的可执行文件
add_executable(cpp_node_exec src/first_node.cpp)

# 2. 引入 ROS 2 节点的依赖库
ament_target_dependencies(cpp_node_exec rclcpp std_msgs)

# 给发布者也链接 sqlite3 库
target_link_libraries(cpp_node_exec sqlite3)

# 3. 把编译好的程序安装到系统的 bin 目录下，方便 ros2 run 指令能找到它
install(TARGETS
  cpp_node_exec
  DESTINATION lib/${PROJECT_NAME}
)

# 1. 把 second_node.cpp 编译成名为 cpp_sub_exec 的可执行文件
add_executable(cpp_sub_exec src/second_node.cpp)

# 2. 引入依赖库
ament_target_dependencies(cpp_sub_exec rclcpp std_msgs)

# ======【核心修改：增加下面这一行】======
# 告诉编译器，把系统的 sqlite3 动态链接库绑定给这个订阅者程序
target_link_libraries(cpp_sub_exec sqlite3)
# ======================================

# 3. 把这个新程序也安装到系统路径中
install(TARGETS
  cpp_sub_exec
  DESTINATION lib/${PROJECT_NAME}
)

# 编译算术服务端
add_executable(cpp_server_exec src/add_server.cpp)
ament_target_dependencies(cpp_server_exec rclcpp example_interfaces)

install(TARGETS
  cpp_server_exec
  DESTINATION lib/${PROJECT_NAME}
)

# 编译算术客户端
add_executable(cpp_client_exec src/add_client.cpp)
ament_target_dependencies(cpp_client_exec rclcpp example_interfaces)

install(TARGETS
  cpp_client_exec
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
