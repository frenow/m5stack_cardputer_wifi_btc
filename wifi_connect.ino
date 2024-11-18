#include "M5Cardputer.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

int menuPosition = 0;
int currentOption = 1;
int offset = 15;

const int MAX_NETWORKS = 8;  // Maximum number of networks to display
int startIdx = 0;            // Start index for displaying networks
int numNetworks = 0;

bool inputText = false;
bool inputTextComplete  = false;
String data = "> ";

String ssid = "";
String password = ""; 

  //const char* ssid = "2G_CLARO04";
  //const char* password = "3714829000"; 

void drawNetworks() {
  M5Cardputer.Display.setCursor(1, 1);  // Set the cursor position
  if (numNetworks == 0) {
    M5Cardputer.Lcd.print("No WiFi found.");
  } else {
    for (int i = 0; i < min(startIdx + MAX_NETWORKS, numNetworks); ++i) {
       int yPos = (i - startIdx) * offset; // Adjust the vertical spacing as needed
       M5Cardputer.Display.setCursor(1, yPos);  // Set the cursor position
       M5Cardputer.Lcd.printf("%d-%s", i + 1,WiFi.SSID(i).c_str());        
    }
  }
}

void drawMenu() {
  M5Cardputer.Display.clear();
  M5Cardputer.Display.fillRect(0, menuPosition, 240, 9, 0xBDF7); 
  //M5Cardputer.Display.setCursor(183, 122);  // Set the cursor position
  //M5Cardputer.Display.printf("Batt: %d%%", M5Cardputer.Power.getBatteryLevel());
  if(WiFi.status() == WL_CONNECTED) M5Cardputer.Display.drawString("ON", M5Cardputer.Display.width() - 20, M5Cardputer.Display.height() - 10);
  else M5Cardputer.Display.drawString("OFF", M5Cardputer.Display.width() - 20, M5Cardputer.Display.height() - 10);
  drawNetworks();
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);

    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextColor(0xFFFF);

    numNetworks = WiFi.scanNetworks();
    drawMenu();
}

void reqhttp() {
    // Perform HTTP GET request
    HTTPClient http;
    String url = "https://api.coincap.io/v2/assets/bitcoin";
    http.begin(url);

    int httpResponseCode = http.GET(); // Make the request

    if (httpResponseCode > 0) { // Check for the returning code
      String payload = http.getString();

      JsonDocument doc;

      // Parse the JSON payload
      DeserializationError error = deserializeJson(doc, payload);
      
      M5Cardputer.Display.setCursor(120, 66);  // Set the cursor position 
      
      if (!error) {
        // Extract the priceUsd value
        float priceUsd = doc["data"]["priceUsd"];
          M5Cardputer.Display.printf("BTC: %f",priceUsd);
      } else {
          M5Cardputer.Display.print("Error get request ");
      }

    }

  http.end();
}

void inputKeyboard() {

  if (M5Cardputer.Keyboard.isPressed()) {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

      for (auto i : status.word) {
          data += i;
      }

      if (status.del) {
          data.remove(data.length() - 1);
      }

      if (status.enter) {
          data.remove(0, 2);
          inputTextComplete = true;
      }
      M5Cardputer.Display.drawString(data, 2, M5Cardputer.Display.height() - 10);

    }
}

void loop() {
    
    M5Cardputer.update();
    if (M5Cardputer.Keyboard.isChange()) {
        if (M5Cardputer.Keyboard.isKeyPressed(';')) {
          if(currentOption > 1) {
            currentOption--;
            menuPosition -= offset;
            drawMenu();
          }
        }else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
            if(currentOption < MAX_NETWORKS) {
            currentOption++;
            menuPosition += offset;
            drawMenu();
            }
        } else if ((M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) && (inputText == false)) {
            data = "> ";
            M5Cardputer.Display.drawString(data, 2, M5Cardputer.Display.height() - 10);
            inputText = true;
        } else if (inputText){ 
              inputTextComplete = false;
              inputKeyboard();

              if (inputTextComplete) {
                    ssid = WiFi.SSID(currentOption-1).c_str();
                    password = data;

                    WiFi.begin(ssid, password);

                    M5Cardputer.Display.setCursor(80, 122);  // Set the cursor position    
                    int i = 0;
                    while (WiFi.status() != WL_CONNECTED) {
                        i++;
                        delay(500);
                        M5.Lcd.print(".");
                      
                        if (i >= 20){   //timeout connection
                          drawMenu();
                          M5Cardputer.Display.setCursor(120, 122);  // Set the cursor position    
                          M5Cardputer.Display.print("failed!");
                          inputText = false;
                          break;
                        }
                    }

                    if(WiFi.status() == WL_CONNECTED) {
                        drawMenu();
                        M5Cardputer.Display.setCursor(120, 122);  // Set the cursor position    
                        M5Cardputer.Display.print("connected!");
                        inputText = false;
                        reqhttp();
                    }  
              }
          }        
    }  
}