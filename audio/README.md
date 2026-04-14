# SD Card Setup

To make the audio work with the DFPlayer Mini and the `duet.ino` code, follow these rules:

## 1. SD Card Formatting
*   The SD card must be formatted to **FAT32**.
*   It is recommended to use a card 32GB or smaller.

## 2. File Structure
The code uses the `play()` function, which usually looks for files in the **root** of the SD card or in a folder named **mp3**. 

**Current Mapping in `duet.ino`:**
*   Tracks 2, 3, 4: Startup & Calibration Cues
*   Tracks 5, 6, 7, 8: Calibration Prompts
*   Tracks 18 - 33: Musical notes for Sensor Gates 1-4

## 3. Filename Convention
Ensure your files are named with four-digit prefixes so the DFPlayer can index them correctly:
*   `0002.mp3`
*   `0018.mp3`
*   ...etc.

*Note: If you add new files, the physical order in which you copy them to the SD card can sometimes affect the playback order on the DFPlayer Mini. It is best to format the card and copy all files onto it in one go.*