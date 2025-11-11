# 🎃 Talking Skull – Halloween Automation (Bone Jovi AI)

### Home Assistant Automation for Spooky AI Interaction

---

## 🧠 Overview

This README documents the **Halloween AI automation** that brings **Bone Jovi**, the animatronic talking skull, to life.  
It uses **Home Assistant**, **ESPHome**, and **LLMVision** to detect motion, analyze images, generate spooky dialogue, and trigger synchronized speech with jaw movement and sound playback.

The automation is designed for the dual‑ESP32 Talking Skull setup:

- `skullaudio` → **Audio ESP32 (Master)** — handles I²S audio playback via MAX98357A amplifier.  
- `jawservo` → **Jaw ESP32 (Slave)** — receives shared I²S signal, analyzes amplitude, and moves the jaw servo.


Uses LLM Vision for image capture and description - https://llmvision.org/

---

## 🔁 System Flow

```
[Motion Sensors / Cameras]
          │
          ▼
     Home Assistant
          │
     ├── Triggers: Motion Detected
     ├── Condition: Skull not speaking
          ▼
   [LLMVision → Image Analyzer]
          │
   Describes scene (e.g., “a person in costume”) - IF NOTHINGFOUND it ends the automation
          ▼
 [Template Phrase Generator]
          │
   Builds Halloween‑themed response
          ▼
 [TTS → Audio ESP32]
          │
   Plays voice through MAX98357A
          ▼
 [Jaw ESP32]
   Reads I²S amplitude → Moves jaw
```

---

## 🧩 Entities and Helpers

| Entity | Type | Purpose |
|--------|------|----------|
| `input_boolean.bonejovi_streaming` | Boolean | Prevents overlapping speech |
| `media_player.skull_audio` | Media Player | Plays audio via Audio ESP32 |
| `binary_sensor.frontcam_amcrest_motion_detected` | Binary Sensor | Triggers the skull |
| `llmvision.image_analyzer` | Service | AI vision analysis |
| `script.skull_speak_phrase` | Script | Sends message to TTS service |

---

## ⚙️ Core Automation YAML

File: `talking-skull-halloween.yaml`

```yaml
alias: Talking Skull AI Halloween
description: ""
triggers:
  - trigger: state
    entity_id:
      - binary_sensor.frontcam_amcrest_motion_detected
      - binary_sensor.frontmotion_motion_detection
      - event.front_door_motion
    to: "on"

conditions:
  - condition: state
    entity_id: input_boolean.bonejovi_streaming
    state: "off"

actions:
  - alias: Analyze front camera for activity
    action: llmvision.image_analyzer
    data:
      include_filename: false
      target_width: 1280
      max_tokens: 500
      expose_images: true
      provider: YOUR AI PROVIDER LLM VISION
      remember: true
      message: >
        Look at the provided camera image and describe who or what you see that has triggered your curiosity.

  - alias: Generate spooky phrase
    variables:
      description: "{{ analysis.response_text | default('') | trim }}"
      phrase: >-
        {% set desc = description %}
        {% if desc | lower == 'nothingfound' or desc == '' %}
          NothingFound
        {% else %}
          {% set lines = [
            "Well, well, look who wandered into my graveyard, {description}.",
            "Mwahaha! The spirits whisper your name, {description}.",
            "Step closer, {description}… I promise I won’t bite. Much.",
            "Ah, {description}! You smell like fear and pumpkin spice.",
            "Beware, {description}, the veil is thin tonight!",
            "{description}, your soul looks delicious this evening!"
          ] %}
          {{ lines | random }}
        {% endif %}

  - alias: Set speaking flag
    action: input_boolean.turn_on
    target:
      entity_id: input_boolean.bonejovi_streaming

  - alias: Speak via TTS
    action: tts.google_translate_say
    target:
      entity_id: media_player.skull_audio
    data:
      message: "{{ phrase }}"

  - delay: 2s

  - alias: Reset speaking flag
    action: input_boolean.turn_off
    target:
      entity_id: input_boolean.bonejovi_streaming
```

---

## 🧙 Template Logic

The automation uses **Jinja2 templates** to turn AI‑generated descriptions into thematic dialogue.  
Example result from `llmvision`:

```
Input image: Person in a cloak with a pumpkin.
Output phrase: "Ah, a cloaked traveler bearing pumpkins! You must be late for the feast of the undead!"
```

You can customize the template lines to match different moods or personalities.

---

## 🔊 Integration with ESPHome Devices

- **Audio ESP32 (skullaudio):** Receives TTS message → plays through MAX98357A amplifier.  
- **Jaw ESP32 (jawservo):** Reads same I²S audio stream and moves the servo based on amplitude.  
- Both devices stay perfectly synchronized through the shared I²S wiring.

---

## 🧪 Testing and Debugging

1. **Test Audio Output:**  
   ```yaml
   service: tts.google_translate_say
   data:
     entity_id: media_player.skull_audio
     message: "Testing one, two, three."
   ```

2. **Verify Jaw Sync:** Jaw should move in rhythm with speech.  
3. **Monitor Logs:** Confirm `input_boolean.bonejovi_streaming` changes to `on` during playback.  
4. **Adjust Delays:** Increase the 2‑second reset delay if speech clips early.

---

## 💡 Expansion Ideas

- Add **WLED eye effects** that pulse with sound or react to motion.  
- Use different templates for “friendly” or “evil” Halloween personalities.  
- Schedule automatic activation at dusk using `sun` triggers.

---

## 📁 Suggested Folder Structure

```
talking-skull/
├── homeassistant/
│   ├── talking-skull-halloween.yaml
│   ├── scripts/
│   │   └── skull_speak_phrase.yaml
│   └── helpers.yaml
├── esphome/
│   ├── esphome-skullaudio.yaml
│   ├── esphome-jawservo.yaml
└── README-Halloween-Automation.md
```

---

## 🧾 Credits

- **Concept & Voice Design:** Paul Goldstein (“Bone Jovi”)  
- **AI Integration:** Home Assistant LLMVision + OpenAI  
- **Servo & Audio Firmware:** Dual‑ESP32 I²S Hardware System  
- **Inspiration:** Mike’s *JawDuino Project*

---

## 📜 License

This automation is provided under the **MIT License**.  
Use freely for personal and creative Halloween displays — attribution appreciated.
