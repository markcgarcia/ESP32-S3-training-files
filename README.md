# ESP32-S3-training-files
Practice with ESP32-S3 microcontroller. In preparation for full project (which will be in a separate repository).

| File | Description| 
| --- | --- |
| blink | Very basic blink program. Implemented with GPTimer and uses register writes to toggle an external LED. | 
| uart | Register-level UART implementation, including playing around with routing signals with the IO mux. | 
| espnow | ESPNOW firmware for the transmitting boards. |
| espnow2 | ESPNOW firmware for the receiving board (hub). |
| i2c | Basic MPU6050 device driver that allows it to send information to ESP32. |
