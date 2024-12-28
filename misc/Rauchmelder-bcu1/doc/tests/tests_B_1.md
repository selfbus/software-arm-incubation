# Purpose
Startup sequence and timing correct.

# Results

| Test case | Device | Tester | Date | Commit | Result |
| --- | --- | --- | --- | --- | :---: |
| B.1 | Dual VdS | @dallmair  | 2023-12-20 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|     | Dual Q   | @Doumanix  | 2023-12-23 | a6e41e38be984ce5c6b8985c9ec173a85ef35d48 | :ok: |
|     | Dual Q   | @Darthyson | 2024-11-20 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |

# Setup
* Setup per [Tests A](tests_A.md)
* Mount the module into a smoke alarm device
* For a Dual VdS device, detach the battery
* For a Dual Q device, keep the base plate unmounted

# Test Cases

## B.1

**Step:** Connect module to KNX bus

**Verify:**
* On startup, 2 status LEDs turn on
* At the same time, it sends the `Error code` and `Smoke detector malfunction` group objects (communication objects 12 and 14) with `some value != 0` and `true`
* In the next minute, nothing else is sent on the bus

**Step:** Connect battery (Dual VdS) / Mount smoke alarm device on base plate (Dual Q)

**Verify:**
* Sends the `Error code` and `Smoke detector malfunction` group objects (communication objects 12 and 14) with values `0` and `false` immediately
* Sends all informational group objects (except communication object 5 `Test alarm status`) with sensible values within the first minute of operation
