/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Supporting functions include file */
#include "supporting_functions/supporting_functions.h"

/* Must be greater than any priority used by the kernel and less than 32 */
#define portINTERRUPT_YIELD 5

BaseType_t xHigherPriorityTaskWoken = pdFALSE;

/* The task functions. */
void vTask1(void *pvParameters);
void vTask2(void *pvParameters);

/* Handles for the tasks create by main_app(). */
static TaskHandle_t xTask1 = NULL, xTask2 = NULL;

/* Define the strings that will be passed in as the task parameters. */
const char *pcTextForTask1 = "Task 1 is running\r\n";
const char *pcTextForTask2 = "Task 2 is running\r\n";
/*-----------------------------------------------------------*/

static unsigned long prvProcessYieldInterrupt(void)
{

	/* xHigherPriorityTaskWoken must be initialised to pdFALSE.  If calling
	vTaskNotifyGiveFromISR() unblocks the handling task, and the priority of
	the handling task is higher than the priority of the currently running task,
	then xHigherPriorityTaskWoken will automatically get set to pdTRUE. */
	xHigherPriorityTaskWoken = pdFALSE;

	/* Send a notification to Task 2 and bring it out of the Blocked
	state. */
	vTaskNotifyGiveFromISR(xTask2, &xHigherPriorityTaskWoken);

	vPrintString((char *)"Notification was sent to Task2.\r\n");

	/* Switch to the highest-priority task */
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
/*-----------------------------------------------------------*/

int main_app(void)
{
	/* Create two tasks and pass string parameters to the task functions. */
	xTaskCreate(vTask1, "Task 1", 100, (void*)pcTextForTask1, 1, &xTask1);	
	xTaskCreate(vTask2, "Task 2", 100, (void*)pcTextForTask2, 2, &xTask2);

	/* Set the simulated ISR using the handler */
	vPortSetInterruptHandler(portINTERRUPT_YIELD, prvProcessYieldInterrupt);

	/* Start the scheduler to start the tasks executing. */
	vTaskStartScheduler();
}
/*-----------------------------------------------------------*/

void vTask1(void *pvParameters)
{
	char *pcTaskString;
	pcTaskString = (char *)pvParameters;

	for ( ;; )
	{
		vTaskDelay((TickType_t)500); //Delay for 500 ticks
		vPrintString(pcTaskString);

		/* Simulate the periodic interrupt hanlded by 
		   prvProcessYieldInterrupt. */
		portYIELD();
	}
}
/*-----------------------------------------------------------*/

void vTask2(void *pvParameters)
{
	char *pcTaskString;
	pcTaskString = (char *)pvParameters;

	for ( ;; )
	{
		vPrintString(pcTaskString);

		/* Block to wait for vTask1() to notify this task. */
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		vPrintString((char *)"Task2 was notified!\r\n");
	}
}
