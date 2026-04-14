# Duet: Shape of Sound

**Duet** is an interactive, 2-player marble instrument designed for musical conversation. By rolling colored marbles down a split-track system, players trigger melodic responses to "speak" to one another. Using TCS3200 color sensors and Arduino, the project explores the intersection of physical movement and shared non-verbal dialogue.

🔗 **[Project Website & Documentation](https://zhengboon.github.io/Shape-Of-Sound/)**

---

## 📂 Repository Structure
*   **/firmware**: Contains the production code (`duet.ino`), the interactive calibration tool (`calibration.ino`), and the hardware troubleshooting script (`debug.ino`).
*   **/hardware**: CAD files for the track assembly (SolidWorks `.SLDPRT` and universal `.STEP` formats).
*   **/docs**: Detailed [User Guide](./docs/User_Guide.md) and [Developer Guide](./docs/Developer_Guide.md).

## 🚀 Quick Start
1.  **Hardware:** Connect the Arduino, 4x TCS3200 sensors, and the DFPlayer Mini according to the schematics in the `/hardware` folder.
2.  **Upload:** Flash `duet.ino` to the Arduino.
3.  **Calibrate:** Upon power-up, the system enters an **Audio-Guided Calibration** phase. 
    *   Wait for the **2-note tone** to place your white marbles.
    *   Wait for the next **2-note tone** to remove them.
    *   The **3-note melody** signals the instrument is ready for play.

## 🛠 Features
*   **Conversational Split-Track:** Designed for two players to face each other and respond to melodies in real-time.
*   **Robust Signal Processing:** Uses rolling averages and consecutive confirmation logic to ensure fast-moving marbles are detected accurately.
*   **Asynchronous Audio Queue:** A circular buffer manages audio triggers so that multiple simultaneous "notes" from both players are never lost.
*   **Dynamic Lockout:** Prevents single marbles from double-triggering, ensuring a clean musical experience.

## 🤝 Handoff Note
This repository was created to archive the Duet project. It includes all necessary firmware and mechanical files to replicate or improve the instrument. For future developers, please refer to the **Developer Guide** in the `/docs` folder for deep-dives into the signal processing logic.

---
**Developed by:** [Zheng Boon](https://zhengboon.github.io/Shape-Of-Sound/)  
**License:** MIT