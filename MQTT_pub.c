#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <pthread.h>
#include <string.h>
#include "MQTTClient.h"

#define ADDRESS "test.mosquitto.org"
#define CLIENTID "ClientPubE"
#define TOPIC "kwak"

#define OUT 4
#define SPI_CH 0
#define ADC_CH 5
#define ADC_CH 3
#define ADC_CS 29
#define SPI_SPEED 500000
#define DHTPIN 25
#define MAXTIMINGS 100
int dht11_dat[5] = {0, 0, 0, 0, 0};

void *soil()
{
    printf("soil\n");
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg_soil = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    int value = 0, i;
    unsigned char buf[3];
    if (wiringPiSetup() == -1)
        return 1;
    if (wiringPiSPISetup() == -1)
        return -1;
    pinMode(ADC_CS, OUTPUT);

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    while (free)
    {
        buf[0] = 0x06 | ((ADC_CH & 0x04) >> 2);
        buf[1] = ((ADC_CH & 0x03) << 6);
        buf[2] = 0x00;
        digitalWrite(ADC_CS, 0);
        wiringPiSPIDataRW(SPI_CH, buf, 3);
        buf[1] = 0x0F & buf[1];
        value = (buf[1] << 8) | buf[2];
        digitalWrite(ADC_CS, 1);
        printf("soil=%d\n", value);
        char msg[5];
        sprintf(msg1, "%d", value);
        pubmsg_soil.payload = msg;
        pubmsg_soil.payloadlen = (int)strlen(msg);
        MQTTClient_publishMessage(client, "Soil", &pubmsg_soil, &token);
        delay(1000);
    }

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
void *dust()
{
    printf("Dust\n");
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg_dust = MQTTClient_message_initializer;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_deliveryToken token;
    int rc;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    int value = 0, i;
    unsigned char buf[3];

    if (wiringPiSetup() == -1)
        return 1;

    if (wiringPiSPISetup() == -1)
        return -1;

    pinMode(ADC_CS, OUTPUT);
    pinMode(OUT, OUTPUT);

    while (free)
    {
        digitalWrite(OUT, LOW);
        delayMicroseconds(280);

        buf[0] = 0x06 | ((ADC_CH & 0x04) >> 2);
        buf[1] = ((ADC_CH & 0x03) << 6);
        buf[2] = 0x00;

        digitalWrite(ADC_CS, 0);

        wiringPiSPIDataRW(SPI_CH, buf, 3);

        buf[1] = 0x0F & buf[1];

        value = (buf[1] << 8) | buf[2];

        digitalWrite(ADC_CS, 1);

        delayMicroseconds(40);
        digitalWrite(OUT, HIGH);
        delayMicroseconds(9680);

        printf("dust=%d\n", value);
        char msg[5];
        sprintf(msg, "%d", value);
        pubmsg_dust.payload = msg;
        pubmsg_dust.payloadlen = (int)strlen(msg);
        MQTTClient_publishMessage(client, "Dust", &pubmsg_dust, &token);

        delay(100);
    }
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}

void *DHT()
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;  //humidity
    MQTTClient_message pubmsg1 = MQTTClient_message_initializer; //temperature
    MQTTClient_deliveryToken token;
    int rc;

    u_int8_t laststate = HIGH;
    u_int8_t counter = 0;
    u_int8_t j = 0, i;
    float f;
    if (wiringPiSetup() == -1)
        return 1;

    dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;

    pinMode(DHTPIN, OUTPUT);
    digitalWrite(DHTPIN, LOW);
    delay(18);
    digitalWrite(DHTPIN, HIGH);
    delayMicroseconds(40);
    pinMode(DHTPIN, INPUT);

    MQTTClient_create(&client, ADDRESS, CLIENTID,
                      MQTTCLIENT_PERSISTENCE_NONE, NULL);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < MAXTIMINGS; i++)
    {
        counter = 0;
        while (digitalRead(DHTPIN) == laststate)
        {
            counter++;
            delayMicroseconds(1);
            if (counter == 255)
            {
                break;
            }
        }
        laststate = digitalRead(DHTPIN);

        if (counter == 255)
            break;

        if ((i >= 4) && (i % 2 == 0))
        {
            dht11_dat[j / 8] <<= 1;
            if (counter > 50)
                dht11_dat[j / 8] |= 1;
            j++;
        }
    }
    char humidity[1];
    char temperature[1];

    int b = dht11_dat[2] + dht11_dat[3] / 100; //Temperature

    if ((j >= 40) &&
        (dht11_dat[4] == ((dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF)))
    {
        f = dht11_dat[2] * 9. / 5. + 32;

        int a = dht11_dat[0]; //humidity
        int b = dht11_dat[2];
        printf("%d\n", a);
        printf("%d\n", b);
        sprintf(humidity, "%d", a);
        sprintf(temperature, "%d", b);
        pubmsg.payload = humidity;
        pubmsg1.payload = temperature;
        MQTTClient_publishMessage(client, "Humidity", &pubmsg, &token);
        MQTTClient_publishMessage(client, "Temperature", &pubmsg1, &token);
    }
    else
    {
        printf("Error");
    }

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}

int main()
{

    pthread_t pthread[3];

    if (wiringPiSetup() == -1)
        return 1;

    pthread_create(&pthread[0], NULL, soil, NULL);
    pthread_create(&pthread[1], NULL, dust, NULL);
    pthread_create(&pthread[2], NULL, DHT, NULL);

    pthread_join(pthread[0], NULL);
    pthread_join(pthread[1], NULL);
    pthread_join(pthread[2], NULL);

    return 0;
}
