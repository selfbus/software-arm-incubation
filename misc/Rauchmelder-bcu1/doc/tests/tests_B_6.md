# Purpose
Parameters regarding periodic sending of Alarm respected.

# Results

| Test case | Tester | Date | Commit | Result |
| --- | --- | --- | --- | :---: |
| B.6.a | @dallmair  | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|       | @Darthyson | 2024-11-21 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |
| B.6.b | @dallmair  | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|       | @Darthyson | 2024-11-21 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |
| B.6.c | @dallmair  | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|       | @Darthyson | 2024-11-21 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |
| B.6.d | @dallmair  | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|       | @Darthyson | 2024-11-21 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |
| B.6.e | @dallmair  | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|       | @dallmair  | 2023-12-29 | a6e41e38be984ce5c6b8985c9ec173a85ef35d48 | :ok: |
|       | @Darthyson | 2024-11-21 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |

# Setup
Setup per finished [Tests B.1](tests_B_1.md), i.e. fully configured module mounted in smoke alarm device, and Group Monitor running.

# Test Cases

## B.6.a

**Steps:**
* In ETS, set parameters on Alarm page as follows
  - Send alarm status periodically: **Yes**
  - send 0 periodically: **No**
  - Send alarm network periodically: **Yes**
  - Interval [sec]: **5**
  - Send alarm delayed: **No**
* Download the changes to the device
* Send a message on the bus to set `Alarm Network` (communication object 0) to value `1`

**Verify:** Does not send any message, because the alarm has not been triggered on the device itself

**Step**: Send a message on the bus to set `Alarm Network` (communication object 0) to value `0`

**Verify:** Does not send any message

## B.6.b

**Steps:**
* Send a message on the bus to set `Alarm Network` (communication object 0) to value `1`
* Silence alarm via button on the smoke alarm device

**Verify:** Sends message on the bus to set `Alarm Reset` (communication object 2) to value `1`

**Step**: Wait for 1 minute before starting the next test

## B.6.c

**Step:** Trigger alarm with a hair dryer

**Verify:** Sends messages on the bus to set both `Alarm Network` (communication object 0) and `Alarm Status` (communication object 3) to value `1` every 5 seconds

**Step:** Silence alarm via button on the smoke alarm device

**Verify:** Sends messages on the bus to set both `Alarm Network` (communication object 0) and `Alarm Status` (communication object 3) to value `0`

## B.6.d

**Steps:**
* In ETS, set parameters on Alarm page as follows
  - Send alarm status periodically: Yes
  - send 0 periodically: No
  - Send alarm network periodically: **No**
  - Interval [sec]: 5
  - Send alarm delayed: No
* Download the changes to the device
* Trigger alarm with a hair dryer

**Verify:** Sends messages on the bus to set both `Alarm Network` (communication object 0) and `Alarm Status` (communication object 3) to value `1`; Only `Alarm Status` (communication object 3) is repeated every 5 seconds

**Step:** Silence alarm via button on the smoke alarm device

**Verify:** Sends messages on the bus to set both `Alarm Network` (communication object 0) and `Alarm Status` (communication object 3) to value `0`

## B.6.e

**Steps:**
* In ETS, set parameters on Alarm page as follows
  - Send alarm status periodically: **No**
  - Send alarm network periodically: **Yes** (set this before setting "Send alarm status periodically" to No)
  - Send alarm delayed: No
* Download the changes to the device
* Trigger alarm with a hair dryer

**Verify:** Sends messages on the bus to set both `Alarm Network` (communication object 0) and `Alarm Status` (communication object 3) to value `1` once

**Step:** Silence alarm via button on the smoke alarm device

**Verify:** Sends messages on the bus to set both `Alarm Network` (communication object 0) and `Alarm Status` (communication object 3) to value `0`
