idf_component_register(
    INCLUDE_DIRS "include"
    SRCS "src/micro_ros_comm.c"
    REQUIRES micro_ros_espidf_component my_custom_compose_msg 
             my_custom_msg esp_timer
)