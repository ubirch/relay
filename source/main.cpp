#include "mbed.h"
#include <M66Interface.h>
#include <fsl_smc.h>
#include <fsl_rcm.h>
#include <fsl_llwu.h>
#include <fsl_port.h>

static const short MOTION_DETECTED = 0x01;

M66Interface modem(GSM_UART_TX, GSM_UART_RX, GSM_PWRKEY, GSM_POWER, true);
InterruptIn movement(PTC4);
DigitalOut extPower(PTC8);
DigitalOut led(LED1);
Thread sendThread(osPriorityNormal, 30 * 1024);

void trigger(void) {
    sendThread.signal_set(MOTION_DETECTED);
}


void sendData(void) {
    TCPSocket socket;

    while (true) {
        Thread::signal_wait(MOTION_DETECTED);
        printf("MOTION DETECTED\r\n");

        // Open a socket on the network interface, and create a TCP connection to www.arm.com
        socket.open(&modem);

        // http://api.demo.dev.ubirch.com:8080/api/avatarService/v1/device/update
        socket.connect("api.demo.dev.ubirch.com", 8080);

        const char *message =
        "POST /api/avatarService/v1/device/update HTTP/1.1\r\n"
        "Host: api.demo.dev.ubirch.com:8080\r\n"
        "Content-Length: 120\r\n"
        "\r\n"
        "{\"v\":\"0.0.0\",\"a\":\"lddGNIlYwKFrWQf1DtfVY09f7yxUTXJUiQ9YlmJyEIKR1IuBV4MCC4aiY6698Z8rHhVBkZ15YozafqCIY9aJIA==\",\"p\":{\"t\":1}}";

        int r = socket.send(message, strlen(message));
        if (r > 0) {
            // Recieve a simple http response and print out the response line
            char buffer[64];
            r = socket.recv(buffer, sizeof(buffer));
            if (r >= 0) {
                printf("received %d bytes\r\n---\r\n%.*s\r\n---\r\n", r, (int) (strstr(buffer, "\r\n") - buffer),
                       buffer);
            } else {
                printf("receive failed: %d\r\n", r);
            }
        } else {
            printf("send failed: %d\r\n", r);
        }

        // Close the socket to return its memory and bring down the network interface
        socket.close();
    }
}

/*!
 * @brief LLWU interrupt handler.
 */
void LLWU_IRQHandler(void)
{
    /* If wakeup by external pin. */
    if (LLWU_GetExternalWakeupPinFlag(LLWU, 3U))
    {
        PORT_SetPinInterruptConfig(PORTA, FSL_FEATURE_LLWU_PIN3_GPIO_PIN, kPORT_InterruptOrDMADisabled);
        PORT_ClearPinsInterruptFlags(PORTA, (1U << FSL_FEATURE_LLWU_PIN3_GPIO_PIN));
        LLWU_ClearExternalWakeupPinFlag(LLWU, 3U);
    }
}


void shutdown() {
    PORT_SetPinInterruptConfig(PORTA, PTA4, kPORT_InterruptFallingEdge);
    LLWU_SetExternalWakeupPinMode(LLWU, 3U, kLLWU_ExternalPinFallingEdge);
    NVIC_EnableIRQ(LLWU_IRQn);

    // power down
    smc_power_mode_vlls_config_t vlls_config; /* Local variable for vlls configuration */
    vlls_config.subMode = kSMC_StopSub0;
    vlls_config.enablePorDetectInVlls0 = true;
    vlls_config.enableLpoClock = false;
    SMC_PreEnterStopModes();
    SMC_SetPowerModeVlls(SMC, &vlls_config);
    SMC_PostExitStopModes();
}

extern rcm_reset_source_t wakeupSource;

int main(void) {
    // print message on cold boot
    if (wakeupSource == RCM_GetPreviousResetSources(RCM)) {
        printf("low power wakeup");
    } else {
        printf("Motion Detect v1.0\r\n");
    }

    led = 1;
    Thread::wait(1000);

//    // power on external system and wait for bootup
//    extPower.write(1);
//    Thread::wait(100);
//
//    // connect modem
//    const int r = modem.connect("eseye.com", "ubirch", "internet");
//    // make sure we actually connected
//    if (r == NSAPI_ERROR_OK) {
//        printf("MODEM CONNECTED\r\n");
//        // start sender thread
//        sendThread.start(callback(sendData));
//        // register signal trigger
//        movement.rise(&trigger);
//        // just loop around
//    } else {
//        printf("MODEM CONNECT FAILED\r\n");
//        // just loop around
//    }

    led = 0;

    shutdown();
}