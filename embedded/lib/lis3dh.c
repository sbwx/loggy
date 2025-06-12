/** 
**************************************************************
* @file embedded/lib/lis3dh.c
* @author Shane Baptist - 47484164
* @date 17/05/2025
* @brief Accelerometer driver (Channel 5-7)
*************************************************************** 
*/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/sensor.h>
#include "lis3dh.h"

// Define threads
K_THREAD_DEFINE(accel_tid, ACCEL_STACK_SIZE, accel_thread, NULL, NULL, NULL, ACCEL_THREAD_PRIORITY, 0, 0);

// Define message queue
K_MSGQ_DEFINE(accelX_msgq, sizeof(int32_t), 1, 1);
K_MSGQ_DEFINE(accelY_msgq, sizeof(int32_t), 1, 1);
K_MSGQ_DEFINE(accelZ_msgq, sizeof(int32_t), 1, 1);

// Get accelerometer device
static const struct device *const lis3dh = DEVICE_DT_GET(DT_NODELABEL(accel));

// Read accelerometer thread
void accel_thread() {
    static unsigned int count;
    struct sensor_value accel[3];
    const char *overrun = "";
    int rc;
    int32_t xAccel;
    int32_t yAccel;
    int32_t zAccel;

    // Check device exists
    if (lis3dh == NULL) {
        printk("No device found\n");
        return;
    }
    // Check device ready
    if (!device_is_ready(lis3dh)) {
        printk("Accelerometer is not ready\n");
        return;
    }

    while (1) {
        // Get sample
        rc = sensor_sample_fetch(lis3dh);
        ++count;
        if (rc == -EBADMSG) {
            /* Sample overrun.  Ignore in polled mode. */
            if (IS_ENABLED(CONFIG_LIS2DH_TRIGGER)) {
                overrun = "[OVERRUN] ";
            }
            rc = 0;
        }
        // Get accelerometer reading
        if (rc == 0) {
            rc = sensor_channel_get(lis3dh, SENSOR_CHAN_ACCEL_XYZ, accel);
        }
        if (rc < 0) {
            printk("ERROR: Update failed: %d\n", rc);
        } 

        // Convert reading to float
        xAccel = sensor_value_to_milli(&accel[0]);
        yAccel = sensor_value_to_milli(&accel[1]);
        zAccel = sensor_value_to_milli(&accel[2]);

        // Put in message queues
        k_sched_lock();
        k_msgq_purge(&accelX_msgq);
        k_msgq_put(&accelX_msgq, &xAccel, K_NO_WAIT);

        k_msgq_purge(&accelY_msgq);
        k_msgq_put(&accelY_msgq, &yAccel, K_NO_WAIT);

        k_msgq_purge(&accelZ_msgq);
        k_msgq_put(&accelZ_msgq, &zAccel, K_NO_WAIT);
        k_sched_unlock();

        // Sampling delay
        k_msleep(ACCEL_SAMPLING_RATE);
    }
}