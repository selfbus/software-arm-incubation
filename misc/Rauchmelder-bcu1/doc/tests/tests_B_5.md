# Purpose
Parameters regarding periodical sending of Test Alarm respected.

# Results

| Test case | Tester | Date | Commit | Result |
| --- | --- | --- | --- | :---: |
| B.5.a | @dallmair  | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|       | @Darthyson | 2024-11-21 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |
| B.5.b | @dallmair  | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|       | @Darthyson | 2024-11-21 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |
| B.5.c | @dallmair  | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|       | @Darthyson | 2024-11-21 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |
| B.5.d | @dallmair  | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|       | @Darthyson | 2024-11-21 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |
| B.5.e | @dallmair  | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|       | @Darthyson | 2024-11-21 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |

# Setup
Setup per finished [Tests B.1](tests_B_1.md), i.e. fully configured module mounted in smoke alarm device, and Group Monitor running.

# Test Cases

## B.5.a

**Steps:**
* In ETS, set parameters on Testalarm page as follows
  - Send test-alarm status periodically: **Yes**
  - Send test-alarm network periodically: **Yes**
  - Interval [sec]: **5**
* Download the changes to the device
* Send a message on the bus to set `Testalarm Network` (communication object 1) to value `1`

**Verify:** Does not send any message, because the test alarm has not been triggered on the device itself

**Step**: Send a message on the bus to set `Testalarm Network` (communication object 1) to value `0`

**Verify:** Does not send any message

## B.5.b

**Steps:**
* Send a message on the bus to set `Testalarm Network` (communication object 1) to value `1`
* Silence test alarm via button on the smoke alarm device

**Verify:** Sends message on the bus to set `Alarm Reset` (communication object 2) to value `1`

**Step**: Wait for 1 minute before starting the next test

## B.5.c

**Steps:**
* In ETS, set parameters on Status Informations page as follows
  - Send status informations periodically: **Yes**
  - Interval [min]: **1**
  - \<all switches>: **Yes**
  - Send operating time as: **seconds (DPT 13.100)**
* Download the changes to the device
* Trigger test alarm via button on the smoke alarm device

**Verify:**
* Sends messages on the bus to set both `Testalarm Network` (communication object 1) and `Test-alarm Status` (communication object 5) to value `1` every 5 seconds
* Does not send any Status Informations communication objects while alarm is on

**Step:** Silence test alarm via button on the smoke alarm device

**Verify:**
* Sends messages on the bus to set both `Testalarm Network` (communication object 1) and `Test-alarm Status` (communication object 5) to value `0`
* Continues sending Status Informations communication objects

## B.5.d

**Steps:**
* In ETS, set parameters on Testalarm page as follows
  - Send test-alarm status periodically: Yes
  - Send test-alarm network periodically: **No**
  - Interval [sec]: 5
* Download the changes to the device
* Trigger test alarm via button on the smoke alarm device

**Verify:** Sends messages on the bus to set both `Testalarm Network` (communication object 1) and `Test-alarm Status` (communication object 5) to value `1`; Only `Test-alarm Status` (communication object 5) is repeated every 5 seconds

**Step:** Silence test alarm via button on the smoke alarm device

**Verify:** Sends messages on the bus to set both `Testalarm Network` (communication object 1) and `Test-alarm Status` (communication object 5) to value `0`

## B.5.e

**Steps:**
* In ETS, set parameters on Testalarm page as follows
  - Send test-alarm status periodically: **No**
  - Send test-alarm network periodically: **Yes** (set this before setting "Send test-alarm status periodically" to No)
  - Interval [sec]: 5
* Download the changes to the device
* Trigger test alarm via button on the smoke alarm device

**Verify:** Sends messages on the bus to set both `Testalarm Network` (communication object 1) and `Test-alarm Status` (communication object 5) to value `1` once

**Step:** Silence test alarm via button on the smoke alarm device

**Verify:** Sends messages on the bus to set both `Testalarm Network` (communication object 1) and `Test-alarm Status` (communication object 5) to value `0`
