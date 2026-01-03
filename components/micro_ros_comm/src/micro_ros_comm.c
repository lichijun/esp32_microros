#include "micro_ros_comm.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microros/rmw_microros.h>
#include <micro_ros_utilities/string_utilities.h>
#include <std_msgs/msg/int32.h>
#include <sensor_msgs/msg/imu.h>
#include "my_custom_message/msg/my_custom_message.h"
#include "my_custom_compose_message/msg/my_custom_compose_message.h"

#define RCCHECK(fn)                                                                      \
    {                                                                                    \
        rcl_ret_t temp_rc = fn;                                                          \
        if ((temp_rc != RCL_RET_OK))                                                     \
        {                                                                                \
            printf("Failed status on line %d: %d. Aborting.\n", __LINE__, (int)temp_rc); \
            vTaskDelete(NULL);                                                           \
        }                                                                                \
    }
#define RCSOFTCHECK(fn)                                                                    \
    {                                                                                      \
        rcl_ret_t temp_rc = fn;                                                            \
        if ((temp_rc != RCL_RET_OK))                                                       \
        {                                                                                  \
            printf("Failed status on line %d: %d. Continuing.\n", __LINE__, (int)temp_rc); \
        }                                                                                  \
    }


#define ROS_NAMESPACE      CONFIG_MICRO_ROS_NAMESPACE
#define ROS_DOMAIN_ID      CONFIG_MICRO_ROS_DOMAIN_ID
#define ROS_AGENT_IP       CONFIG_MICRO_ROS_AGENT_IP
#define ROS_AGENT_PORT     CONFIG_MICRO_ROS_AGENT_PORT
#define PI 3.14159265359

rcl_publisher_t publisher_int32;
rcl_publisher_t publisher_custom_msg;
rcl_publisher_t publisher_custom_compose_msg;
std_msgs__msg__Int32 msg_int32;
my_custom_message__msg__MyCustomMessage msg_custom;
my_custom_compose_message__msg__MyCustomComposeMessage msg_custom_compose;
rcl_subscription_t subscriber_1;
rcl_subscription_t subscriber_2;
my_custom_compose_message__msg__MyCustomComposeMessage msg_sub1;
std_msgs__msg__Int32 msg_sub2;
rcl_timer_t timer_imu;

static const char *TAG = "MICRO_ROS_COMM";
static portMUX_TYPE g_shared_resource_mux = portMUX_INITIALIZER_UNLOCKED;
static portMUX_TYPE g_imu_data_mux = portMUX_INITIALIZER_UNLOCKED;
static volatile int32_t g_data = 0;
static volatile bool g_new_data = false;
static volatile float g_imu_acc_x, g_imu_acc_y, g_imu_acc_z;
static volatile float g_imu_gyro_x, g_imu_gyro_y, g_imu_gyro_z;
static volatile bool g_new_imu_data = false;
static bool micro_ros_comm_inited = false;

// 定时器回调函数
void timer_imu_callback(rcl_timer_t *timer, int64_t last_call_time)
{
    RCLC_UNUSED(last_call_time);
    if (timer != NULL)
    {
        msg_custom_compose.imu_test.linear_acceleration.x = g_imu_acc_x;
        msg_custom_compose.imu_test.linear_acceleration.y = g_imu_acc_y;
        msg_custom_compose.imu_test.linear_acceleration.z = g_imu_acc_z;
        msg_custom_compose.imu_test.angular_velocity.x = g_imu_gyro_x;
        msg_custom_compose.imu_test.angular_velocity.y = g_imu_gyro_y;
        msg_custom_compose.imu_test.angular_velocity.z = g_imu_gyro_z;
        if (g_new_imu_data)
        {
            int64_t ts_publish_start = esp_timer_get_time();
            RCSOFTCHECK(rcl_publish(&publisher_custom_compose_msg, &msg_custom_compose, NULL));
            g_new_imu_data = false;
            int64_t ts_publish_end = esp_timer_get_time();
            if (ts_publish_end - ts_publish_start > 10000)
                ESP_LOGW(TAG, "too long,%dms", (ts_publish_end - ts_publish_start)/1000);
        }
    }
}

// 订阅者Subscriber_1回调函数 
void subscription_1_callback(const void * msgin)
{
	const my_custom_compose_message__msg__MyCustomComposeMessage * msg = (const my_custom_compose_message__msg__MyCustomComposeMessage *)msgin;
	printf("Sub1 Received: %d\n",  (int)  msg->uint64_test);
}

void subscription_2_callback(const void * msgin)
{
	const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;
	// printf("Sub2 Received: %ld\n",  msg->data);
    // taskENTER_CRITICAL(&g_shared_resource_mux);
    // g_data = msg->data;
    // taskEXIT_CRITICAL(&g_shared_resource_mux);
}

// micro_ros处理任务 
void micro_ros_task(void *arg)
{
    rcl_allocator_t allocator = rcl_get_default_allocator();
    rclc_support_t support;

    // 创建rcl初始化选项
    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    RCCHECK(rcl_init_options_init(&init_options, allocator));
    // 修改ROS域ID
    // RCCHECK(rcl_init_options_set_domain_id(&init_options, ROS_DOMAIN_ID));

    // 初始化rmw选项
    rmw_init_options_t *rmw_options = rcl_init_options_get_rmw_init_options(&init_options);

    // 设置静态代理IP和端口
    RCCHECK(rmw_uros_options_set_udp_address(ROS_AGENT_IP, ROS_AGENT_PORT, rmw_options));

    // 尝试连接代理，连接成功才进入下一步。
    int state_agent = 0;
    while (1)
    {
        ESP_LOGI(TAG, "Connecting agent: %s:%s", ROS_AGENT_IP, ROS_AGENT_PORT);
        state_agent = rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator);
        if (state_agent == ESP_OK)
        {
            ESP_LOGI(TAG, "Connected agent: %s:%s", ROS_AGENT_IP, ROS_AGENT_PORT);
            break;
        }
        
        // sleep(1);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    // 创建ROS2节点
    rcl_node_t node;
    RCCHECK(rclc_node_init_default(&node, "imu_publisher", ROS_NAMESPACE, &support));

    // 创建发布者
    RCCHECK(rclc_publisher_init_best_effort(
        &publisher_int32,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "int32num"));

    // RCCHECK(rclc_publisher_init_default(
    RCCHECK(rclc_publisher_init_best_effort(
        &publisher_custom_compose_msg,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(my_custom_compose_message, msg, MyCustomComposeMessage),
        "custom_compose_msg"));

    // 创建订阅者
	RCCHECK(rclc_subscription_init_default(
		&subscriber_1,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(my_custom_compose_message, msg, MyCustomComposeMessage),
		"subscriber_1"));

    RCCHECK(rclc_subscription_init_best_effort(
		&subscriber_2,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
		"command"));

    // 创建定时器，设置发布频率为200HZ
    const unsigned int timer_timeout = 10;
    RCCHECK(rclc_timer_init_default(
        &timer_imu,
        &support,
        RCL_MS_TO_NS(timer_timeout),
        timer_imu_callback));

    // 创建执行者，其中三个参数为执行者控制的数量，要大于或等于添加到执行者的订阅者和发布者数量。
    rclc_executor_t executor;
    int handle_num = 3;
    RCCHECK(rclc_executor_init(&executor, &support.context, handle_num, &allocator));
    
    // 添加发布者的定时器到执行者
    RCCHECK(rclc_executor_add_timer(&executor, &timer_imu));

    // 添加订阅者到执行者
    RCCHECK(rclc_executor_add_subscription(&executor, &subscriber_1, &msg_sub1, &subscription_1_callback, ON_NEW_DATA));
    RCCHECK(rclc_executor_add_subscription(&executor, &subscriber_2, &msg_sub2, &subscription_2_callback, ON_NEW_DATA));

    micro_ros_comm_inited = true;
    // 循环执行microROS任务
    while (1)
    {
        rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
        // usleep(1000);
    }

    // 释放资源
    RCCHECK(rcl_publisher_fini(&publisher_int32, &node));
    RCCHECK(rcl_publisher_fini(&publisher_custom_msg, &node));
    RCCHECK(rcl_publisher_fini(&publisher_custom_compose_msg, &node));
    RCCHECK(rcl_subscription_fini(&subscriber_1, &node));
    RCCHECK(rcl_subscription_fini(&subscriber_2, &node));
    RCCHECK(rcl_node_fini(&node));

    vTaskDelete(NULL);
}

void publish_int32(int32_t data)
{
    if (micro_ros_comm_inited)
    {    
        msg_int32.data = data;
        RCSOFTCHECK(rcl_publish(&publisher_int32, &msg_int32, NULL));
    }
    else
    {
        ESP_LOGI(TAG, "micro ros is not inited");
    }
}

void update_int32(int32_t data)
{
    taskENTER_CRITICAL(&g_shared_resource_mux);
    g_data = data;
    g_new_data = true;
    taskEXIT_CRITICAL(&g_shared_resource_mux);
}

void update_imu_data(float acc_x, float acc_y, float acc_z, float gyro_x, float gyro_y, float gyro_z)
{
    taskENTER_CRITICAL(&g_imu_data_mux);
    g_imu_acc_x = acc_x;
    g_imu_acc_y = acc_y;
    g_imu_acc_z = acc_z;
    g_imu_gyro_x = gyro_x / 180 * PI;
    g_imu_gyro_y = gyro_y / 180 * PI;
    g_imu_gyro_z = gyro_z / 180 * PI;
    g_new_imu_data = true;
    taskEXIT_CRITICAL(&g_imu_data_mux);
}