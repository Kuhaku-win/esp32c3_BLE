#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *ptServer = nullptr;
BLECharacteristic *ptCharacteristic = nullptr;
bool deviceConnected = false;

// UUID定义
#define SERVICE_UUID "4fa464c0-4b30-44b8-8c77-1e836ecf95f3"
#define CHARACTERISTIC_UUID "9e7d657e-4a8d-4b8c-9259-458eb255eb1e"

// 连接状态回调
class Callback: public BLEServerCallbacks {
    void onConnect(BLEServer* ptServer) {
        deviceConnected = true;
    }

    void onDisconnect(BLEServer* ptServer) {
        deviceConnected = false;
    }
};

void setup() {
    Serial.begin(115200);
    BLEDevice::init("ESP32C3_BLE");  // 设置设备名称

    ptServer = BLEDevice::createServer();
    ptServer->setCallbacks(new Callback());
    
    BLEService *pService = ptServer->createService(SERVICE_UUID);

    //配置Characteristic
    ptCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ | 
        BLECharacteristic::PROPERTY_WRITE | 
        BLECharacteristic::PROPERTY_NOTIFY
    );

    ptCharacteristic->addDescriptor(new BLE2902());
    pService->start();
    
    BLEAdvertising *pAdvertising = ptServer->getAdvertising();
    pAdvertising->start();//开始广播
    Serial.println("开始连接");
}

void loop() {
    if (deviceConnected) {
        // 读取手机串口输入
        if (ptCharacteristic->getValue().length() > 0) {
            String message = ptCharacteristic->getValue().c_str();
            Serial.print("From BLE: ");
            Serial.println(message);  // 在电脑串口打印手机输入内容
            ptCharacteristic->setValue("");  // 清空特征值
        }

        // 从电脑串口发送数据到手机
        if (Serial.available()) {
            String mes = Serial.readString();
            ptCharacteristic->setValue(mes.c_str());
            ptCharacteristic->notify();  // 通知手机有新数据
            Serial.print("Sent to BLE: ");
            Serial.println(mes);  // 打印发送到手机的数据
        }
    }
}
