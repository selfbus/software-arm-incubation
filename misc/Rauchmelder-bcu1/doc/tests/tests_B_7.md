# Purpose
Delayed sending of Alarm implemented correctly.

# Results

| Test case | Tester | Date | Commit | Result |
| --- | --- | --- | --- | :---: |
| B.7.a | @dallmair  | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :x:  |
|       | @dallmair  | 2023-12-22 | a6e41e38be984ce5c6b8985c9ec173a85ef35d48 | :ok: |
|       | @Darthyson | 2024-11-21 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |
| B.7.b | @dallmair  | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|       | @Darthyson | 2024-11-21 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |

# Setup
Setup per finished [Tests B.1](tests_B_1.md), i.e. fully configured module mounted in smoke alarm device, and Group Monitor running.

# Test Cases

## B.7.a

**Steps:**
* In ETS, set parameters on Alarm page as follows
  - Send alarm status periodically: **Yes**
  - send 0 periodically: **No**
  - Send alarm network periodically: **Yes**
  - Interval [sec]: **5**
  - Send alarm delayed: **Yes**
  - Delay [sec]: **10**
  - Add `Delayed Alarm` (communication object 4) to a groupadress
* Download the changes to the device
* Trigger alarm with a hair dryer

**Verify:**
* Sends messages on the bus to set both `Alarm Status` (communication object 3) and `Delayed Alarm` (communication object 4) to value `1`
* After 5 seconds, repeats message to set `Alarm Status` (communication object 3) to value `1`
* After 10 seconds, repeats message to set `Alarm Status` (communication object 3) to value `1` and sends messages to set `Delayed Alarm` (communication object 4) to value `0` and `Alarm Network` (communication object 0) to value `1`
* Repeats messages to set both `Alarm Network` (communication object 0) and `Alarm Status` (communication object 3) to value `1` every 5 seconds

**Step:** Silence alarm via button on the smoke alarm device

**Verify:** Sends messages on the bus to set both `Alarm Network` (communication object 0) and `Alarm Status` (communication object 3) to value `0`

## B.7.b

**Steps:**
* Trigger alarm with a hair dryer
* Within the first 10 seconds of alarm, send a message on the bus to set `Alarm Reset` (communication object 2) to value `1`

**Verify:** Sends message on the bus to set `Delayed Alarm` (communication object 4) to value `0`

**Step:** Silence alarm via button on the smoke alarm device

**Verify:** Sends messages on the bus to set both `Alarm Network` (communication object 0) and `Alarm Status` (communication object 3) to value `0`

**Step**: Wait for 1 minute before starting the next test
