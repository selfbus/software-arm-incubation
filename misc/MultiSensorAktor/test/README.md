# Unit tests of the MultiSensorAktor

Host build (x64, Linux) against the sblib repository: its `test/sblib` emulates the registers of the LPC11xx as
plain memory (`cpu-emu`), its `Catch` is the test framework. Nothing of the application's `setup()`/`loop()` runs;
the tests use single classes of `../src`.

```sh
cmake -S . -B cmake-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DSB_PATH_TO_SBLIB_REPO=<sblib repository>
cmake --build cmake-build
./cmake-build/MultiSensorAktor-test_x64            # all
./cmake-build/MultiSensorAktor-test_x64 "[PWMPin]" # by tag
```

`SB_PATH_TO_SBLIB_REPO` defaults to the submodule `software-arm-lib` of this repository.

| File | Tests |
| --- | --- |
| `src/test_pwm_pin.cpp` | `[PWMPin]` value selection of a PWM channel (object value, lock, fixed values) |
| `src/test_pwm_controller.cpp` | `[PwmController]` hardware channels of the pins, timer registers, software schedule and its interrupt, all 16 IOs and all 36 pins at once; `[PWMPin]` invert as ETS writes it |

In the emulation a timer does not count and writing its interrupt register does not clear flags; the tests set the
counter and the flags themselves and call the interrupt handler directly.
