#include "mbed.h"
#include "../mbed-kinetis-lowpower/kinetis_lowpower.h"
#include "../mbed-os-quectelM66-driver/M66Interface.h"
#include "config.h"

static const char *const message_template = "POST /api/avatarService/v1/device/update HTTP/1.1\r\n"
"Host: api.demo.dev.ubirch.com:8080\r\n"
"Content-Length: 120\r\n"
"\r\n"
"{\"v\":\"0.0.0\",\"a\":\"%s\",\"p\":{\"t\":1}}";


void sendMotionEvent() {
    M66Interface modem(GSM_UART_TX, GSM_UART_RX, GSM_PWRKEY, GSM_POWER, true);
    TCPSocket socket;

    //if(!modem.isModemAlive()) modem.powerUpModem();
    
    // connect modem
    int r = modem.connect(CELL_APN, CELL_USER, CELL_PWD);
    // make sure we actually connected
    if (r == NSAPI_ERROR_OK) {
        printf("MODEM CONNECTED\r\n");
        socket.open(&modem);

        // http://api.demo.dev.ubirch.com:8080/api/avatarService/v1/device/update
        socket.connect("api.demo.dev.ubirch.com", 8080);

        // add the auth key to the message
        int message_size = snprintf(NULL, 0, message_template, AUTH_KEY);
        char *message = (char *) malloc((size_t) (message_size + 1));
        sprintf(message, message_template, AUTH_KEY);

        r = socket.send(message, strlen(message));
        if (r > 0) {
            // receive a simple http response and print out the response line
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

        free(message);

        // Close the socket to return its memory and bring down the network interface
        socket.close();
    }

    modem.disconnect();
    modem.powerDown();
}


int main(void) {
    DigitalOut led(LED1);
    led = 1;

    // print message on boot (w/ indication of wakeup)
    printf("Motion Detect v1.0 (%s)\r\n", isLowPowerWakeup() ? "wakeup" : "power on");
    // send a motion event if we woke up from low power sleep
    if (isLowPowerWakeup()) sendMotionEvent();

    led = 0;

    // power down and wait for pin event, we need an interrupt in pin for that
    InterruptIn pinWakeup(PTC3);
    powerDownWakeupOnPin();

    while(1) {
        led = !led;
        wait_ms(100);
    }
}