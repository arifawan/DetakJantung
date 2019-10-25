#include <WiFi.h>              
#include <ESP32WebServer.h>    
#include <ESPmDNS.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define ServerVersion "1.0"
String webpage = "";
bool   SD_present = false;
#define   servername "fileserver" 
ESP32WebServer server(80);

const char* ssid = "asusx"; //SSID Wifi
const char* password = "1234awan"; //Password Wifi

int sensor;
String dataSensor; 

//===================================================================
//                      Fungsi untuk Sd Card
//===================================================================
void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}
void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);
    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}
void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

//===================================================================
//              Fungsi untuk Download+Delete File (Webserver)
//===================================================================
void SendHTML_Header(){
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); 
  server.sendHeader("Pragma", "no-cache"); 
  server.sendHeader("Expires", "-1"); 
  server.setContentLength(CONTENT_LENGTH_UNKNOWN); 
  server.send(200, "text/html", ""); // Empty content 

  webpage  = F("<!DOCTYPE html><html>");
  webpage += F("<head>");
  webpage += F("<title>Download File Sensor</title>"); // NOTE: 1em = 16p
  webpage += F("</head><body><h1>File Download Server Local"); webpage += String(ServerVersion) + "</h1>";
  
  server.sendContent(webpage);
  webpage = "";
}

void HomePage(){
  SendHTML_Header();
  webpage += F("<a href='/download'><button>Download</button></a>");
  webpage += F("<a href='/delete'><button>Delete</button></a>");
  webpage += F("<a href='/dir'><button>Directory</button></a>");
  webpage += F("<a href = 'https://monitordetakjantung.000webhostapp.com/'><b><button>Link Website</button></b></a>"); //link 000webhost
  
  SendHTML_Content();
  SendHTML_Stop(); 
}
void SendHTML_Content(){
  server.sendContent(webpage);
  webpage = "";
}

void SendHTML_Stop(){
  server.sendContent("");
  server.client().stop(); // Stop is needed because no content length was sent
}


void SelectInput1(String heading1, String heading2, String command, String arg_calling_name){
  SendHTML_Header();
  webpage += F("<h3 class='rcorners_m'>");webpage += heading1+"</h3><br>";
  webpage += F("<h3>"); webpage += heading2 + "</h3>"; 
  webpage += F("<FORM action='/"); webpage += command + "' method='post'>"; 
  webpage += F("<input type='text' name='"); webpage += arg_calling_name; webpage += F("' value=''><br>");
  webpage += F("<type='submit' name='"); webpage += arg_calling_name; webpage += F("' value=''><br><br>");
  SendHTML_Content();
  SendHTML_Stop();
}

void SelectInput2(String heading1, String heading2, String command, String arg_calling_name){
  SendHTML_Header();
  webpage += F("<h3 class='rcorners_m'>");webpage += heading1+"</h3><br>";
  webpage += F("<h3>"); webpage += heading2 + "</h3>"; 
  webpage += F("<FORM action='/"); webpage += command + "' method='post'>"; 
  webpage += F("<input type='text' name='"); webpage += arg_calling_name; webpage += F("' value=''><br>");
  webpage += F("<type='submit' name='"); webpage += arg_calling_name; webpage += F("' value=''><br><br>");
  SendHTML_Content();
  SendHTML_Stop();
}


void File_Download(){ 
  if (server.args() > 0 ) { // Arguments were received
    if (server.hasArg("download")) SD_file_download(server.arg(0));
  }
  else SelectInput1("Download File","Enter filename to download(data.txt)","download","download");
}

void SD_file_download(String filename){
  if (SD_present) { 
    File download = SD.open("/"+filename);
    if (download) {
      server.sendHeader("Content-Type", "text/text");
      server.sendHeader("Content-Disposition", "attachment; filename="+filename);
      server.sendHeader("Connection", "close");
      server.streamFile(download, "application/octet-stream");
      download.close();
    }      
  }
}

void File_Delete(){
  if (server.args() > 0 ) { // Arguments were received
    if (server.hasArg("delete")) SD_file_delete(server.arg(0));
  }
  else SelectInput2("Delete File","Select a File to Delete","delete","delete");
}

void SD_file_delete(String filename) { // Delete the file 
  if (SD_present) { 
    SendHTML_Header();
    File dataFile = SD.open("/"+filename, FILE_READ); // Now read data from SD Card 
    Serial.print("Deleting file: "); Serial.println(filename);
    if (dataFile)
    {
      if (SD.remove("/"+filename)) {
        Serial.println(F("File deleted successfully"));
        webpage += "<h3>File '"+filename+"' has been erased</h3>"; 
        webpage += F("<a href='/delete'>[Back]</a><br><br>");
      }
      else
      { 
        webpage += F("<h3>File was not deleted - error</h3>");
        webpage += F("<a href='delete'>[Back]</a><br><br>");
      }
    SendHTML_Content();
    SendHTML_Stop();
}
  }
}

void printDirectory(const char * dirname, uint8_t levels){
  File root = SD.open(dirname);
  #ifdef ESP8266
  root.rewindDirectory(); //Only needed for ESP8266
  #endif
  if(!root){
    return;
  }
  if(!root.isDirectory()){
    return;
  }
  File file = root.openNextFile();
  while(file){
    if (webpage.length() > 1000) {
      SendHTML_Content();
    }
    if(file.isDirectory()){
      webpage += "<tr><td>"+String(file.isDirectory()?"Dir":"File")+"</td><td>"+String(file.name())+"</td><td></td></tr>";
      printDirectory(file.name(), levels-1);
    }
    else
    {
      webpage += "<tr><td>"+String(file.name())+"</td>";
      webpage += "<td>"+String(file.isDirectory()?"Dir":"File")+"</td>";
      int bytes = file.size();
      String fsize = "";
      if (bytes < 1024)                     fsize = String(bytes)+" B";
      else if(bytes < (1024 * 1024))        fsize = String(bytes/1024.0,3)+" KB";
      else if(bytes < (1024 * 1024 * 1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
      else                                  fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
      webpage += "<td>"+fsize+"</td></tr>";
    }
    file = root.openNextFile();
  }
  file.close();
}

void SD_dir(){
  if (SD_present) { 
    File root = SD.open("/");
    if (root) {
      root.rewindDirectory();
      SendHTML_Header();
      webpage += F("<h3 class='rcorners_m'>SD Card Contents</h3><br>");
      webpage += F("<table align='center'>");
      webpage += F("<tr><th>Name/Type</th><th style='width:20%'>Type File/Dir</th><th>File Size</th></tr>");
      printDirectory("/",0);
      webpage += F("</table>");
      SendHTML_Content();
      root.close();
    }
    else 
    {
      SendHTML_Header();
      webpage += F("<h3>No Files Found</h3>");
    }
    SendHTML_Content();
    SendHTML_Stop(); 
}
}


void setup() {
  Serial.begin(115200);
  
  //pin AD8232
  pinMode(16, INPUT); // LO +
  pinMode(17, INPUT); // LO -

  //connect wifi
  WiFi.begin(ssid, password);
  Serial.print("Cennecting wifi");
  while (WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected Wifi"); 
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

// Set your preferred server name
   if (!MDNS.begin(servername)) {          
    Serial.println(F("Error setting up MDNS responder!")); 
    ESP.restart(); 
  } 

  //SD CARD
   if(!SD.begin()){
        Serial.println("Card Mount Failed");
        SD_present = false; 
    } else {
      Serial.println(F("Card initialised... file access enabled..."));
      SD_present = true; 
    }
    uint8_t cardType = SD.cardType();
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    delay(1000);

    //command server
    server.on("/",         HomePage);
    server.on("/download", File_Download);
    server.on("/delete",   File_Delete);
    server.on("/dir",      SD_dir);
    
    server.begin();
    Serial.println("HTTP server started");

    writeFile(SD, "/data.txt", "datanya: \n"); //membuat sebuah file
    delay(10000);
}


void loop(void){
  //Pembacaan nilai sensor AD8232
   if((digitalRead(16) == 1)||(digitalRead(17) == 1)){
   Serial.println('!');
      } else{
        Serial.println(analogRead(34)); // mengirim nilai dari Analog pin
        sensor = analogRead(34);
      }
      delay(1); //pembacaan data per 1ms  
    
    //Menyimpan nilai sensor ke sdcard
    dataSensor = String(sensor) + "\r\n";
    appendFile(SD, "/data.txt", dataSensor.c_str()); //agar bisa menambah karena harus cons char
    delay(1); 

     server.handleClient(); // Listen for client connections
}


