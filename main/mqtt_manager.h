#ifndef _MQTT_MANAGER_H
#define _MQTT_MANAGER_H

void mqtt_app_start();

#define MQTT_MANAGER_TASK_STACK_SIZE 4096

void mqtt_manager(void *pvParameters);

#endif //_MQTT_MANAGER_H