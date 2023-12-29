# Purpose
Unmounted modules do not interfere with operating modules on the bus.

# Results

| Test case | Tester | Date | Commit | Result |
| --- | --- | --- | --- | :---: |
| A | @dallmair | 2023-12-20 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |

# Setup
* Use a single module, unmounted (not connected to smoke alarm device)
* Connect the module to the KNX bus
* In ETS
  - Configure a physical address per your topology
  - Turn on all periodic sending options
    + Alarm page
      * Send alarm status periodically: **Yes**
      *   send 0 periodically: **Yes**
      * Send alarm network periodically: **Yes**
      * Interval [sec]: **60**
      * Send alarm delayed: **No**
    + Testalarm page
      * Send test-alarm status periodically: **Yes**
      * Send test-alarm network periodically: **Yes**
      * Interval [sec]: **60**
    + Status Informations page
      * Send status informations periodically: **Yes**
      * Interval [min]: **1**
      * Serial number: **Yes**
      * Operating time: **Yes**
      * Send operating time as: **seconds (DPT 13.100)**
      * \<all others>: **Yes**
    + Temperature page
      * Temperature-offset in 0.1K: **0**
  - Assign a group address to every communication object
  - Download everything to the device
  - Start Group Monitor
* Disconnect device from KNX bus

# Test Cases

## A

**Step:** Reconnect module to KNX bus

**Verify:**
* On startup, 2 status LEDs turn on
* At the same time, it sends the `Error code` and `Smoke detector malfunction` group objects (communication objects 12 and 14) with `some value != 0` and `true`
* In the next 3 minutes, nothing else is sent on the bus
