#include "mbed.h"
//#include <M66Interface.h>
//#include <config.h>
#include <fsl_smc.h>
#include <fsl_rcm.h>
#include <fsl_llwu.h>
#include <fsl_port.h>
#include <fsl_pmc.h>

static const short MOTION_DETECTED = 0x01;

//M66Interface modem(GSM_UART_TX, GSM_UART_RX, GSM_PWRKEY, GSM_POWER, true);
InterruptIn movement(PTC3);
DigitalOut extPower(PTC8);
DigitalOut led(LED1);
//Thread sendThread(osPriorityNormal, 30 * 1024);

static const char *const message_template = "POST /api/avatarService/v1/device/update HTTP/1.1\r\n"
"Host: api.demo.dev.ubirch.com:8080\r\n"
"Content-Length: 120\r\n"
"\r\n"
"{\"v\":\"0.0.0\",\"a\":\"%s\",\"p\":{\"t\":1}}";

//void trigger(void) {
//    sendThread.signal_set(MOTION_DETECTED);
//}


//void sendData(void) {
//    TCPSocket socket;
//
//    while (true) {
//        Thread::signal_wait(MOTION_DETECTED);
//        printf("MOTION DETECTED\r\n");
//
//        // Open a socket on the network interface, and create a TCP connection to www.arm.com
//        socket.open(&modem);
//
//        // http://api.demo.dev.ubirch.com:8080/api/avatarService/v1/device/update
//        socket.connect("api.demo.dev.ubirch.com", 8080);

//        int message_size = snprintf(NULL, 0, message_template, imeiHash);
//        char *message = (char *) malloc((size_t) (message_size + 1));
//        sprintf(message, message_template, imeiHash);

//        int r = socket.send(message, strlen(message));
//        if (r > 0) {
//            // Recieve a simple http response and print out the response line
//            char buffer[64];
//            r = socket.recv(buffer, sizeof(buffer));
//            if (r >= 0) {
//                printf("received %d bytes\r\n---\r\n%.*s\r\n---\r\n", r, (int) (strstr(buffer, "\r\n") - buffer),
//                       buffer);
//            } else {
//                printf("receive failed: %d\r\n", r);
//            }
//        } else {
//            printf("send failed: %d\r\n", r);
//        }
//
//        free(message);
//
//        // Close the socket to return its memory and bring down the network interface
//        socket.close();
//    }
//}

#define LLWU_WAKEUP_PIN_IDX 7U /* LLWU_P7 */
#define LLWU_WAKEUP_PIN_TYPE kLLWU_ExternalPinFallingEdge //kLLWU_ExternalPinAnyEdge

#define BOARD_WAKEUP_GPIO GPIOC
#define BOARD_WAKEUP_PORT PORTC
#define BOARD_WAKEUP_GPIO_PIN 3U
#define BOARD_WAKEUP_IRQ PORTC_IRQn

/*!
 * @brief LLWU interrupt handler.
 */

void LLWU_IRQHandler(void) {
    /* If wakeup by external pin. */
    if (LLWU_GetExternalWakeupPinFlag(LLWU, LLWU_WAKEUP_PIN_IDX)) {
        PORT_SetPinInterruptConfig(BOARD_WAKEUP_PORT, BOARD_WAKEUP_GPIO_PIN, kPORT_InterruptOrDMADisabled);
        PORT_ClearPinsInterruptFlags(BOARD_WAKEUP_PORT, (1U << BOARD_WAKEUP_GPIO_PIN));
        LLWU_ClearExternalWakeupPinFlag(LLWU, LLWU_WAKEUP_PIN_IDX);
    }
}

void WDOG_EWM_IRQHandler(void) { //Watchdog_IRQHandler(void){
//    while(1);
}

void shutdown() {

//    PORT_SetPinInterruptConfig(PORTA, PTA4, kPORT_InterruptFallingEdge);
    LLWU_SetExternalWakeupPinMode(LLWU, LLWU_WAKEUP_PIN_IDX, LLWU_WAKEUP_PIN_TYPE);
    printf("enabled the irqs\r\n");
    NVIC_EnableIRQ(LLWU_IRQn);

    wait(1);
    PORT_SetPinMux(PORTB, 16U, kPORT_PinDisabledOrAnalog);

    // power down
    smc_power_mode_vlls_config_t vlls_config;
    vlls_config.enablePorDetectInVlls0 = true;
    vlls_config.enableRam2InVlls2 = true; /*!< Enable RAM2 power in VLLS2 */
    vlls_config.enableLpoClock = true;    /*!< Enable LPO clock in VLLS mode */

    smc_power_mode_lls_config_t lls_config;
    lls_config.enableLpoClock = true;
    lls_config.subMode = kSMC_StopSub3;

    vlls_config.subMode = kSMC_StopSub3;
    SMC_PreEnterStopModes();
    SMC_SetPowerModeVlls(SMC, &vlls_config);
    SMC_PostExitStopModes();
}

extern rcm_reset_source_t wakeupSource;

void ShowPowerMode(smc_power_state_t powerMode) {
    switch (powerMode) {
        case kSMC_PowerStateRun:
            printf("    Power mode: RUN\r\n");
            break;
        case kSMC_PowerStateVlpr:
            printf("    Power mode: VLPR\r\n");
            break;
        case kSMC_PowerStateHsrun:
            printf("    Power mode: HSRUN\r\n");
            break;
        default:
            printf("    Power mode wrong\r\n");
            break;
    }
}

//volatile uint32_t milliseconds = 0;
//bool on = true;
//
//void SysTick_Handler() {
//    milliseconds++;
//    if (milliseconds % 1000 == 0) on = !on;
//    led = 1;
//}

int main(void) {
    uint32_t sec;
    uint32_t currSeconds;
    uint32_t freq = 0;
    smc_power_state_t curPowerState;

    printf("############the power modes ##########\r\n");
//    SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeAll);
//    if (kRCM_SourceWakeup & RCM_GetPreviousResetSources(RCM)) /* Wakeup from VLLS. */
//    {
//        PMC_ClearPeriphIOIsolationFlag(PMC);
//        NVIC_ClearPendingIRQ(LLWU_IRQn);
//    }
//
////     print message on cold boot
    if (wakeupSource & kRCM_SourceWakeup) /* Wakeup from VLLS. */
    {
        printf("    \r\nMCU wakeup from VLLS modes...\r\n");
    } else {
        printf("Motion Detect v1.0\r\n");
    }

    curPowerState = SMC_GetPowerModeState(SMC);
    freq = CLOCK_GetFreq(kCLOCK_CoreSysClk);
    ShowPowerMode(curPowerState);
    printf("    Core Clock = %dHz \r\n", (int) freq);

    led = 1;

    wait(4);

//    NVIC_DisableIRQ(SysTick_IRQn);

    NVIC_SetVector(LLWU_IRQn, (uint32_t) &LLWU_IRQHandler);
    NVIC_EnableIRQ(LLWU_IRQn);

    LLWU_SetExternalWakeupPinMode(LLWU, LLWU_WAKEUP_PIN_IDX, LLWU_WAKEUP_PIN_TYPE);

    //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    printf("now let me sleep2\r\n\n\n");

    led = 0;
    smc_power_mode_vlls_config_t vlls_config;
    vlls_config.enablePorDetectInVlls0 = true;
    vlls_config.enableRam2InVlls2 = true; /*!< Enable RAM2 power in VLLS2 */
    vlls_config.enableLpoClock = true;    /*!< Enable LPO clock in VLLS mode */

    smc_power_mode_lls_config_t lls_config;
    lls_config.enableLpoClock = true;
    lls_config.subMode = kSMC_StopSub3;

    vlls_config.subMode = kSMC_StopSub0;
    SMC_PreEnterStopModes();
    SMC_SetPowerModeVlls(SMC, &vlls_config);
    SMC_PostExitStopModes();

    while (1) {
        led = !led;
        Thread::wait(1000);
    }
//    // power on external system and wait for bootup
//    extPower.write(1);
//    Thread::wait(100);
//
//    // connect modem
//    const int r = modem.connect(CELL_APN, CELL_USER, CELL_PWD);
//    // make sure we actually connected
//    if (r == NSAPI_ERROR_OK) {
//        printf("MODEM CONNECTED\r\n");
//        // start sender thread
//        sendThread.start(callback(sendData));
//        // register signal trigger
//        movement.rise(&trigger);
//        // just loop around
//        while (1) {
//            led = !led;
//            wait(1.0);
//        }
//    } else {
//        printf("MODEM CONNECT FAILED\r\n");
//        // just loop around
//        while (1) {
//            led = !led;
//            wait(0.15);
//        }
//    }
}