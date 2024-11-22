# Purpose
Ensure correct handling of alarms when smoke alarm devices are networked using both the green networking terminal and KNX bus.

# Results

| Test case | Tester | Date | Commit | Result |
| --- | --- | --- | --- | :---: |
| C.2.a | @dallmair  | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|       | @Darthyson | 2024-11-22 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |
| C.2.b | @dallmair  | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|       | @Darthyson | 2024-11-22 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |

# Setup
Hardware setup per [Tests C.1](tests_C_1.md).

Both connected KNX modules are configured to have alarm networking enabled:
* In ETS, set parameters on Alarm page as follows
  - Send alarm status periodically: **No**
  - Send alarm delayed: **No**
* In ETS, ensure `Alarm Network` (communication object 0) is linked to the same group address
* Download the changes to the devices
* Start Group Monitor

# Test Cases

## C.2.a

**Step:** Trigger alarm via hair dryer on Device 1

**Verify:** Device 2 sends messages on the bus to set both `Alarm Network` (communication object 0) and `Alarm Status` (communication object 3) to value `1`

**Step:** Silence alarm via button on Device 1

**Verify:** Device 2 sends messages on the bus to set both `Alarm Network` (communication object 0) and `Alarm Status` (communication object 3) to value `0`

## C.2.b

**Step:**
* Trigger alarm via hair dryer on Device 1
* Try to silence alarm via button on Device 2

**Verify:** Nothing happens (it cannot silence the wired alarm, and it does not have bus but local alarm, so it does not silence the bus alarm via `Alarm Reset`)

**Step:** Try to silence alarm via button on Device 3

**Verify:**
* Device 3 is silenced
* Device 3 sends message on the bus to set `Alarm Reset` (communication object 2) to value `1`

**Step:** Silence alarm via button on Device 1

**Verify:** Device 2 sends messages on the bus to set both `Alarm Network` (communication object 0) and `Alarm Status` (communication object 3) to value `0`

**Step**: Wait for 1 minute before starting the next test
