#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#define SS_PIN D8
#define RST_PIN D3
//the port on which data has to be dumped
#define PORT "5000"
//whether to store key after scan or clear it
#define STORE_KEY_AFTER_SCAN true

//MFRC Class Instance
MFRC522 rfid(SS_PIN, RST_PIN);

//create a variable for key
MFRC522::MIFARE_Key key; 

//4 bytes to store new NUID 
byte nuidPICC[4];
//remains Global, updates on each successfull call of scan()
String hexKey = "";
String teamID = "TEAM";

void scan()
{
  //clear variable on every call of scan
  if(!STORE_KEY_AFTER_SCAN)
    hexKey = "";
  // Look for new cards and verify that the card has been read already
  if ( ! rfid.PICC_IsNewCardPresent())
    return;
  if ( ! rfid.PICC_ReadCardSerial())
    return;

 MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  //Debugging - Type of Card
  /*
  Serial.print(F("PICC type: "));
  Serial.println(rfid.PICC_GetTypeName(piccType));
  */ 
  // Check type of PICC
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K)
  {
    hexKey = "FF FF FF FF";
    /*
    Choose whether key value will be checked or String above
    for(byte i = 0; i < 6; i++)
      key.keyByte[i] = 0xFF;
    Serial.println(F("Not a MIFARE Classic type tag"));
    */
    return;
  }

  //update variable only on new scan
  if(STORE_KEY_AFTER_SCAN)
    hexKey = "";

  //store NUID
  for (byte i = 0; i < 4; i++)
    nuidPICC[i] = rfid.uid.uidByte[i];

  createHex(nuidPICC, sizeof(nuidPICC));
  
  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

//Generate Hex key from Binary dump
void createHex(byte *buffer, byte bufferSize)
{
  //An eleven character string is stored in hexKey
  for (byte i = 0; i < bufferSize; i++)
  {
    if(i)
      hexKey += (buffer[i] < 0x10 ? " 0" : " ");
    else
      hexKey += (buffer[i] < 0x10 ? "0" : "");
    hexKey += String(buffer[i], HEX);
  }
}

void sendPOST()
{
  //here I'm assuming WiFi is already connected
  HTTPClient http;
  String message = hexKey;
  String POSTaddress = "http://" + String(WiFi.localIP()) + ":" + PORT + "/" + teamID + "/";
  http.begin(POSTaddress);
  http.addHeader("Content-Type", "text/plain");
  int httpCode = http.POST(message);
/*
  Our expected data from the GET request can be fetched here if you want
  if(httpCode > 0)
  {
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    if(httpCode == HTTP_CODE_OK)
    {
      String payload = http.getString();
      Serial.println(payload);
    }
  }
  else
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
*/
  http.end();
}

void setup()
{ 
  Serial.begin(115200);

  //initialisations
  SPI.begin();
  rfid.PCD_Init();
}

void loop()
{
  scan();
  sendPOST();
  //Serial.println(hexKey);
  delay(500);
}

/*
> I've given separate functions for scanning and posting, combine them in the scan function if you want.
> You might want to check, or make changes to line 91, where POSTaddress is being created. Check httpCode
  value to determine whether post was successful.
*/