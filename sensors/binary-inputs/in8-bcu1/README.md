# in8-bcu1

This is an example application that uses the SBLib with BCU1 emulation.
The emulated device is a Jung 2118 with 8 digital inputs.

Please use the Jung 2118 VD for ETS configuration from this site:
https://downloads.jung.de/public/de/software/produktdatenbanken/ets2_ets3/de_2118_reg.vd1

## Used IO Ports

### lpc1115_4MU_MID controller
| Function | Selfbus 26-pol | Selfbus IO | ARM-GPIO |
|:--------:|:--------------:|:----------:|----------|
| Input 1  |       7        |    IO1     | PIO2_2   |
| Input 2  |       8        |    IO2     | PIO0_7   |
| Input 3  |       9        |    IO3     | PIO2_10  |
| Input 4  |       10       |    IO4     | PIO2_9   |
| Input 5  |       13       |    IO5     | PIO0_2   |
| Input 6  |       14       |    IO6     | PIO0_8   |
| Input 7  |       15       |    IO7     | PIO0_9   |
| Input 8  |       16       |    IO8     | PIO2_11  |
|   GND    |     2, 21      |            |          |
|   3.3V   |       5        |            |          |

### TS_ARM
| Function | ARM-GPIO |
|:--------:|----------|
| Input 1  | PIO2_2   |
| Input 2  | PIO0_9   |
| Input 3  | PIO2_11  |
| Input 4  | PIO1_1   |
| Input 5  | PIO3_0   |
| Input 6  | PIO3_1   |
| Input 7  | PIO3_2   |
| Input 8  | PIO2_9   |
