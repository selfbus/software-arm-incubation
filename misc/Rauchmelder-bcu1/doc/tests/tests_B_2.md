# Purpose
Parameters regarding periodic sending of `Alarm Status` `0` respected, i.e. when there is no alarm.

# Results

| Test case | Tester | Date | Commit | Result |
| --- | --- | --- | --- | :---: |
| B.2.a | @dallmair | 2023-12-20 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
| B.2.b | @dallmair | 2023-12-20 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
| B.2.c | @dallmair | 2023-12-20 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |

# Setup
Setup per finished [Tests B.1](tests_B_1.md), i.e. fully configured module mounted in smoke alarm device, and Group Monitor running.

# Test Cases

## B.2.a

**Steps:**
* In ETS, set parameters on Alarm page as follows
  - Send alarm status periodically: **Yes**
  - send 0 periodically: **Yes**
  - Send alarm network periodically: **Yes**
  - Interval [sec]: **10**
  - Send alarm delayed: **No**
* Download the changes to the device

**Verify:** Sends `Alarm Status` (communication object 3) with value `0` every 10 seconds

## B.2.b

**Steps:**
* In ETS, set parameters on Alarm page as follows
  - Send alarm status periodically: Yes
  - send 0 periodically: **No**
  - Send alarm network periodically: Yes
  - Interval [sec]: 10
  - Send alarm delayed: No
* Download the changes to the device

**Verify:** Does not send `Alarm Status` (communication object 3) at all

## B.2.c

**Steps:**
* In ETS, set parameters on Alarm page as follows
  - Send alarm status periodically: **No**
  - send 0 periodically: **Yes** (set this before setting "Send alarm status periodically" to No)
  - Send alarm network periodically: Yes
  - Interval [sec]: 10
* Download the changes to the device

**Verify:** Does not send `Alarm Status` (communication object 3) at all
