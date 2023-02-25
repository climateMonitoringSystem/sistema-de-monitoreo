// For the DHT sensor
#include <DHT.h>
#include <DHT_U.h>
// For the I2C adapter
#include <LiquidCrystal_I2C.h>
// For the ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
// For the Json File
#include <ArduinoJson.h>
#include <Thread.h>

#define LDR A0

const char *ssid = "BetelFO(3491)";                                            // Network ssid
const char *password = "rocky@bruno2020";                                      // Network password
const char *host = "http://climatemonitoringsystem.pythonanywhere.com/record"; // URL server

LiquidCrystal_I2C lcd(0x27, 16, 2); // creating object lcd
DHT dht(D4, DHT11);                 //  creating object dht
HTTPClient http;                    // creating object http
WiFiClient wifiClient;              // creating object  wifi client

// Variables to store the record
float temperature, humidity;
int light;

Semaphore temperatureHumiditySemaphore(1);
Semaphore lightSemaphore(1);

Thread threadHumidity;
Thread threadTemperature;
Thread threadLight;


// Function to read the humidity
void readHumidity()
{
    while (true)
    {
        temperatureHumiditySemaphore.take();
        humidity = dht.readHumidity();
        temperatureHumiditySemaphore.give();
        delay(2000);
    }
}

// Function to read the Temperature
void readTemperature()
{
    while (true)
    {
        temperatureHumiditySemaphore.take();
        temperature = dht.readTemperature();
        temperatureHumiditySemaphore.give();
        delay(2000);
    }
}

// Function to read the light
void readLight()
{
    while (true)
    {
        lightSemaphore.take();
        light = analogRead(LDR);
        lightSemaphore.give();
        delay(2000);
    }
}

void setup()
{

    Serial.begin(9600);

    // Initializing lcd
    lcd.init();
    lcd.begin(16, 2);
    lcd.backlight();
    lcd.clear();
    // Setting the cursor at position 0, 0 on the display
    lcd.setCursor(0, 0);
    // Printing message
    lcd.print("Connecting...");

    // Trying to connect to the network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        // Delay of 500 milliseconds
        delay(500);
    }

    String ip = WiFi.localIP().toString();

    // cleaning the screen
    lcd.clear();
    // Setting the cursor at position 0, 0 on the display
    lcd.setCursor(0, 0);
    lcd.print("Connected");
    // Setting the cursor at position 0, 1 on the display
    lcd.setCursor(0, 1);
    lcd.print("IP: " + ip);
    // Delay of 2000 milliseconds = 2 seconds
    delay(2000);

    // Starting threads to read humidity, temperature and light in parallel
    threadHumidity.onRun(readHumidity);
    threadTemperature.onRun(readTemperature);
    threadLight.onRun(readLight);

    threadHumidity.setInterval(2000);
    threadTemperature.setInterval(2000);
    threadLight.setInterval(2000);

    threadHumidity.onRun();
    threadTemperature.onRun();
    threadLight.onRun();
}

void loop()
{

    // Printing record
    //
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(String(temperature)+ "C " + String(humidity) + "%HC");
    lcd.setCursor(0, 1);
    lcd.print(" Light: " + String(light));

    // Checking network connection
    if (WiFi.status() == WL_CONNECTED)
    {
        // Wait for the temperature and humidity semaphore
        temperatureHumiditySemaphore.take();
        // Store the temperature and humidity in variables
        float currentTemperature = temperature;
        float currentHumidity = humidity;
        // Release the temperature and humidity semaphore
        temperatureHumiditySemaphore.give();

        // Wait for the light semaphore
        lightSemaphore.take();
        // Store the light in a variable
        int currentLight = light;
        // Release the light semaphore
        lightSemaphore.give();

        http.begin(wifiClient, host);                       // Init connection
        http.addHeader("Content-Type", "application/json"); // Adding header

        // Creating Json Document
        DynamicJsonDocument doc(2048);
        doc["temperature"] = currentTemperature;
        doc["humidity"] = currentHumidity;
        doc["light"] = currentLight;
        // Serialize JSON document
        String json;
        serializeJson(doc, json);

        // Sending request
        int httpCode = http.POST(json);
        // Ending connection
        http.end();
    }

    // Delay of 2 seconds
    delay(2000);
}
