# Duet: Shape of Sound

**Duet** is an interactive, 2-player marble instrument designed for musical conversation. As players roll colored marbles down mirrored tracks, sensors trigger unique melodic responses. This shared dialogue explores the intersection of physical movement and sound, allowing two people to "speak" to one another through a generative soundscape.

🔗 **[Project Website & Full Documentation](https://zhengboon.github.io/Shape-Of-Sound/)**

---

## 📂 Repository Structure

*   **/audio**: The sound library (WAV/MP3) and the SD card configuration guide.
*   **/docs**: Comprehensive [User Guide](./docs/User_Guide.md), [Developer Guide](./docs/Developer_Guide.md), and [Bill of Materials](./docs/BOM.md).
*   **/firmware**: 
    *   `duet/`: The main production software.
    *   `calibration/`: Interactive tool for hard-coding sensor values.
    *   `debug/`: Real-time serial monitoring of sensor data.
*   **/hardware**: 
    *   `electronics/`: PCB designs and wiring schematics (PDF).
    *   `sensor cover CAD/`: 3D models in SolidWorks (.SLDPRT), .STEP, and .STL formats.
*   **LICENSE**: MIT License.

---

## 🚀 Quick Start & Audio Calibration

1.  **Hardware:** Connect the Arduino, 4x TCS3200 sensors, and the DFPlayer Mini according to the schematics in `/hardware/electronics`.
2.  **Audio:** Load the files in `/audio` onto a FAT32 formatted SD card.
3.  **Upload:** Flash `duet.ino` to the Arduino.
4.  **Calibration:** Upon power-up, the system enters an **Audio-Guided Calibration** phase:
    *   **3-Note Melody:** System Start.
    *   **2-Note Tone:** Place white marbles at all sensor gates now.
    *   **1-Note Tone:** System is recording White levels.
    *   **2-Note Tone:** Remove marbles (or replace with black samples).
    *   **1-Note Tone:** System is recording Black levels.
    *   **3-Note Melody:** Calibration Complete. Start your duet!

---

## 🛠 Technical Features

*   **Conversational Logic:** A split-track architecture designed for non-verbal dialogue between two participants.
*   **Robust Signal Processing:** Implements rolling averages and consecutive confirmation logic to accurately detect marbles at high velocities.
*   **Asynchronous Audio Queue:** A circular buffer manages audio triggers so that simultaneous "replies" from both players are queued and played without overlapping or loss.
*   **Dynamic Lockout:** A software-defined lockout prevents single marbles from double-triggering notes, ensuring a clean musical experience.

---

## 🤝 Handoff Note
This repository serves as a complete archive for the Duet project. It includes all firmware, mechanical designs, and assets required to replicate or iterate on the instrument. Future developers should consult the **Developer Guide** in the `/docs` folder for details on the signal processing pipeline and calibration tuning.

---
**Developed by:** [Zheng Boon](https://zhengboon.github.io/Shape-Of-Sound/)  
**License:** MIT