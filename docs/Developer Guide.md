# Developer's Guide: Duet Logic & Tools

This guide explains the software architecture and the specialized tools provided for maintaining and improving the Duet system.

## 🛠 Developer Toolset
While the main `duet.ino` file is the production software, this repository includes two specialized scripts for development:

1.  **`calibration.ino`**: Use this to generate hard-coded calibration values. If the instrument will be in a permanent installation with fixed lighting, run this script, copy the Serial output, and paste it into the `CALIBRATION VALUES` fallback defaults in `duet.ino`.
2.  **`debug.ino`**: Use this to troubleshoot sensor hardware. It prints raw pulse widths, mapped RGB values, and the current classification to the Serial Monitor (9600 baud) every 500ms.

## 📡 Signal Processing Pipeline
The system uses a three-stage pipeline to handle fast-moving marbles:
*   **Robust Sampling (`robustRead`):** Takes 5 samples and discards timeouts (`0` values) to filter electrical noise.
*   **Rolling Averages:** Smooths raw data through a 5-sample circular buffer to account for the marble's curved, reflective surface.
*   **Consecutive Confirmation:** A color must be detected `3` times in a row before a note is triggered to prevent "ghost" sounds.

## 🎹 Musical Dialogue Logic
### Audio Queueing
Because two players may trigger notes simultaneously, the system uses an **Asynchronous Audio Queue**. 
*   Detected notes are added to an 8-slot buffer.
*   `serviceQueue()` monitors the DFPlayer’s `busyPin` (A2).
*   Tracks are played back-to-back as the player becomes free, ensuring no "responses" in the conversation are lost.

### Dynamic Lockout
To prevent a single marble from "double-triggering" a note, the sensor enters a lockout state immediately upon detection. It only resets once `avgVals` drops below the `noColourThresh`, signaling that the marble has physically exited the sensor gate.

## ⚙️ Key Constants
*   `HIGH_FRACTION (0.75)`: Brightness threshold for color detection.
*   `GAP_FRACTION (0.15)`: Necessary separation between color channels for classification.
*   `SENSOR_SETTLE_US (200)`: Delay to stabilize sensors after switching shared pins.

---
**Original Developer:** Zheng Boon  
**Project:** Duet (Shape of Sound)