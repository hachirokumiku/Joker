#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define LOCK_PIN 7  // Lock control pin

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

struct RFID_Card {
    byte UID[4];
    const char* name;
};

// Registered Cards
RFID_Card knownCards[] = {
    {{0xB3, 0xBD, 0x3F, 0xFC}, "Joker Card"},
    {{0x04, 0x2A, 0x4F, 0x6A}, "Skeleton Key"},
    {{0xE7, 0xBF, 0xF5, 0xB3}, "Guest Key"}
};

bool lockState = true;  // true = locked, false = unlocked
unsigned long unlockTime = 0;

void setup() {
    Serial.begin(9600);
    SPI.begin();
    rfid.PCD_Init();
    lcd.begin();
    lcd.backlight();

    pinMode(LOCK_PIN, OUTPUT);
    digitalWrite(LOCK_PIN, HIGH);  // Start in locked state

    displayMessage("Scan Your Card");
}

void loop() {
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
        // Auto re-lock after 15 seconds
        if (!lockState && millis() - unlockTime >= 15000) {
            lockDoor();
        }
        return;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scanning...");

    const char* cardName = identifyCard(rfid.uid.uidByte);

    if (cardName != nullptr) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(cardName);
        lcd.setCursor(0, 1);

        if (lockState) {
            lcd.print("Door Unlocked");
            unlockDoor();
        } else {
            lcd.print("Door Locked");
            lockDoor();
        }
    } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access Denied");
        delay(2000);
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}

void unlockDoor() {
    Serial.println("Door Unlocked");
    digitalWrite(LOCK_PIN, LOW);  
    lockState = false;
    unlockTime = millis();  
}

void lockDoor() {
    Serial.println("Door Locked");
    digitalWrite(LOCK_PIN, HIGH);  
    lockState = true;
    displayMessage("Scan Your Card");
}

void displayMessage(const char* message) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
}

const char* identifyCard(byte* uid) {
    for (RFID_Card card : knownCards) {
        if (memcmp(uid, card.UID, 4) == 0) {
            return card.name;
        }
    }
    return nullptr;
}
