# etask_rgbpanel_clock

This package contains the code to show a clock on an RGB panel
It also provides a FreeRTOS task `etask_rgbpanel_clock()`, which can run as part of your project.

Include this repository as a submodule in your project, or just download as a library.

## Example excerpt

~~~c
#include <etask_rgbpanel_clock.h>

TaskHandle_t rgbpanel_task;

void setup() {
  ...
  // Create a task to check and get OTA updates from S3
  xTaskCreate(
    etask_rgbpanel_clock,
    "RGBPanel_Task",
    5000,
    NULL,
    0,
    &rgbpanel_task
  );
}
~~~
