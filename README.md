# Wearable Health Monitoring Device

A compact ESP32-C3-based wearable that continuously monitors **heart rate** and **body surface temperature** on the wrist/finger, shows live readings on a small OLED, and vibrates if the heart rate crosses safety thresholds. The board was taken from schematic to PCB in KiCad.

![Circuit diagram](https://github.com/ghulam420-sarwar/Wearable-Health-Monitor/blob/main/circuit_diagram.png)

## Highlights

- **Optical HR sensing** with MAX30102 using the SparkFun peak-detection algorithm
- **Non-contact body temperature** via MLX90614 IR thermometer
- **Local alert** with a vibration motor — no phone needed
- **KiCad schematic → PCB → assembled prototype** (all images are present in files`)
![Circuit diagram](https://github.com/ghulam420-sarwar/Wearable-Health-Monitor/blob/main/pcb_layout.png)
- **LiPo-powered** with TP4056 charging circuit

## Hardware

| Component       | Role                    | Interface |
| --------------- | ----------------------- | --------- |
| ESP32-C3 Mini   | MCU (small footprint)   | —         |
| MAX30102        | HR + SpO₂ (IR + red)    | I²C 0x57  |
| MLX90614        | IR body temperature     | I²C 0x5A  |
| SSD1306 OLED    | 128×64 display          | I²C 0x3C  |
| Coin vibration motor + MOSFET | Silent alert      | GPIO6 (PWM) |
| LiPo 3.7 V + TP4056 | Power + charging    | —         |

### Pinout

| ESP32-C3 | Net               |
| -------- | ----------------- |
| GPIO8    | I²C SDA           |
| GPIO9    | I²C SCL           |
| GPIO6    | Vibration motor   |
| GPIO5    | User button       |

## Algorithm

1. Continuously sample the IR channel of the MAX30102
2. Run each sample through `checkForBeat()` (zero-crossing on filtered signal)
3. On detected beat → compute instantaneous BPM = 60 000 / Δt
4. Apply a rolling average over 8 beats to smooth
5. Alert if avg BPM < 40 or > 160

## Build

```bash
pio run -t upload
pio device monitor
```

## Validation

| Test                         | Result                        |
| ---------------------------- | ----------------------------- |
| HR vs pulse oximeter (Beurer PO40) | ±2 BPM over 5-minute window |
| Body temp vs oral thermometer      | ±0.3 °C (skin vs oral offset) |
| Current draw (active)              | ~42 mA @ 3.7 V              |
| Battery life (500 mAh LiPo)        | ~11 hours continuous        |

## What I learned

- Designing a small 2-layer PCB with a dedicated analog ground for the optical sensor
- Interpreting and filtering a raw PPG (photoplethysmography) signal
- Driving an inductive load (vibration motor) via MOSFET with a flyback diode
- Picking sensor placement so ambient light does not swamp the IR signal

## License

MIT © Ghulam Sarwar
