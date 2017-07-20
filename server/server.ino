#include <ESP8266WiFi.h>

#define isConnected() ((WiFi.status() != WL_CONNECTED)? false : true)


/*
Include tag data here.
Because the RFID code usually spans 5 bytes,
I'm assuming the incoming RFID data to be a 10 byte string.
(1 byte will be stored in 2 characters)
Since we're receiving strings from the port,
direct comparision is easy.
*/
typedef struct tag
{
  string rfidcode, instruction;
}tag0,tag1,tag2;

tag0.rfidcode = "10bytedata";
tag0.instruction = "3";
tag1.rfidcode = "bytedata10";
tag1.instruction = "2";
tag2.rfidcode = "databyte10";
tag2.instruction = "1";
/*
WiFi Configuration:
Includes storing router ssid,password and port to listen to incoming data
*/
const char* ssid = "ssid";
const char* password = "password";
unsigned int port = 1234;
uint8_t mac[6]; //stores mac address of device
uint8_t *bssid; //broadcasted SSID
int8_t rssi;    //RSSI, also sent on request from client

// specify the port to listen
WiFiServer server(port);

void connectToRouter()
{
  // Connect to WiFi network
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void startServer()
{
  server.begin();
  Serial.println("Server started");
}

void printDetails()
{
  bool isCon = false;
  Serial.println(isConnected() ? "\nConnected" : "\nWaiting to Connect");
  if(isConnected())
  {
      Serial.print("SSID: ");
      Serial.println(WiFi.SSID());
      bssid = WiFi.BSSID();
      Serial.printf("bssid: %02x:%02x:%02x:%02x:%02x:%02x\n",bssid[0],bssid[1],bssid[2],bssid[3],bssid[4],bssid[5]);
      rssi = WiFi.RSSI();
      Serial.printf("RSSID: %d dBm\n", rssi);
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
      Serial.print("Subnet Mask: ");
      Serial.println(WiFi.subnetMask());
      Serial.printf("Gateway: ");
      Serial.println(WiFi.gatewayIP());
      Serial.print("DNS\t#1: ");
      Serial.print(WiFi.dnsIP());
      Serial.print("\n\t#2: ");
      Serial.println(WiFi.dnsIP(1));
  }
  Serial.printf("DHCP Hostname: %s\n",WiFi.hostname().c_str());
  WiFi.macAddress(mac);
  Serial.printf("macAddress: %02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
}

void setup()
{
  Serial.begin(115200);
  delay(10);

  pinMode(0, OUTPUT);
  digitalWrite(0, 0); //active Low GPIOs on ESP-01
    
  connectToRouter();
  startServer();
  printDetails();

}

void loop()
{
  // Check if a client has connected
  WiFiClient client = server.available();
  if(!client) return;
  
  // Wait until the client sends some data
  Serial.println("Waiting for new request...");
  while(!client.available())
  {
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.print("Received Scan-Code: ");
  Serial.println(req);
  client.flush();

  //variable for sending instruction
  string instruct = "";

  //match request
  if (req.indexOf("test") != -1)  //a ping test that return connection strength
    instruct = str(WiFi.RSSI());
  else if (req.indexOf(tag0.rfidcode) != -1)
    instruct = tag0.instruction;
  else if (req.indexOf(tag1.rfidcode) != -1)
    instruct = tag1.instruction;
  else if (req.indexOf(tag2.rfidcode) != -1)
    instruct = tag2.instruction;
  else
  {
    Serial.println("Invalid Request");
    client.stop();
    return;
  }
  
  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
  s += instruct;
  s += "</html>\n";

  // Send the response to the client
  Serial.print("Sending Instruction: ");
  Serial.println(instruct);
  delay(1);
  client.print(s);
  delay(10);
  Serial.println("Instruction Sent!");
}