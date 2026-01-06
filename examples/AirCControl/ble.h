/*#include <BLEDevice.h>

void setup() {
  Serial.begin(115200);

  // Initialize BLE
  BLEDevice::init("");

  // Get the BLE address
  BLEAddress bleAddress = BLEDevice::getAddress();
  
  Serial.print("ESP32 BLE MAC Address: ");
  Serial.println(bleAddress.toString().c_str());
}

void loop() {
  // Do nothing in the loop
}
*/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


/************************* BLE **********************************/
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);

        Serial.println();
        Serial.println("*********");

        uint8_t tempValue = (uint8_t) (std::stoi(rxValue));
        pTxCharacteristic->setValue(&tempValue, 1);
        pTxCharacteristic->notify();
        
      }
    }
};
