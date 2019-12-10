#pragma once
#if defined USE_ESP32_AT_SERIAL1
    #include"Arduino.h"
    #include"Seeed_Arduino_FreeRTOS.h"
    
    struct Rtos{
        static uint32_t msToTicks(uint32_t ms){
            return ((ms * 1000) / portTICK_PERIOD_US);
        }
        template<class func>
        static void critical(func const & call){
            taskENTER_CRITICAL();
            call();
            taskEXIT_CRITICAL();
        }
        static void loopWait(volatile bool * flag, uint32_t ms = -1){
            loopWait([&](){ return flag[0]; }, ms);
        }
        template<class func>
        static void loopWait(func const & call, uint32_t ms = -1){
            while (ms) {
                Rtos::delayus(500); if (call()) break;
                Rtos::delayus(500); if (call()) break;
                if (~ms){
                    ms -= 1;
                }
                if (ms == 0){
                    return;
                }
            }
        }
        static void delayus(uint32_t us){
            vTaskDelay(((us) / portTICK_PERIOD_US));
        }
        static void delayms(uint32_t ms){
            delayus(ms * 1000);
        }
        static void createSemaphore(void ** handle, uint32_t maxPushRequest, uint32_t initialValue = 0){
            handle[0] = xSemaphoreCreateCounting(maxPushRequest, initialValue);
        }
        static void createSemaphore(void ** handle){
            handle[0] = xSemaphoreCreateBinary();
        }
        static void createQueue(void ** handle, uint32_t length, uint32_t itemSize = 1){
            handle[0] = xQueueCreate(length, itemSize);
        }
        template<class type>
        static void queueSend(void * handle, type const & value, uint32_t Timeout = portMAX_DELAY){
            xQueueSend(QueueHandle_t(handle), & value, Timeout);
        }
        template<class type>
        static void queueSendFromISR(void * handle, type const & value){
            BaseType_t tmp = pdFALSE;
            xQueueSendToBackFromISR(QueueHandle_t(handle), & value, & tmp);
        }
        template<class type>
        static void queueReceive(void * handle, type * value, uint32_t Timeout = portMAX_DELAY){
            type dummy;
            xQueueReceive(QueueHandle_t(handle), value ? value : & dummy, Timeout);
        }
        template<class type>
        static void queuePeek(void * handle, type * value, uint32_t Timeout = portMAX_DELAY){
            xQueuePeek(QueueHandle_t(handle), value, Timeout);
        }
        static void queueSize(void * handle, uint32_t * size){
            size[0] = uxQueueMessagesWaiting(QueueHandle_t(handle));
        }
        static void take(void * sema, uint32_t Timeout = portMAX_DELAY){
            xSemaphoreTake((SemaphoreHandle_t)sema, Timeout);
        }
        static void give(void * sema) {
            xSemaphoreGive((SemaphoreHandle_t)sema);
        }
        static void giveFromISR(void * sema) {
            auto tmp = pdFALSE;
            xSemaphoreGiveFromISR((SemaphoreHandle_t)sema, & tmp);
        }
        static void createThread(
            void **      handle, 
            const char * taskName, 
            void      (* invoke)(void *), 
            void       * arg,
            uint32_t     stackSize, 
            uint32_t     priority){
            xTaskCreate(invoke, taskName, stackSize, arg, tskIDLE_PRIORITY + priority, (tskTaskControlBlock **)handle);
        }
        static void scheduler(){
            vTaskStartScheduler();
        }
        static void malloc(void ** ptr, uint32_t size){
            ptr[0] = pvPortMalloc(size);
        }
        static void free(void * ptr){
            if (ptr){
                vPortFree(ptr);
            }
        }
    };
#else
    struct Rtos{
        static uint32_t msToTicks(uint32_t ms){
            return ((ms * 1000) / portTICK_PERIOD_MS);
        }
        template<class func>
        static void critical(func const & call){}
        static void loopWait(volatile bool * flag, uint32_t ms = -1){}
        template<class func>
        static void loopWait(func const & call, uint32_t ms = -1){}
        static void delayus(uint32_t us){}
        static void delayms(uint32_t ms){}
        static void createSemaphore(void ** handle, uint32_t maxPushRequest, uint32_t initialValue = 0){}
        static void createSemaphore(void ** handle){}
        static void createQueue(void ** handle, uint32_t length, uint32_t itemSize = 1){}
        template<class type>
        static void queueSend(void * handle, type const & value, uint32_t Timeout = -1){}
        template<class type>
        static void queueSendFromISR(void * handle, type const & value){}
        template<class type>
        static void queueReceive(void * handle, type * value, uint32_t Timeout = -1){}
        template<class type>
        static void queuePeek(void * handle, type * value, uint32_t Timeout = -1){}
        static void queueSize(void * handle, uint32_t * size){}
        static void take(void * sema, uint32_t Timeout = -1){}
        static void give(void * sema) {}
        static void giveFromISR(void * sema) {}
        static void createThread(
            void **      handle, 
            const char * taskName, 
            void      (* invoke)(void *), 
            void       * arg,
            uint32_t     stackSize, 
            uint32_t     priority){}
        static void scheduler(){}
        static void malloc(void ** ptr, uint32_t size){}
        static void free(void * ptr){}
    };
#endif

