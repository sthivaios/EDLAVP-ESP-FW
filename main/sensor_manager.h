#ifndef _SENSOR_MANAGER_H
#define _SENSOR_MANAGER_H

#define SENSOR_MANAGER_TASK_STACK_SIZE 8192
#define ONEWIRE_MAX_RX_BYTES 10 // 1byte ROM command + 8byte ROM number + 1byte device command

void sensor_manager(void *pvParameters);

#endif //_SENSOR_MANAGER_H
