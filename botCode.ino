#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <Servo.h>

#define SS_PIN D4
#define RST_PIN D3

#define BSSID ARC
#define PASSWORD password
// whether to store key after scan or clear it
#define STORE_KEY_AFTER_SCAN true

//MFRC Class Instance
MFRC522 rfid(SS_PIN, RST_PIN);

//create a variable for key
MFRC522::MIFARE_Key key; 

//4 bytes to store new NUID 
byte nuidPICC[4];
//remains Global, updates on each successfull call of scan()
String hexKey = "";

byte commands[4]={0,0,0,0};

int lmotor1 = D1;
int lmotor2 = D2;
int rmotor1 = D8;
int rmotor2 = D6;

Servo gripper;

int gripper_pos=0;  //180

int NEUTRAL = 1000;
int LEFT = 999;
int RIGHT = 1001;

int MOV_NEUTRAL = 10;
int FORWARD = 11;
int BACKWARD = 9;

int corr_factor = 2;
void setup()
{
  //initialisations
  pinMode(lmotor1, OUTPUT);
  pinMode(lmotor2, OUTPUT);
  pinMode(rmotor1, OUTPUT);
  pinMode(rmotor2, OUTPUT);
  analogWrite(lmotor1, 0);
  analogWrite(lmotor2, 0);
  analogWrite(rmotor1, 0);
  analogWrite(rmotor2, 0); 
  
  SPI.begin();
  rfid.PCD_Init();
  Serial.begin(9600);
  delay(1000);
  Serial.println("Serial Initialized");

  // connect to WiFi
  WiFi.begin(BSSID, PASSWORD);
  Serial.println("Contacting Host");
  while(Wifi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected to " + str(BSSID));
  
  // positon gripper
  pinMode(D0, OUTPUT);
  gripper.attach(D0);
  gripper.write(100);
}

void loop(){
  // put your main code here, to run repeatedly:
  if(Serial.available()>4)
  {
     // if the first byte read is not the first byte sent from the bluetooth skip it and read the next byte
    while(1){
    commands[0] = Serial.read();  //Direction
    // check if the first byte value corresponds to those sent by the app over bluetooth
    if(commands[0]==241 || commands[0]==242 || commands[0]==243)
      break;
    }
  // read the next 3 bytes as well
    commands[1] = Serial.read();  //Speed
    commands[2] = Serial.read();  //Angle
    commands[3] = Serial.read();
//  Serial.print("0 ");
//  Serial.print(commands[0]);
//  Serial.print(" 1 ");
//  Serial.print(commands[1]);
//  Serial.print(" 2 ");
//  Serial.print(commands[2]);
//  Serial.print(" 3 ");
//  Serial.println(commands[3]);

    if(commands[3] == 4)
    {
      scan();
      sendGET();
      Serial.print(F("\n-------\n" + str(hexKey) + "\n-------\n"));
      delay(500);
    }

    //Serial.print("comm3 \t");
    //Serial.print(commands[3]+"\t");
    if(commands[3]==16)
    {
     gripper.write(gripper_pos);
     gripper_pos++;
    }
    else if(commands[3]==8)
    {
      gripper.write(gripper_pos);
      gripper_pos--;
    }

    if(commands[3]==0){gripper.write(gripper_pos);}


    int movDir = MOV_NEUTRAL, turnDir = NEUTRAL, scaledTurnSpeed = 0, scaledSpeed = 0;
    int recSpeed = commands[1];
    scaledSpeed = map(recSpeed, 0, 255, 0, 1023);
    int recTurnSpeed = commands[2];
    //Serial.print("recTurnSpeed\t");
    //Serial.println(recTurnSpeed);
    if(recTurnSpeed > 88){
      scaledTurnSpeed = map(recTurnSpeed - 89, 0, 21, 0, 1010);
      turnDir = RIGHT;
    }
    else if(recTurnSpeed < 88){
      scaledTurnSpeed = map(87 - recTurnSpeed, 0, 22, 0, 1010);
      turnDir = LEFT;
    }
    else{turnDir = NEUTRAL;}
    
    if(commands[0] == 241){
      movDir = FORWARD;
    }
    else if(commands[0] == 242){
      movDir = BACKWARD;
    }
    else if(commands[0] == 243){
      movDir = MOV_NEUTRAL;
    }
    /*
    Serial.print(movDir);
    Serial.print("\t");
    Serial.print(scaledSpeed);
    Serial.print("\t");
    Serial.print(turnDir);
    Serial.print("\t");
    Serial.println(scaledTurnSpeed);
    */
    leftWheel(movDir, scaledSpeed, turnDir, scaledTurnSpeed);
    rightWheel(movDir, scaledSpeed, turnDir, scaledTurnSpeed);
  }
}

void leftWheel(int mov_dir, int scaledSpeed, int turn_dir, int scaledTurn){
  if(turn_dir == NEUTRAL){
    if(mov_dir == FORWARD){
      analogWrite(lmotor1, scaledSpeed);
      analogWrite(lmotor2, 0);
    }
    else if(mov_dir == BACKWARD){
      analogWrite(lmotor2, scaledSpeed);
      analogWrite(lmotor1, 0);
    }
    else
    {
      analogWrite(lmotor2, 0);
      analogWrite(lmotor1, 0);
    }
  }
  else if(turn_dir == RIGHT){
    if(mov_dir == FORWARD){
      analogWrite(lmotor1, scaledTurn);
      analogWrite(lmotor2, 0);
    }
    else
    if(mov_dir == BACKWARD){
      analogWrite(lmotor2, scaledTurn);
      analogWrite(lmotor1, 0);
    }
  }
  else if(turn_dir == LEFT){
    if(mov_dir == FORWARD){
      analogWrite(lmotor1, scaledTurn/corr_factor);
      analogWrite(lmotor2, 0);
    }
    else
  if(mov_dir == BACKWARD){
      analogWrite(lmotor2, scaledTurn/corr_factor);
      analogWrite(lmotor1, 0);
    }
  }
}

void rightWheel(int mov_dir, int scaledSpeed, int turn_dir, int scaledTurn){
  if(turn_dir == NEUTRAL){
    if(mov_dir == FORWARD){
      analogWrite(rmotor1, scaledSpeed);
      analogWrite(rmotor2, 0); 
    }
    else if(mov_dir == BACKWARD){
      analogWrite(rmotor2, scaledSpeed);
      analogWrite(rmotor1, 0);
    }
    else
    {
      analogWrite(rmotor2, 0);
      analogWrite(rmotor1, 0);
    }
  }
  else if(turn_dir == LEFT){
    if(mov_dir == FORWARD){
      analogWrite(rmotor1, scaledTurn);
      analogWrite(rmotor2, 0);
    }
    else if(mov_dir == BACKWARD){
      analogWrite(rmotor2, scaledTurn);
      analogWrite(rmotor1, 0);
    }
    else
    {
      analogWrite(rmotor2, 0);
      analogWrite(rmotor1, 0);
    }
  }
  else if(turn_dir == RIGHT){
    if(mov_dir == FORWARD){
      analogWrite(rmotor1, scaledTurn/corr_factor);
      analogWrite(rmotor2, 0);
    }
    else if(mov_dir == BACKWARD){
      analogWrite(rmotor2, scaledTurn/corr_factor);
      analogWrite(rmotor1, 0);
    }
    else
    {
      analogWrite(rmotor2, 0);
      analogWrite(rmotor1, 0);
    }
  }
}

/*
--------------------------------------
MFRC SCANNING AND GET REQUEST
--------------------------------------
*/
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
    piccType != MFRC522::PICC_TYPE_MIFARE_4K){
      hexKey = "FFFFFFFF";
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
  //An eight character string is stored in hexKey
  for (byte i = 0; i < bufferSize; i++)
  { 
    hexKey += (buffer[i] < 0x10 ? "0" : "");
    hexKey += String(buffer[i], HEX);
  }
}

void sendGET()
{
  //here I'm assuming WiFi is already connected
  HTTPClient http;
  String message = "";
  String GETaddress = "http://"+ str(WiFi.gatewayIP()) +":3000/a/" + hexKey + "/";
  http.begin(GETaddress);
  int httpCode = http.GET(message);
/*
  Expected data from the GET request can be fetched here if you want. We are not providing it though
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          USE_SERIAL.println(payload);
      }
  } else {
      USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
*/
  http.end();
}
/*
--------------------------------------
MFRC SCANNING AND GET REQUEST
--------------------------------------
*/

/*
> I've given separate functions for scanning and posting, combine them in the scan function if you want.
> You might want to check, or make changes to line 91, where POSTaddress is being created. Check httpCode
  value to determine whether post was successful.
*/
