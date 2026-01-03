本项目在esp32s3上部署microros，将esp32作为mcu和pc端ros2的通信桥梁，能够实现100Hz的传感器数据和ros数据的稳定传输

### 使用docker安装ROS2

[14个ROS/ROS2版本任选 | 用Docker实现一键安装ROS_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1hY411N7HF/?spm_id_from=333.337.search-card.all.click&vd_source=569f9712cf640a369925c786ad64cd39)

### 安装PC端micro-Ros Agent

[First micro-ROS Application on Linux | micro-ROS](https://micro.ros.org/docs/tutorials/core/first_application_linux/)

[1.安装micro-ros_agent_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1Kz4y1i7A5?spm_id_from=333.788.player.switch&vd_source=569f9712cf640a369925c786ad64cd39&p=2)

[GitHub - micro-ROS/micro_ros_espidf_component: micro-ROS ESP32 IDF component and sample code](https://github.com/micro-ROS/micro_ros_espidf_component)

[micro_ros_espidf_component/examples at kilted · micro-ROS/micro_ros_espidf_component · GitHub](https://github.com/micro-ROS/micro_ros_espidf_component/tree/kilted/examples)

### 编译运行esp32程序

编译运行前配置esp32的micro-ROS Agent IP为PC所连接的wifi的IP地址

### 运行PC端micro-Ros Agent

[GitHub - micro-ROS/micro_ros_espidf_component: micro-ROS ESP32 IDF component and sample code](https://github.com/micro-ROS/micro_ros_espidf_component)

不建议在WSL2运行（UDP端口无法暴露给主机）

[UDP port not exposed from WSL2 to host · Issue #8868 · microsoft/WSL](https://github.com/microsoft/WSL/issues/8868)

`ros2 run micro_ros_agent micro_ros_agent udp4 --port 8888`

### 提高通信稳定性的措施

- 仅使用一个发布方和一个订阅方，通过自定义数据类型实现多源数据的发布订阅
- 发布方使用rclc_publisher_init_best_effort。rclc_publisher_init_default会等待接收方的确认，判断是否需要重传，相比之下rclc_publisher_init_best_effort没有重传机制，通信协议延迟低
- 使用优质网络环境，无线路由器优于手机热点
- esp32主频开到最大，减少esp32日志打印，确保esp32的wifi任务调度正常
