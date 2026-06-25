# Robot Code Style Guide

A lightweight set of conventions for keeping our code readable and safe across the two boards.

---

## Naming

Consistent naming is the cheapest way to make code readable. We use:

- **Types / classes** — `PascalCase`: `MotorController`, `Odometry`
- **Functions / methods** — `camelCase`: `readEncoder()`, `setServoAngle()`
- **Variables** — `camelCase`: `leftSpeed`, `headingDeg`
- **Member variables** — trailing underscore: `pinLeft_`, `lastCount_` (makes "is this a field or a local?" obvious at a glance)
- **Constants / macros** — `UPPER_SNAKE_CASE`: `MAX_SPEED`, `ENCODER_PIN_A`
- **Files** — match the class: `MotorController.h` / `MotorController.cpp`

Prefer names that say what something *is* or *does* over short cryptic ones. `encoderTicksLeft` beats `etl`. The few extra characters pay for themselves the first time someone else reads it.

Units in names when it prevents confusion: `delayMs`, `headingDeg`, `distanceCm`.

---

## Layout & readability

- One class per folder under `lib/` (PlatformIO requirement anyway), with matching `.h`/`.cpp`.
- Keep `main.cpp` thin — it initializes things and starts tasks. Real logic lives in classes.
- Group related members; put `public:` first in a class so the interface is what a reader sees first.
- A function that doesn't fit on one screen is usually doing too much — consider splitting.
- Use `const` wherever a value or method doesn't change state. It documents intent and lets the compiler catch mistakes.

### Comments

- A short comment above any non-obvious block (an ISR, a timing-sensitive section, a bit-twiddling line) saves the other person real time.

---

## Documentation

We document every function and method public *and* private with
Doxygen comments. Being able to read a function's contract without tracing its body saves
real time. Doxygen describes *what a function does and expects*;
inline comments explain *why* a tricky line exists.

- Every file opens with a `@file` header and one-line `@brief`
  (this replaces the "one-line comment at the top" note above).
- Every function/method gets a `@brief`. Document params with `@param`
  and returns with `@return`.
- Flag hardware side effects or timing constraints with `@note` /
  `@warning` — e.g. "blocks for one I2C transaction."
- Trivial helpers (a one-line getter, a clamp) can take a bare `@brief`
  — don't pad obvious code with boilerplate `@param`/`@return`.

### Example

​```cpp
/**
 * @brief Moves a single leg servo to a target angle.
 *
 * @param legId   Index of the leg (0–3).
 * @param jointId Joint within the leg (0 = hip, 1 = knee).
 * @param angle   Target angle in degrees, clamped to the joint's range.
 * @return true if accepted, false if legId/jointId out of range.
 */
bool setLegAngle(uint8_t legId, uint8_t jointId, uint8_t angle);
​```

Enforced in the `Doxyfile`: `EXTRACT_PRIVATE = YES`, `EXTRACT_STATIC = YES`,
`WARN_IF_UNDOCUMENTED = YES` (keep `EXTRACT_ALL = NO`, or the undocumented
warnings get suppressed).

## Safety — memory

Embedded RAM is tiny and there's no OS to bail us out, so a few habits matter:

- **Avoid dynamic allocation in the main loop.** `new`/`malloc`/`String` concatenation in a loop fragments the heap and eventually crashes. Allocate once at startup, or use fixed-size buffers.
- **Prefer fixed-size types** from `<stdint.h>` — `int16_t`, `uint8_t` — over bare `int` when the size matters (it always does in the shared protocol structs).
- **Avoid `String`** on the hot path. Use `char[]` buffers or fixed structs. `String` is convenient but heap-backed and a common source of slow fragmentation.
- **Size buffers explicitly** and never write past them. If a serial read can exceed the buffer, guard it.
- **Initialize variables** when you declare them. Uninitialized memory on embedded reads as garbage, not zero.

---

## Safety — servos
 
The body is a quadruped: 8 servos driven through a PCA9685 PWM board over I2C. That setup has its own footguns, mostly around power and not blocking the control loop:
 
- **Don't block the loop with `delay()` for servo motion.** A leg that needs to move over 300 ms shouldn't freeze the whole robot with `delay(300)` — nothing else (IMU, UART) runs during that pause. Track timing with `millis()` and step the servo target each loop instead.
- **Move servos gradually, not in jumps.** Commanding a servo straight from 0° to 180° makes it slam and spike current. Step toward the target a few degrees per loop for smoother motion and a gentler power draw — important when 8 are moving at once.
- **Clamp every angle to a safe range** before sending it. A leg servo driven past its mechanical limit will grind, stall, and overheat. Keep min/max constants per joint and never write outside them: `angle = constrain(angle, MIN_HIP, MAX_HIP);`
- **Fail safe.** If the link to the head drops or a bad command arrives, decide a safe default (e.g. hold the last valid pose or settle into a stable stance) rather than leaving servos in an undefined state.

---

## Testing

- Tests live under `test/`, named `test_<unit>` (PlatformIO convention).
- Pure logic — gait math, `clampServoAngle`, protocol struct parsing —
  should be unit-tested and run on the `native` environment, no board needed.
- Hardware-coupled code (PCA9685, encoders, IMU) is hard to test directly.
  Keep the logic separate from the I/O call so the logic half stays testable.
- Add or update a test when you change a unit's behavior; keep `main` green.

---

## Git

- Small, focused commits with a present-tense summary line: "Add encoder ISR debouncing", not "stuff".
- Don't commit `.pio/` or `.vscode/` (they're in `.gitignore`).
- Pull before you push; keep `main` buildable.
- Don't make massive pull requests

---

*This is a living document — if a rule turns out to be annoying or unclear, raise it and we'll adjust. The point is shared readability, not bureaucracy.*