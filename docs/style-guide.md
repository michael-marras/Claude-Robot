# Robot Code Style Guide

A lightweight set of conventions for keeping our code readable and safe across the two boards. The goal isn't rigid rules — it's that either of us can open the other's code and follow it quickly, and that the body's real-time motor/sensor code doesn't trip over the usual embedded footguns.

This applies to both boards, but the safety section matters most for the **body** (motors, encoders, IMU), since that's where interrupts and timing live.

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

Units in names when it prevents confusion: `delayMs`, `headingDeg`, `distanceCm`. Mixing up units is a classic robot bug — naming heads it off.

---

## Layout & readability

- One class per folder under `lib/` (PlatformIO requirement anyway), with matching `.h`/`.cpp`.
- Keep `main.cpp` thin — it initializes things and starts tasks. Real logic lives in classes.
- Group related members; put `public:` first in a class so the interface is what a reader sees first.
- A function that doesn't fit on one screen is usually doing too much — consider splitting.
- Use `const` wherever a value or method doesn't change state. It documents intent and lets the compiler catch mistakes.

### Comments

- Comment the *why*, not the *what*. `i++; // increment i` is noise; `// skip header byte, it's not part of the checksum` earns its place.
- A short comment above any non-obvious block (an ISR, a timing-sensitive section, a bit-twiddling line) saves the other person real time.
- Keep a one-line comment at the top of each file saying what it's for.

---

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
 
- **Never power servos off the ESP32.** Eight servos under load can pull several amps. They get their own 5–6V supply (a UBEC or dedicated servo rail); the ESP32 only sends I2C signals to the PCA9685. Share a common ground between the ESP32, the PCA9685, and the servo power supply, or the signals won't reference correctly.
- **Don't block the loop with `delay()` for servo motion.** A leg that needs to move over 300 ms shouldn't freeze the whole robot with `delay(300)` — nothing else (IMU, UART) runs during that pause. Track timing with `millis()` and step the servo target each loop instead.
- **Move servos gradually, not in jumps.** Commanding a servo straight from 0° to 180° makes it slam and spike current. Step toward the target a few degrees per loop for smoother motion and a gentler power draw — important when 8 are moving at once.
- **Clamp every angle to a safe range** before sending it. A leg servo driven past its mechanical limit will grind, stall, and overheat. Keep min/max constants per joint and never write outside them: `angle = constrain(angle, MIN_HIP, MAX_HIP);`
- **Fail safe.** If the link to the head drops or a bad command arrives, decide a safe default (e.g. hold the last valid pose or settle into a stable stance) rather than leaving servos in an undefined state.
> **A note on interrupts:** the original wheeled design used encoder interrupts (ISRs), which have strict rules. The quadruped doesn't use them for the legs, so that's no longer core. *If* you later add interrupt-driven sensors (foot-contact switches, limit switches), the rules are: keep the ISR tiny (read pin, set flag, return), mark shared variables `volatile`, mark the function `IRAM_ATTR`, and never call blocking functions like `Serial` or `delay()` inside it. Until then, don't worry about it.
 
---

## Git

- Small, focused commits with a present-tense summary line: "Add encoder ISR debouncing", not "stuff".
- Don't commit `.pio/` or `.vscode/` (they're in `.gitignore`).
- Pull before you push; keep `main` buildable.
- Don't make massive pull requests

---

*This is a living document — if a rule turns out to be annoying or unclear, raise it and we'll adjust. The point is shared readability, not bureaucracy.*