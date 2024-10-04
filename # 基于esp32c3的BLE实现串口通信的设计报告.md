# 基于esp32c3的BLE实现串口通信的设计报告
## 大纲
 - 为什么选择arduino
 - 心路历程
 - 技术介绍
## 为什么选择arduino
经过前期了解，开发esp32c3主要有使用vscode+ioplatform和vscode+esp-idf还有基于linux的esp-idf+vscode
但是由于前期配置环境的种种问题（主要指使用espidf在使用clion开发工具链时cmake丢失），并且由于不了解团队工作了流和开发环境，所以选择简单的ardunio

## 心路历程 
刚开始因为在stm32,,mspm0的板子上实现过同样的功能，所以并未在意，之后在开发过程中，最难顶的是配置开发环境，配了5个小时最后还是选择arduino，有点难顶。其次在开发过程中，似乎是esp32c3芯片的包中不支持经典蓝牙和bluetoothSeria.h的库所以无法使用经典蓝牙的实现方法无奈学习BLE（低功耗蓝牙）实现手机串口到电脑的无线串口的开发

## 技术介绍
首先创建回调函数，确定连接状态

```
class MyServerCallbacks: public BLEServerCallbacks {
        void onConnect(BLEServer* pServer) {
            deviceConnected = true;
        }

        void onDisconnect(BLEServer* pServer) {
            deviceConnected = false;
        }
    };
```
其次使用BLE，并且启动设备创建服务器，并且创建服务，最后配置BLECharacteristic，并且启动服务，开始广播蓝牙信号
```
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
```
接下来，在loop中编写代码，实时监测电脑和手机输入内容并且打印完成后实时清空value，通过修改pCharacteristic实时决定是否推送信息
```
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
```