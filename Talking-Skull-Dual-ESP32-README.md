# 🦴 Talking Skull – Dual ESP32 Hardware and Firmware Design

### Realistic Audio‑Reactive Skull Using Shared I²S Bus

---

## 🧠 Overview

This version of the **Talking Skull (Bone Jovi)** uses **two ESP32 boards** connected through a **shared I²S audio bus** to produce lifelike, synchronized jaw motion.  
The **Audio ESP32** (Master) generates digital audio and drives a **MAX98357A** amplifier for sound output.  
The same I²S signal is shared with the **Jaw ESP32** (Slave), which reads the audio stream directly through its I²S interface and moves the servo according to amplitude — perfectly synchronized with speech or sound.

This architecture eliminates latency, ensures realistic jaw motion, and requires no wireless synchronization or MQTT delay.

It creates a HomeAssistant media_player device for the audio that can be used to play other audio streams or mp3s.
I have created detailed automations that trigger based upon motion detection that can perform AI based image descriptions and merge this into phrases 
that can be "spoken" to the visitor.

---

## ⚙️ System Architecture

Two ESP32 devices work together:

| Device | Role | Description |
|---------|------|-------------|
| **ESP32 #1 (Audio)** | Audio/TTS playback | Plays TTS or MP3 files using a MAX98357A I²S amplifier. |
| **ESP32 #2 (Jaw Control)** | Servo motion | Moves the skull’s jaw in sync with sound amplitude captured from the audio line. |

Audio output and control signals are shared between them through a safe resistor divider.


```
+------------------------------------+
|        AUDIO ESP32 (Master)        |
|------------------------------------|
| GPIO26 ──220Ω───┬─► MAX98357A BCLK |
|                 └─► Jaw ESP32 GPIO26|
| GPIO27 ──220Ω───┬─► MAX98357A LRC  |
|                 └─► Jaw ESP32 GPIO27|
| GPIO33 ──220Ω───┬─► MAX98357A DIN  |
|                 └─► Jaw ESP32 GPIO33|
| GND ─────────────┴──────── GND      |
+------------------------------------+
                   │
                   ▼
+------------------------------------+
|          JAW ESP32 (Slave)         |
|------------------------------------|
| GPIO26  (BCLK IN) ◄────────────────┘
| GPIO27  (LRCLK IN)◄────────────────┘
| GPIO33  (DATA IN) ◄────────────────┘
| GPIO23 → Servo Signal               |
| 5 V / GND → Servo Power             |
+------------------------------------+
```

---

## 🧩 Bill of Materials (BOM)

| Component | Quantity | Notes |
|------------|-----------|-------|
| ESP32 Dev Board | 2 | One for audio, one for jaw |
| MAX98357A I²S Amplifier | 1 | 3.3 V compatible audio output |
| MG996R or SG90 Servo | 1 | Controls jaw |
| 220 Ω resistors | 3 | Inline between I²S shared lines |
| 5 V Power Supply | 1 | Powers both ESP32s and servo |
| Common GND wire | — | Must connect all GNDs together |
| Skull prop + jaw linkage | 1 | Hinged or spring‑loaded jaw |

---

## 🧮 Pin Mapping

### **Audio ESP32 (Master)**

| Function | GPIO | Connected To |
|-----------|------|--------------|
| I²S BCLK | GPIO26 | MAX98357A BCLK, Jaw ESP32 GPIO26 |
| I²S LRCLK | GPIO27 | MAX98357A LRC, Jaw ESP32 GPIO27 |
| I²S DIN | GPIO33 | MAX98357A DIN, Jaw ESP32 GPIO33 |
| GND | — | Common ground |
| 5 V | VIN | Powers amplifier |

### **Jaw ESP32 (Slave)**

| Function | GPIO | Connected To |
|-----------|------|--------------|
| I²S BCLK IN | GPIO26 | From Audio ESP32 GPIO26 |
| I²S LRCLK IN | GPIO27 | From Audio ESP32 GPIO27 |
| I²S DATA IN | GPIO33 | From Audio ESP32 GPIO33 |
| Servo Signal | GPIO23 | Jaw servo control |
| 5 V / GND | — | Servo power and shared ground |

---

## 🔍 Theory of Operation

- The **Audio ESP32** acts as an I²S **master**, generating a continuous 16‑bit mono or stereo audio stream.  
- The **MAX98357A** amplifier converts this digital stream into analog sound for the speaker.  
- The **Jaw ESP32**, wired in parallel to the same I²S lines through 220 Ω protection resistors, acts as an **I²S slave receiver**.  
- It samples the same digital stream and computes the instantaneous **audio amplitude**, mapping it to a **servo angle**.  
- Because both boards share the same timing signals (BCLK, LRCLK), the servo motion matches the spoken audio *with no perceptible lag*.

---

## 🧠 Firmware Design

### 1️⃣ `esphome-skullaudio.yaml` – Audio ESP32 (Master)

Handles I²S output, amplifier control, and Home Assistant communication.

**Key YAML Snippet:**
```yaml
i2s_audio:
  - id: i2s_out
    i2s_lrclk_pin: 27
    i2s_bclk_pin: 26
    i2s_dout_pin: 33

media_player:
  - platform: i2s_audio
    name: "Skull Audio"
    dac_type: external
    i2s_audio_id: i2s_out
```

This board can also handle TTS or stream audio from Home Assistant (`media_player.skull_audio`).

---

### 2️⃣ `esphome-jawservo.yaml` – Jaw ESP32 (Slave)

Controls the jaw servo based on the incoming audio amplitude.

**Key Features:**
- Reads analog input (ADC) corresponding to audio amplitude
- Smoothly moves the jaw servo proportionally to loudness
- Allows testing and calibration modes through switches
- Integrates with Home Assistant for manual control and diagnostics

**Key YAML Snippet:**
```yaml
i2s_audio:
  - id: i2s_in
    i2s_lrclk_pin: 27
    i2s_bclk_pin: 26
    i2s_din_pin: 33
    mode: rx
    bits_per_sample: 16bit
    sample_rate: 44100

sensor:
  - platform: i2s_audio_amplitude
    id: jaw_amp
    i2s_audio_id: i2s_in
    update_interval: 10ms
    on_value:
      then:
        - servo.write:
            id: jaw_servo
            level: !lambda 'return x * 0.9;'

servo:
  - id: jaw_servo
    output: jaw_output

output:
  - platform: ledc
    id: jaw_output
    pin: 23
```

The `i2s_audio_amplitude` sensor continuously analyzes incoming digital samples and maps the loudness to the servo’s angle.


### 2️⃣ `esphome-jawservo.yaml` – Jaw ESP32 (Slave)

---

## ⚙️ Setup Procedure

1. **Flash both ESP32 boards** via ESPHome (or esphome‑web).  
   - `esphome-skullaudio.yaml` → Audio Master  
   - `esphome-jawservo.yaml` → Jaw Slave  

2. **Wire according to the schematic.**  
   - Keep I²S wires **under 6 inches** to avoid noise.  
   - Use **220 Ω resistors** in series on BCLK, LRCLK, and DIN.  
   - Tie grounds together.

3. **Power both from a regulated 5 V source** (≥2 A if using a large servo).

4. **Servo Calibration:**  
   Adjust `level:` multiplier or servo min/max to match your skull’s jaw range (typically 15°–25°).

5. **Audio Test:**  
   Play any media via Home Assistant or ESPHome’s `media_player` service — the jaw should move in sync.

---

## 🧰 Custom Component: `SoftRcPulseOut`

Originally written by **RC Navy**, this library provides **software-based servo pulse control** with reduced jitter — an alternative to the standard `Servo` library.  
While the ESPHome version uses hardware PWM, the original Arduino version used this for smooth analog servo movement.

**Features:**
- Microsecond precision for servo pulse width  
- Handles multiple software-generated PWM outputs  
- Compatible with low-resource microcontrollers (e.g., ATtiny85)


## 🔋 Powering Everything

Use a **regulated 5V power supply** rated for at least **2A**:
- Servos can draw large transient currents.
- Both ESP32s and MAX98357A share the 5V rail.
- Use separate ground wires for audio and servo power to reduce noise.

If using USB:
- Power Audio ESP32 via USB.
- Power Jaw ESP32 via VIN with 5V directly.
- Ensure **common ground** across both boards.

## 🧩 Integrations

| Integration | Purpose |
|--------------|----------|
| **Home Assistant** | Send TTS or MP3 files for playback |
| **ESPHome API** | Wireless control and synchronization |
| **Node-RED (optional)** | Create reactive flows for speech and motion |
| **Alexa Media Player** | Play TTS through the skull |

---

## ⚠️ Troubleshooting

| Issue | Cause | Solution |
|-------|--------|----------|
| Scratchy audio | Shared ground loop or missing resistor divider | Add isolation resistors; verify common ground |
| Jaw buzzes at rest | Servo holding torque too high | Add dead-band or relax servo after idle |
| No jaw motion | Wrong ADC pin or scaling | Check analog wiring and calibration |
| Servo jitter | Power noise from audio amp | Add decoupling capacitors and separate grounds |

## 🧩 Optional Enhancements

- Working on incorporating the Adafruit uncanny eyes project - https://github.com/adafruit/Uncanny_Eyes
- Use **WLED** or an additional ESP32 for matrix eyes.  
- Connect both boards to **Home Assistant** for remote control, TTS playback, and scene automations.  
- Integrate seasonal personality packs (Halloween, Christmas, etc.).

---

## 🧱 Folder Structure

```
talking-skull/
├── esphome/
│   ├── esphome-skullaudio.yaml
│   ├── esphome-jawservo.yaml
├── docs/
│   ├── wiring-diagram.txt
│   ├── jawduino.pdf
├── firmware/
│   ├── SoftRcPulseOut.h
│   ├── SoftRcPulseOut.cpp
└── README.md
```

---

## 🧾 Credits

- **Hardware Concept:** Paul Goldstein (“Bone Jovi”)  
- **Original Servo Logic:** Mike’s *JawDuino Project*  
- **SoftRcPulseOut Library:** RC Navy  
- **Firmware & Integration:** ESPHome / Home Assistant community  

---

## 📜 License

Released under the **MIT License**.  
Free for personal and educational animatronics use — attribution appreciated.
