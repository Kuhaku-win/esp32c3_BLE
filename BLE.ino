#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;
bool deviceConnected = false;

// UUID定义
#define SERVICE_UUID "4fa464c0-4b30-44b8-8c77-1e836ecf95f3"
#define CHARACTERISTIC_UUID "9e7d657e-4a8d-4b8c-9259-458eb255eb1e"

// 连接状态回调
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
    }
};

void setup() {
    Serial.begin(115200);
    BLEDevice::init("ESP32C3_BLE");  // 设置设备名称

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ | 
        BLECharacteristic::PROPERTY_WRITE | 
        BLECharacteristic::PROPERTY_NOTIFY
    );

    pCharacteristic->addDescriptor(new BLE2902());
    pService->start();
    
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
    Serial.println("BLE Server is ready. Connect with your phone.");
}

void loop() {
    if (deviceConnected) {
        // 读取手机串口输入
        if (pCharacteristic->getValue().length() > 0) {
            String message = pCharacteristic->getValue().c_str();
            Serial.print("From BLE: ");
            Serial.println(message);  // 在电脑串口打印手机输入内容
            pCharacteristic->setValue("");  // 清空特征值
        }

        // 从电脑串口发送数据到手机
        if (Serial.available()) {
            String pcInput = Serial.readString();
            pCharacteristic->setValue(pcInput.c_str());
            pCharacteristic->notify();  // 通知手机有新数据
            Serial.print("Sent to BLE: ");
            Serial.println(pcInput);  // 打印发送到手机的数据
        }
    }
}
