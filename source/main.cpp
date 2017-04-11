#include "mbed.h"
#include "../mbed-kinetis-lowpower/kinetis_lowpower.h"
#include "../mbed-os-quectelM66-driver/M66Interface.h"
#include "config.h"
#include "http_request.h"

static const char *const message_template = "{\"v\":\"0.0.0\",\"a\":\"%s\",\"p\":{\"t\":1}}";


void dump_response(HttpResponse* res) {
    printf("Status: %d - %s\n", res->get_status_code(), res->get_status_message().c_str());

    printf("Headers:\n");
    for (size_t ix = 0; ix < res->get_headers_length(); ix++) {
        printf("\t%s: %s\n", res->get_headers_fields()[ix]->c_str(), res->get_headers_values()[ix]->c_str());
    }
    printf("\nBody (%d bytes):\n\n%s\n", res->get_body_length(), res->get_body_as_string().c_str());
}

void sendMotionEvent() {
    M66Interface modem(GSM_UART_TX, GSM_UART_RX, GSM_PWRKEY, GSM_POWER, true);
    TCPSocket *socket = new TCPSocket();

    //if(!modem.isModemAlive()) modem.powerUpModem();


    // connect modem
    int r = modem.connect(CELL_APN, CELL_USER, CELL_PWD);


    // Create a TCP socket
    printf("\n----- Setting up TCP connection -----\r\n");

    char theIP[20];
    bool ipret = modem.queryIP("api.demo.dev.ubirch.com", theIP);

    nsapi_error_t open_result = socket->open(&modem);

    if (open_result != 0) {
        printf("Opening TCPSocket failed... %d\n", open_result);
        return;
    }

    nsapi_error_t connect_result = socket->connect(theIP, 8080);
    if (connect_result != 0) {
        printf("Connecting over TCPSocket failed... %d\n", connect_result);
        return;
    }

    // add the auth key to the message
    const char *AUTH_KEY = "XPL7H+/Q5hlp8JDm0n7JhK7jYmoEnjVk+CLGN10owj7EJx8hGWcVdlvxL+t7rQ/2N8I2qlICLCKnve9GcPQGYg==";
    int message_size = snprintf(NULL, 0, message_template, AUTH_KEY);
    char *message = (char *) malloc((size_t) (message_size + 1));
    sprintf(message, message_template, AUTH_KEY);

    // POST request to api.demo.dev.ubirch.com
    {
        HttpRequest *post_req = new HttpRequest(socket, HTTP_POST,
                                                "http://api.demo.dev.ubirch.com/api/avatarService/v1/device/update");
        post_req->set_header("Content-Type", "application/json");

        HttpResponse *post_res = post_req->send(message, strlen(message));
        if (!post_res) {
            printf("HttpRequest failed (error code %d)\n", post_req->get_error());
            return;
        }

        printf("\n----- HTTP POST response -----\n");
        dump_response(post_res);

        free(message);

        delete post_req;
    }
    delete socket;

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