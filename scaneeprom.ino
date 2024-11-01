#include <SPI.h>
#include <PN532_SPI.h>
#include "PN532.h"
#include <EEPROM.h>

PN532_SPI pn532spi(SPI, 10);
PN532 nfc(pn532spi);

//  in SPI mode (jumper 0 - 1) and SCK MISO MOSI SS VCC GND

byte byte1;
uint8_t t1;
uint8_t t2;

void setup(void) {
  delay(1000);
  Serial.begin(115200);
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  } 

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  
  // delay(10000);
  int eeAddress = 0;
  for (int i=0; i<16; i++) { // Secteurs
    Serial.print("Serial ");
    Serial.print(i);
    Serial.println("-----------------------------");
    for (int k=0; k<4; k++) { // Blocs
      Serial.print("Bloc ");
      Serial.print(k);
      Serial.print(": ");
      for (int j=0; j<16; j++) { // Valeurs
          byte1 = EEPROM.read(eeAddress);
          
          if (byte1 <= 0xF) {
            Serial.print(0);
          }
          Serial.print(byte1, HEX);
          eeAddress += 1;
      }
      Serial.println();
    }
  }
          
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.update(i, 0);
  }

  nfc.begin();
  
  // configure board to read RFID tags
  nfc.SAMConfig();
}

void loop(void) {
  boolean success;                          // Flag to check if there was an error with the PN532
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint8_t currentblock;                     // Counter to keep track of which block we're on
  uint8_t currentsector;                    // Counter to keep track of which sector we're on
  bool authenticated = false;               // Flag to indicate if the sector is authenticated
  uint8_t data[16];                         // Array to store block data during reads
  int size = sizeof(data);
  int eeAddress = 0;                        // Location we want the data to be put.

  uint8_t key0_4[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  uint8_t key5_13[6] = { 0x6A, 0x19, 0x87, 0xC4, 0x0A, 0x21 };

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  
  if (success) {
    
    if (uidLength == 4) {
      for (currentblock = 0; currentblock < 64; currentblock++) {
        if (nfc.mifareclassic_IsFirstBlock(currentblock)) authenticated = false;

        if (!authenticated) {
          
          currentsector = (int)currentblock/4;
          if (currentsector <= 4) {
              success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentblock, 0, &key0_4[0]);
          } else {
              // This will be 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF for Mifare Classic (non-NDEF!)
              // or 0xD3 0xF7 0xD3 0xF7 0xD3 0xF7 for NDEF formatted cards using key a,
              // but keyb should be the same for both (0xFF 0xFF 0xFF 0xFF 0xFF 0xFF)
              success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentblock, 0, &key5_13[0]);
          }
          
          if (success) {
            authenticated = true;
          } 
        }
        if (authenticated) {
          success = nfc.mifareclassic_ReadDataBlock(currentblock, data);
          
          if (success) {
            Serial.println();
            for (int i = 0; i < size; i++) 
            {
              EEPROM.write(eeAddress, data[i]);
              byte1 = data[i];
              if (byte1 <= 0xF) {
                Serial.print(0);
              }
              Serial.print(byte1, HEX);
              eeAddress += 1;
            }
          } 
        }

      }
      while(1);
    }
  }
}
