#include "WiFiClientSecure.h"
char ssid[] = "INTIWLTRAINING";
char pass[] = "1234554321";  
#define SERVER     "monitordetakjantung.000webhostapp.com"
#define PORT     443
#define BOUNDARY     "--------------------------133747188241686651551404"  
#define TIMEOUT      20000

void setup() 
{

  Serial.begin(115200);
  
  WiFi.begin(ssid, pass);
  unsigned char led_cnt=0;
  while (WiFi.status() != WL_CONNECTED) 
  {
     Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(5000);
  uint8_t dataku[]={'b','a','t','a','p','i','c'};
   String res = sendImage("asdw","A54S89EF5",dataku,7);
        Serial.println(res);
}

void loop() 
{
}

//////

String sendImage(String token,String message, uint8_t *data_pic,size_t size_pic)
{
  String bodyTxt =  body("message",message);
  String bodyPic =  body("imageFile",message);
  String bodyEnd =  String("--")+BOUNDARY+String("--\r\n");
  size_t allLen = bodyTxt.length()+bodyPic.length()+size_pic+bodyEnd.length();
  String headerTxt =  header(token,allLen);
  WiFiClientSecure client;
   if (!client.connect(SERVER,PORT)) 
   {
    return("connection failed");   
   }

   client.print(headerTxt+bodyTxt+bodyPic);
   client.write(data_pic,size_pic);
   client.print("\r\n"+bodyEnd);

   delay(20);
   long tOut = millis() + TIMEOUT;
   while(client.connected() && tOut > millis()) 
   {
    if (client.available()) 
    {
      String serverRes = client.readStringUntil('\r');
        return(serverRes);
    }
   }
}
String header(String token,size_t length)
{
  String  data;
      data =  F("POST /uploaddata.php HTTP/1.1\r\n");
      data += F("cache-control: no-cache\r\n");
      data += F("Content-Type: multipart/form-data; boundary=");
      data += BOUNDARY;
      data += "\r\n";
      data += F("User-Agent: PostmanRuntime/6.4.1\r\n");
      data += F("Accept: */*\r\n");
      data += F("Host: ");
      data += SERVER;
      data += F("\r\n");
      data += F("accept-encoding: gzip, deflate\r\n");
      data += F("Connection: keep-alive\r\n");
      data += F("content-length: ");
      data += String(length);
      data += "\r\n";
      data += "\r\n";
    return(data);
}
String body(String content , String message)
{
  String data;
  data = "--";
  data += BOUNDARY;
  data += F("\r\n");
  if(content=="imageFile")
  {
    data += F("Content-Disposition: form-data; name=\"imageFile\"; filename=\"test.txt\"\r\n");
    data += F("Content-Type: image/jpeg\r\n");
    data += F("\r\n");
  }
  else
  {
    data += "Content-Disposition: form-data; name=\"" + content +"\"\r\n";
    data += "\r\n";
    data += message;
    data += "\r\n";
  }
   return(data);
}
