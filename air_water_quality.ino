#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial GSM(6, 7); // RX, TX for GSM

char phone_no1[] = "+910000000000"; // Replace with actual number
char phone_no2[] = "+910000000000"; // Replace with actual number

#define SENSOR_PIN A0
#define SMOKE_PIN 2
#define BUZZER_PIN 3

void setup() {
    pinMode(SENSOR_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(SMOKE_PIN, INPUT);

    Serial.begin(9600);
    GSM.begin(9600);

    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("AIR & WATER MONITOR");
    delay(2000);
    lcd.clear();

    Serial.println("Initializing GSM...");
    initModule("AT", "OK", 2000);
    initModule("ATE1", "OK", 2000);
    initModule("AT+CPIN?", "READY", 2000);
    initModule("AT+CMGF=1", "OK", 2000); // Set SMS text mode
    Serial.println("GSM Initialized");

    lcd.setCursor(0, 0);
    lcd.print("GSM INITIALIZED");
    delay(2000);
    lcd.clear();

    sendSMS(phone_no1, "System started successfully");
    sendSMS(phone_no2, "System started successfully");
}

void loop() {
    // Air quality check
    if (digitalRead(SMOKE_PIN) == 0) {
        lcd.setCursor(0, 0);
        lcd.print("AQI >40% - Poor Air ");
        digitalWrite(BUZZER_PIN, HIGH);
        sendSMS(phone_no1, "Alert! Poor air quality detected");
        sendSMS(phone_no2, "Alert! Poor air quality detected");
        delay(2000);
    } else {
        lcd.setCursor(0, 0);
        lcd.print("AQI <40% - Clean Air");
        digitalWrite(BUZZER_PIN, LOW);
    }

    // Water turbidity check
    int read_ADC = analogRead(SENSOR_PIN);
    int ntu = map(read_ADC, 0, 208, 0, 100);

    lcd.setCursor(0, 1);
    lcd.print("Turbidity: ");
    lcd.print(ntu);
    lcd.print("   "); // Clear residue

    lcd.setCursor(12, 1);
    if (ntu < 10) {
        lcd.print("Clean ");
    } else if (ntu < 30) {
        lcd.print("Cloudy");
    } else {
        lcd.print("Dirty ");
        digitalWrite(BUZZER_PIN, HIGH);
        sendSMS(phone_no1, "Alert! Dirty water detected");
        sendSMS(phone_no2, "Alert! Dirty water detected");
        delay(500);
        digitalWrite(BUZZER_PIN, LOW);
    }

    delay(3000); // Delay before next cycle
}

void sendSMS(char *number, char *msg) {
    GSM.println("AT+CMGF=1"); // Set SMS mode to text
    delay(1000);

    GSM.print("AT+CMGS=\"");
    GSM.print(number);
    GSM.println("\"");
    delay(1000);

    GSM.print(msg);
    GSM.write(26); // CTRL+Z
    delay(5000);   // Wait for the message to be sent
}

void initModule(const char *cmd, const char *expected, int timeout) {
    GSM.println(cmd);
    long int time = millis();
    while ((millis() - time) < timeout) {
        while (GSM.available()) {
            String response = GSM.readString();
            if (response.indexOf(expected) != -1) {
                Serial.println(String(cmd) + " -> OK");
                return;
            }
        }
    }
    Serial.println(String(cmd) + " -> No Response");
}
