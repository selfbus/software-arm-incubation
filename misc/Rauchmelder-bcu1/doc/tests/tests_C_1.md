# Purpose
Ensure correct handling of test alarms when smoke alarm devices are networked using both the green networking terminal and KNX bus.

# Results

| Test case | Tester | Date | Commit | Result |
| --- | --- | --- | --- | :---: |
| C.1.a | @dallmair | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
| C.1.b | @dallmair | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |

# Setup
Use three smoke alarm devices in the following setup:
* Device 1 does **not** have a KNX module mounted, or the module is disconnected from the KNX bus. However, it is connected to Device 2 via the green networking terminal.
* Device 2 does have a KNX module mounted and connected, and is also connected to Device 1 via the green networking terminal.
* Device 3 only has a KNX module mounted and connected.

Both connected KNX modules are configured to have test alarm networking enabled:
* In ETS, set parameters on Testalarm page as follows
  - Send test-alarm status periodically: **No**
* In ETS, ensure `Testalarm Network` (communication object 1) is linked to the same group address
* Download the changes to the devices
* Start Group Monitor

# Test Cases

## C.1.a

**Step:** Trigger test alarm via button on Device 1

**Verify:** Device 2 sends messages on the bus to set both `Testalarm Network` (communication object 1) and `Test-alarm Status` (communication object 5) to value `1`

**Step:** Silence test alarm via button on Device 1

**Verify:** Device 2 sends messages on the bus to set both `Testalarm Network` (communication object 1) and `Test-alarm Status` (communication object 5) to value `0`

## C.1.b

**Step:**
* Trigger test alarm via button on Device 1
* Try to silence test alarm via button on Device 2

**Verify:** Nothing happens (it cannot silence the wired test alarm, and it does not have bus but local alarm, so it does not silence the bus alarm via `Alarm Reset`)

**Step:** Try to silence test alarm via button on Device 3

**Verify:**
* Device 3 is silenced
* Device 3 sends message on the bus to set `Alarm Reset` (communication object 2) to value `1`

**Step:** Silence test alarm via button on Device 1

**Verify:** Device 2 sends messages on the bus to set both `Testalarm Network` (communication object 1) and `Test-alarm Status` (communication object 5) to value `0`

**Step**: Wait for 1 minute before starting the next test
