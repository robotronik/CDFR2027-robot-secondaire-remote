# CDFR2027 robot secondaire — remote (ESP32-S3)

Phone remote for the cdfr2023 base roulante: an ESP32-S3 serves a web page over Wi-Fi and drives the STM32 control board over I2C. Step buttons: ▲▼ 100 mm, ◀▶ 15°, STOP.

## Wiring

ESP32-S3 GPIO8 (SDA) → STM32 PB7, GPIO9 (SCL) → PB6, GND ↔ GND.

The STM32 pins have no pull-ups; the old Pi supplied them. If the control board has none, add 4.7 kΩ from SDA and SCL to 3.3 V (the ESP internal pull-ups are too weak to rely on).

## Build & flash

Clone `librairie-commune` next to this repo, at the pinned commit:

```
git clone https://github.com/robotronik/librairie-commune ../librairie-commune && git -C ../librairie-commune checkout 11d32bb
```

The STM32 (`cdfr2023-programme-base-roulante-asservissment`) must be flashed against the same commit, otherwise a version-mismatch warning is logged.

Activate ESP-IDF v6.0.2 first, e.g. `. ~/esp/esp-idf/export.sh` (path depends on your install).

```
idf.py set-target esp32s3 && idf.py -p /dev/ttyACM0 flash monitor
```

Tested with ESP-IDF v6.0.2.

## Use

Join Wi-Fi `cdfr2023-teleop` (password `robotronik2023`), open http://192.168.4.1/. Each tap queues a step relative to the last target.

Safety: if the page goes silent for 300 ms (Wi-Fi lost, phone locked), the robot stops. STOP and the safety stop re-sync the next step from the robot's real position. One phone at a time.

## Host test

```
g++ -std=c++17 -Wall -Wextra -Imain test/test_teleop.cpp -o /tmp/t && /tmp/t
```

## Bench checklist (wheels off the ground)

- [ ] Phone joins `cdfr2023-teleop`, `http://192.168.4.1/` loads and shows "Connected".
- [ ] Each arrow turns the wheels the right way.
- [ ] Five fast ▲ taps move 500 mm in total.
- [ ] ▲ ×5 then STOP mid-run: wheels stop, the next ▲ moves 100 mm from that point.
- [ ] ▲ ×5 then turn off the phone's Wi-Fi: wheels stop within about 0.3 s.
- [ ] A second phone can't join the network.
- [ ] With the STM32 off or unplugged, the ESP logs I2C errors and keeps running.
