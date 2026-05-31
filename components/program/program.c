#include "program.h"
#include "freertos/FreeRTOS.h" // IWYU pragma: keep — required before task.h
#include "freertos/task.h"
#include "portmacro.h"

// program_Main() is the FreeRTOS task body and must never return — it has to run
// forever by itself (loop, or end with vTaskDelete(NULL)), or the kernel aborts.
void program_Main(void *pvParameters)
{
    // Main program logic here

    while (true)
    {
        vTaskDelay(portMAX_DELAY);
    }

    // Unreachable here, but the correct way to end a task: self-delete rather
    // than return (returning makes the kernel abort).
    vTaskDelete(NULL);
}

void program_OnDestroy()
{
    // Cleanup code here
}