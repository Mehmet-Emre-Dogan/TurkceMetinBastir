// Linklerdeki örneklerden esinlenerek yapılmıştır. Originated from these links:
// https://github.com/MajicDesigns/MD_Parola/blob/main/examples/Parola_UFT-8_Display/Parola_UFT-8_Display.ino
// https://mytectutor.com/esp8266-nodemcu-relay-control-from-web-page-over-wifi/

// Gerekli Kütüphaneler ve Define'lar (Necessary Libraries and Defines)

// Matris sürücü (matrix driver)
#include <MD_Parola.h>       
#include <MD_MAX72xx.h>

// Webserver
#include <ESP8266WiFi.h>

// Ota için (For Ota)
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Türkçe Karakterli Fontumuz [Uyarı: Bazı diğer unicode karakterler devre dışı bırakıldı] (Font including Turkish characters; however, some other unicode characters are disabled)
#include "TR_Fonts_data.h"
#include "secrets.h"

// Matris ile ilgili (Parameters about dot matrix
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4       
#define CLK_PIN   D8        
#define DATA_PIN  D6
#define CS_PIN    D7

; // silme, yoksa hata veriyor: expected ',' or ';' before 'MD_Parola' ( do not erase, otherwise you will see error message:  expected ',' or ';' before 'MD_Parola')

// Matrisi kullamak için 'ekran' nesnesini oluştur (Create the object 'ekran' in order to control the dot matrix)
MD_Parola ekran = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Esp wifi server ile ilgili işlemler (About Esp wifi webserver)
WiFiServer server(80); // HTTP protokolü 80 portunu kullanır, o yüzden burayı 80 olarak ayarlıyoruz ( HTTP protocol uses port 80, so set it 80)

// ESP'ye statik IP veriyorum ki aradığım zaman kolayca bulabileyim.
// Siz başka bir IP adresi verebilirsiniz. Verdiğiniz adresi modemden rezerve ettiğinize ve bu adresin ağdaki başka bir alet tarafından kullanılmadığına emin olun.
// Set ESP's static IP address in order to find it easily later
// You may change that IP address and make sure the IP address is reserved from router and not other devices in your network are using that IP address.
IPAddress staticIP(192, 168, 1, 102); 
IPAddress gateway(192, 168, 1, 1);   // Modemin (routerın) IP adresi (IP address of your router)
IPAddress subnet(255, 255, 255, 0);  // Ağ alt maskesi (Subnet mask)
IPAddress dns(8, 8, 8, 8);  // DNS

String header; // HTTP isteklerini tutan değişken (Variable to store the HTTP requests)
String ttbp = "merhaba"; // Görüntülenecek metin (Text To Be Printed)
char curMessage3[250]; // Gönderilen metni tutan array (Array to store sent text)
bool canIreset = false;

//Bu linkten alıntıdır (copied and edited from this link):
//https://github.com/MajicDesigns/MD_Parola/blob/main/examples/Parola_UFT-8_Display/Parola_UFT-8_Display.ino

// Return "0" if a byte has to be ignored.
uint8_t utf8Ascii(uint8_t ascii){
  static uint8_t cPrev;
  uint8_t c = '\0';

  if (ascii < 0x7f)   // Standart ASCII karakter kümesi 0..0x7F, değişiklik yok
  {
    cPrev = '\0';
    c = ascii;
  }
  else
  {
    switch (cPrev)  // Öndeki UTF8 karaktere göre çevirme işlemi
    {
    case 0xC2: c = ascii;  break;
    case 0xC3: c = ascii | 0xC0;  break;
    case 0x82: if (ascii==0xAC) c = 0x80; // Euro sembolü
    }
    cPrev = ascii;   // Son karakteri kaydet
  }

  return(c);
}

void utf8Ascii(char* s)
// UTF-8 karakter dizisini (string) genişletilmiş ASCII'ye çevir.
// Genişletilmiş ASCII karakter dizgisi her zaman daha kısadır.
{
  uint8_t c;
  char *cp = s;
  while (*s != '\0')
  {
    c = utf8Ascii(*s++);
    if (c != '\0')
      *cp++ = c;
  }
  *cp = '\0';   // Yeni karakter dizisini (stringi) sonlandır
}


void printText(String str){
  ekran.setTextAlignment(PA_CENTER);
  char charArr[250] = {""};
  str.toCharArray(charArr, 100);
  utf8Ascii(charArr);
  if(ekran.displayAnimate()){
    ekran.displayText(charArr, PA_LEFT, 15, 2000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  }
}

void(* resetFunc) (void) = 0; // bunu çağırınca reset atıyor (when you call it, it resets the board)

String beautifyString(String inputStr){
  inputStr += "$$$"; // Biraz ekstra karakter ekle ki index error almayalım (add some dummy chars to prevent out of index error)
  String str = ""; // Güzelleştirilmiş mesajı depolayan değişken (the string holds beautified charset)
  for( int i = 0; inputStr[i] != '$'; i++){
    if(inputStr[i] == '_' && inputStr[i+1] == '!'){
      switch(inputStr[i+2]){
        
        case 'B':
        str += ">";
        i += 2;
        break;

        case 'K':
        str += "<";
        i += 2;
        break;

        case 'G':
        str += "À";
        i += 2;
        break;

        case 'S':
        str += "Æ";
        i += 2;
        break;

        case 'C':
        str += "Ç";
        i += 2;
        break;

        case 'I':
        str += "Ì";
        i += 2;
        break;
        
        case 'O':
        str += "Ö";
        i += 2;
        break;

        case 'U':
        str += "Ü";
        i += 2;
        break;    

        case 'g':
        str += "à";
        i += 2;
        break;
        
        case 's':
        str += "æ";
        i += 2;
        break;
        
        case 'c':
        str += "ç";
        i += 2;
        break; 
        
        case 'i':
        str += "ì";
        i += 2;
        break; 

        case 'o':
        str += "ö";
        i += 2;
        break;

        case 'u':
        str += "ü";
        i += 2;
        break;

        default:
        ;
      }
    }
   else{
     str += inputStr[i];
   }
  }
  return str;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  ekran.begin();
  ekran.setFont(TR);
  ekran.print("start");

  // Wifi ayarları (Wifi settings)
  WiFi.disconnect(); // Önceden bağlı bağlantı varsa kopart. (Disconnect from old connections)
  WiFi.begin(ssid, password);
  WiFi.hostname(deviceName);
  WiFi.mode(WIFI_STA); // Modu modeme bağlanma modu olarak ayarla. (Wifi mode STAtion) (Only connect the router)
  while (WiFi.status() != WL_CONNECTED) { // Bağlanana kadar döngüde dön. (Loop here until connected) 
    digitalWrite(LED_BUILTIN, LOW);
    delay(400);
    digitalWrite(LED_BUILTIN, HIGH);                
    delay(100);
    // Toplam 500 ms delay iyi oluyor. 400, 100 şeklinde ayırınca led efektli bir şekilde yanıp sönmüş oluyor. (For delay, 500 ms in total is good. Splitting it to 400 + 100 enables us to see blinking LED)                            
  }
  WiFi.config(staticIP, subnet, gateway, dns); // Wifi konfigürasyonlarını ayarla (Configure Wifi settings)
  server.begin(); // Serverı başlat )(start server)

  // Ota ayarları (Ota settings)
  // Arduino OTA örneğinden alıntıdır. (Adopted from Arduino OTA example sketch)
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    //Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    //Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    //Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      //Serial.println("Auth Failed");
      ;
    } else if (error == OTA_BEGIN_ERROR) {
      //Serial.println("Begin Failed");
      ;
    } else if (error == OTA_CONNECT_ERROR) {
      //Serial.println("Connect Failed");
      ;
    } else if (error == OTA_RECEIVE_ERROR) {
      //Serial.println("Receive Failed");
      ;
    } else if (error == OTA_END_ERROR) {
      //Serial.println("End Failed");
      ;
    }
  });
  ArduinoOTA.begin();

  ekran.setIntensity(15); // en fazla parlaklık (maximum brightness)
  ekran.print("ready");
}
void loop() {
    
  WiFiClient client = server.available(); // web arayüzünden gelen istekleri sisteme aktar (Transfer the requests coming from web clients)

  if (client) {                               // Yeni bir istemci bağlanırsa (if a new client connects)
    String currentLine = "";                  // Bu istemciden gelen istekleri tutacak bir değişken tanımla ( make a String to hold incoming data from the client)
    while (client.connected()) {              // İstemci bağlı olduğu sürece döngüde kal (loop while the client's connected)
      if (client.available()) {               // İstemcide okunacak byte varsa (if there's bytes to read from the client)
        char c = client.read();               // O byte'ı oku (read a byte)
        header += c;
        if (c == '\n') {                      // if the byte is a newline character
          // gelen satır boşsa, 2 tane yeni satır karakterimiz vardır demektir (if the current line is blank, you got two newline characters in a row)
          // bu gelen HTTP sorgusunun sonuna ulaştığımız anlamna gelir; bu yüzden cevabımızı gönderiyoruz  (that's the end of the client HTTP request, so send a response)
          if (currentLine.length() == 0) {
            // HTTP başlıkları her zaman bir cevap koduyla başar, örneğin HTTP/1.1 200 OK ( HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK) )
            // buna ek olarak, istemcinin gelen bilginin türünü anlaması için bir içerik tipi bilgisi gönderilir; son olarak da sadece boş satır ( and a content-type so the client knows what's coming, then a blank line)
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
              // Eğer istemci görüntülenecek yeni metin göndermişse (if client wants to change the text being printed)
              // İstemciden gelen metin bilgisini alıyoruz (store the text data that the client has requested)
             if (header.indexOf("GET /setText/$") >= 0) {
              int index = header.indexOf('$') + 1;
              String mystr = "";
              for(; header[index] != '$'; index++){
                // ardı ardına iki tane "_" ("__") gelirse yerine " " (boşluk) yaz (replace "__" ("_" + "_") with " " (space) )
                if(header[index] == '_' && header[index+1] =='_'){ 
                  mystr += ' ';
                  index++;                      
                }
                else{
                  mystr += header[index];
                }
              }
              ttbp = beautifyString(mystr);
            }
          
  
            // Reset butonuna basılırsa reset atmaya hazırlan (prepare to reset if reset button clicked)
            else if (header.indexOf("GET /reboot") >= 0)
            {
              canIreset = true;
            }

            // Websayfasını görüntüle (display webpage)
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta charset='utf-8' name='viewport' content='width=device-width, initial-scale=1'>");
            client.println("<style>html, body { font-family: Helvetica; display: block; margin: 0px; text-align: left; padding-left: 15px; background-color: rgb(37, 37, 38);}");
            client.println();
            client.println(".button3 { background-color: #14b2b8; border: none; color: white;  width: 200px; padding: 12px 24px;  ");
            client.println("text-decoration: none; font-size: 20px;  cursor: pointer;}");
            client.println();
            client.println("</style></head>");
            client.println();
            client.println();
            client.println("<body><h1 style='color:white;'>Matrix Kontrol</h1>");
            client.println("  <div><a href='/reboot' style='display: block;' ><button class='button3'>Reset Matrix</button></a></div>");
            client.println("</body></html>");
      
            // Break out of the while loop
            break;
        }
        else { // yeni satır varsa, currentLine'ı temizle (if you got a newline, then clear currentLine)
          currentLine = "";
        }
       } 
       else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
     }
   }
    // Header değişkenini temizle (Clear the header variable)
    header = "";
    // İstemciyle olan bağlantıyı kapat (Close the connection between server and client)
    client.stop();
  }

  // işleri hallet (handle things)
  ArduinoOTA.handle(); 
  printText(ttbp);
  if(canIreset){
    ekran.setTextAlignment(PA_CENTER);
    ekran.print("wait");
    delay(1000); // Daha randımanlı oluyor (for consistency)
     resetFunc();
  }

}
