# Purpose
Positive and negative temperatures and temperature offsets work.

# Results

| Test case | Tester | Date | Commit | Result |
| --- | --- | --- | --- | :---: |
| B.4.a | @dallmair  | 2023-12-20 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|       | @Darthyson | 2024-11-20 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |
| B.4.b | @dallmair  | 2023-12-20 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|       | @Darthyson | 2024-11-20 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |
| B.4.c | @dallmair  | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|       | @Darthyson | 2024-11-20 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |
| B.4.d | @dallmair  | 2023-12-22 | a870a7c5b946b9443e7fba81ecee02e796dd77a9 | :ok: |
|       | @Darthyson | 2024-11-20 | bd820b81e9b31380050ef2a95776e714b3d819a8 | :ok: |

# Setup
Setup per finished [Tests B.1](tests_B_1.md), i.e. fully configured module mounted in smoke alarm device, and Group Monitor running.

# Test Cases

## B.4.a

**Steps:**
* In ETS, set parameters on Status Informations page as follows
  - Send status informations periodically: **Yes**
  - Interval [min]: **1**
  - Temperature: **Yes**
  - \<all other switches>: **No**
* In ETS, set parameters on Temperature page as follows
  - Temperature-offset in 0.1K: **-120**
* Download the changes to the device
* Have device indoors

**Verify:** Sends temperature in range 6 to 13 °C, depending on the room's temperature

## B.4.b

**Steps:**
* In ETS, set parameters on Temperature page as follows
  - Temperature-offset in 0.1K: **120**
* Download the changes to the device
* Have device indoors

**Verify:** Sends temperature in range 30 to 37 °C, depending on the room's temperature

## B.4.c

**Steps:**
* In ETS, set parameters on Temperature page as follows
  - Temperature-offset in 0.1K: **-120**
* Download the changes to the device
* **Put device into a freezer**

> [!WARNING]
> The Dira Dual smoke alarm devices are actually only allowed to be operated down to -5 °C. Use a dedicated test device for this test, i.e. do **not** use the device in production afterwards anymore.
>
> Also, be prepared that the smoke alarm can be triggered in this test due to condensate in the smoke box.

**Verify:** Sends temperature in range -32 to -26 °C, depending on the freezer's temperature

## B.4.d

**Steps:**
* In ETS, set parameters on Temperature page as follows
  - Temperature-offset in 0.1K: **120**
* Download the changes to the device
* Put device into a freezer

> [!WARNING]
> The Dira Dual smoke alarm devices are actually only allowed to be operated down to -5 °C. Use a dedicated test device for this test, i.e. do **not** use the device in production afterwards anymore.
>
> Also, be prepared that the smoke alarm can be triggered in this test due to condensate in the smoke box.

**Verify:** Sends temperature in range -8 to -2 °C, depending on the freezer's temperature
