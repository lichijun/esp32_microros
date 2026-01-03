// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from my_custom_compose_message:msg/MyCustomComposeMessage.idl
// generated code does not contain a copyright notice

#ifndef MY_CUSTOM_COMPOSE_MESSAGE__MSG__DETAIL__MY_CUSTOM_COMPOSE_MESSAGE__STRUCT_H_
#define MY_CUSTOM_COMPOSE_MESSAGE__MSG__DETAIL__MY_CUSTOM_COMPOSE_MESSAGE__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'imu_test'
#include "sensor_msgs/msg/detail/imu__struct.h"

/// Struct defined in msg/MyCustomComposeMessage in the package my_custom_compose_message.
typedef struct my_custom_compose_message__msg__MyCustomComposeMessage
{
  int64_t int64_test;
  uint64_t uint64_test;
  sensor_msgs__msg__Imu imu_test;
} my_custom_compose_message__msg__MyCustomComposeMessage;

// Struct for a sequence of my_custom_compose_message__msg__MyCustomComposeMessage.
typedef struct my_custom_compose_message__msg__MyCustomComposeMessage__Sequence
{
  my_custom_compose_message__msg__MyCustomComposeMessage * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} my_custom_compose_message__msg__MyCustomComposeMessage__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // MY_CUSTOM_COMPOSE_MESSAGE__MSG__DETAIL__MY_CUSTOM_COMPOSE_MESSAGE__STRUCT_H_
