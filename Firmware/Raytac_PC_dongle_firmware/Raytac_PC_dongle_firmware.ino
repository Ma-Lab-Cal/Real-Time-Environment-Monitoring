
#include <bluefruit.h>

BLEClientBas  client_battery_info;  // battery client
BLEClientDis  client_device_info;  // device information client
BLEClientUart client_uart; // bleuart client


void onScanHandler(ble_gap_evt_adv_report_t* report) {
  if (Bluefruit.Scanner.checkReportForService(report, client_uart)) {
    Bluefruit.Central.connect(report);
  }
  else {      
    // For Softdevice v6: after received a report, scanner will be paused
    // We need to call Scanner resume() to continue scanning
    Bluefruit.Scanner.resume();
  }
}


void onConnectHandler(uint16_t conn_handle) {
  // manufacturer information
  if (client_device_info.discover(conn_handle)) {
    char buf[32+1];
    
    memset(buf, 0, sizeof(buf));
    if (client_device_info.getManufacturer(buf, sizeof(buf))) {
      // `buf` will store the manufacturer name
    }

    memset(buf, 0, sizeof(buf));
    if (client_device_info.getModel(buf, sizeof(buf))) {
      // `buf` will store the model name
    }
  }

  // battery information
  if (client_battery_info.discover(conn_handle)) {
    int battery = client_battery_info.read();
  }

  if (client_uart.discover(conn_handle)) {
    client_uart.setTimeout(100);
    client_uart.enableTXD();
  }
  else {
    Bluefruit.disconnect(conn_handle);
  }
}

void onDisconnectHandler(uint16_t conn_handle, uint8_t reason) {
}

void onBLE_UART_RXHandler(BLEClientUart& uart_svc) {
  if (uart_svc.available()) {
    uint8_t buf[128];
    uint16_t size = uart_svc.readBytesUntil('\n', buf, 128);
    Serial.write(buf, size);

    // we DO need to send another newline character here.
    // `uart_svc.readBytesUntil()` will NOT store the ending newline char into the buf.
    Serial.write('\n');
  }
}

void BLE_init(void) {
  Bluefruit.begin(0, 1);
  
  Bluefruit.setName("CentralNode");

  client_battery_info.begin();  

  client_device_info.begin();

  client_uart.begin();
  client_uart.setRxCallback(onBLE_UART_RXHandler);

  // Increase Blink rate to different from PrPh advertising mode
  Bluefruit.setConnLedInterval(250);

  Bluefruit.Central.setConnectCallback(onConnectHandler);
  Bluefruit.Central.setDisconnectCallback(onDisconnectHandler);

  Bluefruit.Scanner.setRxCallback(onScanHandler);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms
  Bluefruit.Scanner.useActiveScan(false);
  Bluefruit.Scanner.start(0);                   // // 0 = Don't stop scanning after n seconds
}

void setup() {
  Serial.begin(115200);
  
  while (!Serial) {}
  
  Serial.setTimeout(100);
  
  BLE_init();
}

void loop() {
  if (Bluefruit.Central.connected() && client_uart.discovered()) {
    if (Serial.available()) {
      uint8_t buf[128];
      uint16_t size = Serial.readBytesUntil('\n', buf, 128);
      client_uart.write(buf, size);
      
      // we DO NOT need to send another newline character here.
      // `Serial.readBytesUntil()` will store the ending newline char into the buf.
    }
  }
}
