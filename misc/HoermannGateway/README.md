# Hoermann Gateway

## Tools
- [logmx hoermann parser](tools/logmx_hoermann_parser) - Simple logMX Java parser to decode serial Hoermann debug logs.

### TS_ARM PCB top connector

| Pin | PIO     | Function        |
|:---:|---------|-----------------|
|  1  |         | GND             |
|  3  |         | +3.3V           |
|  5  |         | EIB_DC          |
|  7  | PIO1_1  |                 |
|  9  | PIO2_2  |                 |
| 11  | PIO0_9  | LED 2           |
| 13  | PIO2_1  | LED 1           |
| 15  | PIO3_0  | Tx UART (RS485) |
| 17  | PIO3_1  | Rx UART (RS485) |
| 19  | PIO3_2  | Break trigger   |

### TS_ARM PCB bottom connector

| Pin | PIO     | Function              |
|:---:|---------|-----------------------|
|  2  |         |                       |
|  4  |         |                       |
|  6  |         |                       |
|  8  |         |                       |
| 10  |         |                       |
| 12  |         |                       |
| 14  |         |                       |
| 16  | PIO1_2  | Tx software UART      |
| 18  | PIO2_3  | Rx software UART      |
| 20  | PIO1_5  | Driver enable (RS485) |