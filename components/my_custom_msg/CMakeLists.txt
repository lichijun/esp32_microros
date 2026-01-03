idf_component_register(
    INCLUDE_DIRS include                     # 头文件搜索路径（对应上面的 include/ 目录）
    SRCS                                     # 不需要源文件，当前目标作为依赖转发器，把静态库传递给上层目标
    REQUIRES micro_ros_espidf_component      # 依赖 micro-ROS 组件（获取 microxrcedds 接口）
)

# 2. 链接静态库，必须使用INTERFACE表明当前目标为接口库
target_link_libraries(${COMPONENT_LIB}
    INTERFACE                            
    ${CMAKE_CURRENT_LIST_DIR}/lib/libmy_custom_message__rosidl_typesupport_c.a
    ${CMAKE_CURRENT_LIST_DIR}/lib/libmy_custom_message__rosidl_typesupport_introspection_c.a
    ${CMAKE_CURRENT_LIST_DIR}/lib/libmy_custom_message__rosidl_typesupport_microxrcedds_c.a
    ${CMAKE_CURRENT_LIST_DIR}/lib/libmy_custom_message__rosidl_generator_c.a
)