// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from my_custom_compose_message:msg/MyCustomComposeMessage.idl
// generated code does not contain a copyright notice
#include "my_custom_compose_message/msg/detail/my_custom_compose_message__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `imu_test`
#include "sensor_msgs/msg/detail/imu__functions.h"

bool
my_custom_compose_message__msg__MyCustomComposeMessage__init(my_custom_compose_message__msg__MyCustomComposeMessage * msg)
{
  if (!msg) {
    return false;
  }
  // int64_test
  // uint64_test
  // imu_test
  if (!sensor_msgs__msg__Imu__init(&msg->imu_test)) {
    my_custom_compose_message__msg__MyCustomComposeMessage__fini(msg);
    return false;
  }
  return true;
}

void
my_custom_compose_message__msg__MyCustomComposeMessage__fini(my_custom_compose_message__msg__MyCustomComposeMessage * msg)
{
  if (!msg) {
    return;
  }
  // int64_test
  // uint64_test
  // imu_test
  sensor_msgs__msg__Imu__fini(&msg->imu_test);
}

bool
my_custom_compose_message__msg__MyCustomComposeMessage__are_equal(const my_custom_compose_message__msg__MyCustomComposeMessage * lhs, const my_custom_compose_message__msg__MyCustomComposeMessage * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // int64_test
  if (lhs->int64_test != rhs->int64_test) {
    return false;
  }
  // uint64_test
  if (lhs->uint64_test != rhs->uint64_test) {
    return false;
  }
  // imu_test
  if (!sensor_msgs__msg__Imu__are_equal(
      &(lhs->imu_test), &(rhs->imu_test)))
  {
    return false;
  }
  return true;
}

bool
my_custom_compose_message__msg__MyCustomComposeMessage__copy(
  const my_custom_compose_message__msg__MyCustomComposeMessage * input,
  my_custom_compose_message__msg__MyCustomComposeMessage * output)
{
  if (!input || !output) {
    return false;
  }
  // int64_test
  output->int64_test = input->int64_test;
  // uint64_test
  output->uint64_test = input->uint64_test;
  // imu_test
  if (!sensor_msgs__msg__Imu__copy(
      &(input->imu_test), &(output->imu_test)))
  {
    return false;
  }
  return true;
}

my_custom_compose_message__msg__MyCustomComposeMessage *
my_custom_compose_message__msg__MyCustomComposeMessage__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  my_custom_compose_message__msg__MyCustomComposeMessage * msg = (my_custom_compose_message__msg__MyCustomComposeMessage *)allocator.allocate(sizeof(my_custom_compose_message__msg__MyCustomComposeMessage), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(my_custom_compose_message__msg__MyCustomComposeMessage));
  bool success = my_custom_compose_message__msg__MyCustomComposeMessage__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
my_custom_compose_message__msg__MyCustomComposeMessage__destroy(my_custom_compose_message__msg__MyCustomComposeMessage * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    my_custom_compose_message__msg__MyCustomComposeMessage__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
my_custom_compose_message__msg__MyCustomComposeMessage__Sequence__init(my_custom_compose_message__msg__MyCustomComposeMessage__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  my_custom_compose_message__msg__MyCustomComposeMessage * data = NULL;

  if (size) {
    data = (my_custom_compose_message__msg__MyCustomComposeMessage *)allocator.zero_allocate(size, sizeof(my_custom_compose_message__msg__MyCustomComposeMessage), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = my_custom_compose_message__msg__MyCustomComposeMessage__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        my_custom_compose_message__msg__MyCustomComposeMessage__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
my_custom_compose_message__msg__MyCustomComposeMessage__Sequence__fini(my_custom_compose_message__msg__MyCustomComposeMessage__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      my_custom_compose_message__msg__MyCustomComposeMessage__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

my_custom_compose_message__msg__MyCustomComposeMessage__Sequence *
my_custom_compose_message__msg__MyCustomComposeMessage__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  my_custom_compose_message__msg__MyCustomComposeMessage__Sequence * array = (my_custom_compose_message__msg__MyCustomComposeMessage__Sequence *)allocator.allocate(sizeof(my_custom_compose_message__msg__MyCustomComposeMessage__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = my_custom_compose_message__msg__MyCustomComposeMessage__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
my_custom_compose_message__msg__MyCustomComposeMessage__Sequence__destroy(my_custom_compose_message__msg__MyCustomComposeMessage__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    my_custom_compose_message__msg__MyCustomComposeMessage__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
my_custom_compose_message__msg__MyCustomComposeMessage__Sequence__are_equal(const my_custom_compose_message__msg__MyCustomComposeMessage__Sequence * lhs, const my_custom_compose_message__msg__MyCustomComposeMessage__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!my_custom_compose_message__msg__MyCustomComposeMessage__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
my_custom_compose_message__msg__MyCustomComposeMessage__Sequence__copy(
  const my_custom_compose_message__msg__MyCustomComposeMessage__Sequence * input,
  my_custom_compose_message__msg__MyCustomComposeMessage__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(my_custom_compose_message__msg__MyCustomComposeMessage);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    my_custom_compose_message__msg__MyCustomComposeMessage * data =
      (my_custom_compose_message__msg__MyCustomComposeMessage *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!my_custom_compose_message__msg__MyCustomComposeMessage__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          my_custom_compose_message__msg__MyCustomComposeMessage__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!my_custom_compose_message__msg__MyCustomComposeMessage__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
