#include <SPI.h>
#include <PN532_SPI.h>
#include "PN532.h"
#include <time.h>

PN532_SPI pn532spi(SPI, 10);
PN532 nfc(pn532spi);

//  in SPI mode (jumper 0 - 1) and SCK MISO MOSI SS VCC GND

int t1;
int t2;
byte byte1;

void setup(void) {
  delay(1000);  
  Serial.begin(115200);
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
   nfc.setPassiveActivationRetries(0xFF);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  Serial.println("Waiting for an ISO14443A card");
}

void loop(void) 
{
  boolean success;                          // Flag to check if there was an error with the PN532
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint8_t currentblock;                     // Counter to keep track of which block we're on
  uint8_t currentsector;                    // Counter to keep track of which sector we're on
  bool authenticated = false;               // Flag to indicate if the sector is authenticated
  uint8_t data[16];                         // Array to store block data during reads
  int size = sizeof(data);
  int value;
  uint8_t key0_4[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  uint8_t key5_13[6] = { 0x6A, 0x19, 0x87, 0xC4, 0x0A, 0x21 };
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  
  if (success) {
    Serial.println("Found a card!");
    Serial.print("UID Length: ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i=0; i < uidLength; i++) {
      Serial.print(" 0x");
      Serial.print(uid[i], HEX); 
    }
    Serial.println("");
    
    if (uidLength == 4) {
      Serial.println("Seems to be a Mifare Classic card (4 byte UID)");
      t1 = millis();
      for (currentblock = 0; currentblock < 64; currentblock++) {
        // Check if this is a new block so that we can reauthenticate
        if (nfc.mifareclassic_IsFirstBlock(currentblock)) authenticated = false;
        if (!authenticated) {
          currentsector = (int)currentblock/4;
          Serial.print("------------------------Sector ");
          Serial.print(currentsector, DEC);
          Serial.println("-------------------------");
          if (currentsector <= 4) success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentblock, 0, &key0_4[0]);
          else success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentblock, 0, &key5_13[0]);
          
          if (success) authenticated = true;
          else Serial.println("Authentication error. Unable to authenticate. Block skipped.");
        }

        if (authenticated) {
          success = nfc.mifareclassic_ReadDataBlock(currentblock, data);
          if (success) {
            // Read successful
            // Serial.print("Block ");
            // Serial.print(currentblock%4, DEC);
            // Dump the raw data
            nfc.PrintHexChar(data, 16);
          //  Serial.println();
           // for (int i = 0; i < size; i++) {
            //  byte1 = data[i];
         //     if (byte1 <= 0xF) Serial.print(0);
        //      Serial.print(byte1, HEX);
        //    }
          }
          else Serial.println(" unable to read this block");
        }
        Serial.println();
      }
      t2 = millis();
      Serial.println();
      Serial.println("Time : ");
      Serial.println(t2-t1);

      while(1);
    } 
    else 
    {
      Serial.println("Ooops ... this doesn't seem to be a Mifare Classic card!");
    }
  }
}

/*
[docs]
    def getFirmwareVersion(self):
        """
        uint32_t
        getFirmwareVersion()

        Checks the firmware version of the PN5xx chip.

        Checks the firmware version of the PN5xx chip

        the chip's firmware version and ID

        The chip's firmware version and ID 
        """
        return _pyupm_pn532.PN532_getFirmwareVersion(self)



[docs]
    def sendCommandCheckAck(self, cmd, cmdlen, timeout=1000):
        """
        bool
        sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t
        timeout=1000)

        Sends a command and waits a specified period for the ACK.

        sends a command and waits a specified period for the ACK

        Parameters:
        -----------

        cmd:  Pointer to the command buffer

        cmdlen:  the size of the command in bytes

        timeout:  timeout before giving up (in ms)

        true if everything is OK, false if timeout occurred before an ACK was
        received

        Parameters:
        -----------

        cmd:  Pointer to the command buffer

        cmdlen:  The size of the command in bytes

        timeout:  timeout before giving up

        1 if everything is OK, 0 if timeout occurred before an ACK was
        received 
        """
        return _pyupm_pn532.PN532_sendCommandCheckAck(self, cmd, cmdlen, timeout)



[docs]
    def SAMConfig(self):
        """
        bool SAMConfig(void)

        Configures the SAM (Secure Access Module)

        configures the SAM (Secure Access Module)

        true if successfully configured 
        """
        return _pyupm_pn532.PN532_SAMConfig(self)



[docs]
    def setPassiveActivationRetries(self, maxRetries):
        """
        bool
        setPassiveActivationRetries(uint8_t maxRetries)

        sets the MxRtyPassiveActivation byte of the RFConfiguration register.
        By default the pn532 will retry indefinitely.

        Parameters:
        -----------

        maxRetries:  0xFF to wait forever, 0x00..0xFE to timeout after
        maxRetries. 0x00 means try once, with no retries on failure.

        true if everything executed properly, false for an error  Sets the
        MxRtyPassiveActivation byte of the RFConfiguration register

        Parameters:
        -----------

        maxRetries:  0xFF to wait forever, 0x00..0xFE to timeout after
        mxRetries

        1 if everything executed properly, 0 for an error 
        """
        return _pyupm_pn532.PN532_setPassiveActivationRetries(self, maxRetries)



[docs]
    def readPassiveTargetID(self, cardbaudrate, uid, uidLength, timeout):
        """
        bool
        readPassiveTargetID(BAUD_T cardbaudrate, uint8_t *uid, uint8_t
        *uidLength, uint16_t timeout)

        waits for an ISO14443A target to enter the field

        Parameters:
        -----------

        cardbaudrate:  baud rate of the card, one of the BAUD_T values

        uid:  Pointer to the array that will be populated with the cards UID,
        up to 7 bytes

        uidLength:  Pointer to the variable that will hold the length of the
        card's UID.

        timeout:  the number of milliseconds to wait

        true if everything executed properly, false for an error  Waits for an
        ISO14443A target to enter the field

        Parameters:
        -----------

        cardBaudRate:  Baud rate of the card

        uid:  Pointer to the array that will be populated with the card's UID
        (up to 7 bytes)

        uidLength:  Pointer to the variable that will hold the length of the
        card's UID.

        1 if everything executed properly, 0 for an error 
        """
        return _pyupm_pn532.PN532_readPassiveTargetID(self, cardbaudrate, uid, uidLength, timeout)



[docs]
    def inDataExchange(self, send, sendLength, response, responseLength):
        """
        bool
        inDataExchange(uint8_t *send, uint8_t sendLength, uint8_t *response,
        uint8_t *responseLength)

        Exchanges an APDU with the currently inlisted peer.

        exchanges an APDU (Application Protocol Data Unit) with the currently
        inlisted peer

        Parameters:
        -----------

        send:  Pointer to data to send

        sendLength:  Length of the data to send

        response:  Pointer to response data

        responseLength:  Pointer to the response data length

        true if everything executed properly, false for an error

        Parameters:
        -----------

        send:  Pointer to data to send

        sendLength:  Length of the data to send

        response:  Pointer to response data

        responseLength:  Pointer to the response data length 
        """
        return _pyupm_pn532.PN532_inDataExchange(self, send, sendLength, response, responseLength)



[docs]
    def inListPassiveTarget(self):
        """
        bool
        inListPassiveTarget()

        'InLists' a passive target. PN532 acting as reader/initiator, peer
        acting as card/responder.

        'InLists' a passive target. PN532 acting as reader/initiator, peer
        acting as card/responder.

        true if everything executed properly, false for an error 
        """
        return _pyupm_pn532.PN532_inListPassiveTarget(self)



[docs]
    def mifareclassic_IsFirstBlock(self, uiBlock):
        """
        bool
        mifareclassic_IsFirstBlock(uint32_t uiBlock)

        Indicates whether the specified block number is the first block in the
        sector (block 0 relative to the current sector)

        true if it's the first block, false otherwise  Indicates whether the
        specified block number is the first block in the sector (block 0
        relative to the current sector) 
        """
        return _pyupm_pn532.PN532_mifareclassic_IsFirstBlock(self, uiBlock)



[docs]
    def mifareclassic_IsTrailerBlock(self, uiBlock):
        """
        bool
        mifareclassic_IsTrailerBlock(uint32_t uiBlock)

        indicates whether the specified block number is the sector trailer

        true if it's the trailer block, false otherwise  Indicates whether the
        specified block number is the sector trailer 
        """
        return _pyupm_pn532.PN532_mifareclassic_IsTrailerBlock(self, uiBlock)



[docs]
    def mifareclassic_AuthenticateBlock(self, uid, uidLen, blockNumber, keyNumber, keyData):
        """
        bool mifareclassic_AuthenticateBlock(uint8_t *uid, uint8_t uidLen,
        uint32_t blockNumber, uint8_t keyNumber, uint8_t *keyData)

        tries to authenticate a block of memory on a MIFARE card using the
        INDATAEXCHANGE command. See section 7.3.8 of the PN532 User Manual for
        more information on sending MIFARE and other commands.

        Parameters:
        -----------

        uid:  Pointer to a byte array containing the card UID

        uidLen:  The length (in bytes) of the card's UID (Should be 4 for
        MIFARE Classic)

        blockNumber:  The block number to authenticate. (0..63 for 1KB cards,
        and 0..255 for 4KB cards).

        keyNumber:  Which key type to use during authentication (0 =
        MIFARE_CMD_AUTH_A, 1 = MIFARE_CMD_AUTH_B)

        keyData:  Pointer to a byte array containing the 6 byte key value

        true if everything executed properly, false for an error  Tries to
        authenticate a block of memory on a MIFARE card using the
        INDATAEXCHANGE command. See section 7.3.8 of the PN532 User Manual for
        more information on sending MIFARE and other commands.

        Parameters:
        -----------

        uid:  Pointer to a byte array containing the card UID

        uidLen:  The length (in bytes) of the card's UID (Should be 4 for
        MIFARE Classic)

        blockNumber:  The block number to authenticate. (0..63 for 1KB cards,
        and 0..255 for 4KB cards).

        keyNumber:  Which key type to use during authentication (0 =
        MIFARE_CMD_AUTH_A, 1 = MIFARE_CMD_AUTH_B)

        keyData:  Pointer to a byte array containing the 6 byte key value

        1 if everything executed properly, 0 for an error 
        """
        return _pyupm_pn532.PN532_mifareclassic_AuthenticateBlock(self, uid, uidLen, blockNumber, keyNumber, keyData)



[docs]
    def mifareclassic_ReadDataBlock(self, blockNumber, data):
        """
        bool
        mifareclassic_ReadDataBlock(uint8_t blockNumber, uint8_t *data)

        tries to read an entire 16-byte data block at the specified block
        address.

        Parameters:
        -----------

        blockNumber:  The block number to read (0..63 for 1KB cards, and
        0..255 for 4KB cards).

        data:  Pointer to the byte array that will hold the retrieved data (if
        any)

        true if everything executed properly, false for an error  Tries to
        read an entire 16-byte data block at the specified block address.

        Parameters:
        -----------

        blockNumber:  The block number to authenticate. (0..63 for 1KB cards,
        and 0..255 for 4KB cards).

        data:  Pointer to the byte array that will hold the retrieved data (if
        any)

        1 if everything executed properly, 0 for an error 
        """
        return _pyupm_pn532.PN532_mifareclassic_ReadDataBlock(self, blockNumber, data)



[docs]
    def mifareclassic_WriteDataBlock(self, blockNumber, data):
        """
        bool
        mifareclassic_WriteDataBlock(uint8_t blockNumber, uint8_t *data)

        tries to write an entire 16-byte data block at the specified block
        address.

        Parameters:
        -----------

        blockNumber:  The block number to write. (0..63 for 1KB cards, and
        0..255 for 4KB cards).

        data:  The byte array that contains the data to write.

        true if everything executed properly, false for an error  Tries to
        write an entire 16-byte data block at the specified block address.

        Parameters:
        -----------

        blockNumber:  The block number to authenticate. (0..63 for 1KB cards,
        and 0..255 for 4KB cards).

        data:  The byte array that contains the data to write.

        1 if everything executed properly, 0 for an error 
        """
        return _pyupm_pn532.PN532_mifareclassic_WriteDataBlock(self, blockNumber, data)



[docs]
    def mifareclassic_FormatNDEF(self):
        """
        bool
        mifareclassic_FormatNDEF(void)

        formats a Mifare Classic card to store NDEF Records

        true if everything executed properly, false for an error  Formats a
        Mifare Classic card to store NDEF Records

        1 if everything executed properly, 0 for an error 
        """
        return _pyupm_pn532.PN532_mifareclassic_FormatNDEF(self)



[docs]
    def mifareclassic_WriteNDEFURI(self, sectorNumber, uriIdentifier, url):
        """
        bool
        mifareclassic_WriteNDEFURI(uint8_t sectorNumber, NDEF_URI_T
        uriIdentifier, const char *url)

        writes an NDEF URI Record to the specified sector (1..15)

        Note that this function assumes that the Mifare Classic card is
        already formatted to work as an "NFC Forum Tag" and uses a MAD1 file
        system. You can use the NXP TagWriter app on Android to properly
        format cards for this.

        Parameters:
        -----------

        sectorNumber:  The sector that the URI record should be written to
        (can be 1..15 for a 1K card)

        uriIdentifier:  The uri identifier code (one of the NDEF_URI_T values

        url:  the uri text to write (max 38 characters).

        true if everything executed properly, false for an error  Writes an
        NDEF URI Record to the specified sector (1..15)

        Note that this function assumes that the Mifare Classic card is
        already formatted to work as an "NFC Forum Tag" and uses a MAD1 file
        system. You can use the NXP TagWriter app on Android to properly
        format cards for this.

        Parameters:
        -----------

        sectorNumber:  The sector that the URI record should be written to
        (can be 1..15 for a 1K card)

        uriIdentifier:  The uri identifier code (0 = none, 0x01 =
        "http://www.", etc.)

        url:  The uri text to write (max 38 characters).

        1 if everything executed properly, 0 for an error 
        """
        return _pyupm_pn532.PN532_mifareclassic_WriteNDEFURI(self, sectorNumber, uriIdentifier, url)



[docs]
    def ntag2xx_ReadPage(self, page, buffer):
        """
        bool
        ntag2xx_ReadPage(uint8_t page, uint8_t *buffer)

        read an entire 4-byte page at the specified address

        Parameters:
        -----------

        page:  The page number (0..63 in most cases)

        buffer:  Pointer to the byte array that will hold the retrieved data
        (if any)

        true if everything executed properly, false for an error  Tries to
        read an entire 4-byte page at the specified address.

        Parameters:
        -----------

        page:  The page number (0..63 in most cases)

        buffer:  Pointer to the byte array that will hold the retrieved data
        (if any) 
        """
        return _pyupm_pn532.PN532_ntag2xx_ReadPage(self, page, buffer)



[docs]
    def ntag2xx_WritePage(self, page, data):
        """
        bool
        ntag2xx_WritePage(uint8_t page, uint8_t *data)

        write an entire 4-byte page at the specified block address

        Parameters:
        -----------

        page:  The page number to write. (0..63 for most cases)

        data:  The byte array that contains the data to write. Should be
        exactly 4 bytes long.

        true if everything executed properly, false for an error  Tries to
        write an entire 4-byte page at the specified block address.

        Parameters:
        -----------

        page:  The page number to write. (0..63 for most cases)

        data:  The byte array that contains the data to write. Should be
        exactly 4 bytes long.

        1 if everything executed properly, 0 for an error 
        """
        return _pyupm_pn532.PN532_ntag2xx_WritePage(self, page, data)



[docs]
    def ntag2xx_WriteNDEFURI(self, uriIdentifier, url, dataLen):
        """
        bool
        ntag2xx_WriteNDEFURI(NDEF_URI_T uriIdentifier, char *url, uint8_t
        dataLen)

        writes an NDEF URI Record starting at the specified page (4..nn)

        Note that this function assumes that the NTAG2xx card is already
        formatted to work as an "NFC Forum Tag".

        Parameters:
        -----------

        uriIdentifier:  The uri identifier code (one of the NDEF_URI_T values

        url:  The uri text to write (null-terminated string).

        dataLen:  The size of the data area for overflow checks.

        true if everything executed properly, false for an error  Writes an
        NDEF URI Record starting at the specified page (4..nn)

        Note that this function assumes that the NTAG2xx card is already
        formatted to work as an "NFC Forum Tag".

        Parameters:
        -----------

        uriIdentifier:  The uri identifier code (0 = none, 0x01 =
        "http://www.", etc.)

        url:  The uri text to write (null-terminated string).

        dataLen:  The size of the data area for overflow checks.

        1 if everything executed properly, 0 for an error 
        """
        return _pyupm_pn532.PN532_ntag2xx_WriteNDEFURI(self, uriIdentifier, url, dataLen)



[docs]
    def getATQA(self):
        """
        uint16_t getATQA()

        return the ATQA (Answer to Request Acknowledge) value. This value is
        only valid after a successful call to readPassiveTargetID()

        ATQA value 
        """
        return _pyupm_pn532.PN532_getATQA(self)



[docs]
    def getSAK(self):
        """
        uint8_t getSAK()

        return the SAK (Select Acknowledge) value. This value is only valid
        after a successful call to readPassiveTargetID()

        SAK value 
        """
        return _pyupm_pn532.PN532_getSAK(self)



[docs]
    def i2cContext(self):
        """
        mraa::I2c&
        i2cContext()

        provide public access to the class's MRAA i2C context for direct user
        access

        a reference to the class i2c context 
        """
        return _pyupm_pn532.PN532_i2cContext(self)



[docs]
    def pn532Debug(self, enable):
        """
        void pn532Debug(bool
        enable)

        enable or disable debugging output for pn532 related operations

        Parameters:
        -----------

        enable:  true to enabloe debug output, false to disable 
        """
        return _pyupm_pn532.PN532_pn532Debug(self, enable)



[docs]
    def mifareDebug(self, enable):
        """
        void mifareDebug(bool
        enable)

        enable or disable debugging output for mifare related operations

        Parameters:
        -----------

        enable:  true to enabloe debug output, false to disable 
        """
        return _pyupm_pn532.PN532_mifareDebug(self, enable)



[docs]
    def tagType(self):
        """
        PN532::TAG_TYPE_T
        tagType()

        try to determine the tag type

        one of the TAG_TYPE_T values 
        """
        return _pyupm_pn532.PN532_tagType(self)

*/
