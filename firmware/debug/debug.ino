#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ---- Sensor count ----
const int NUM_SENSORS = 4;

// ---- Color Definitions ----
const int COL_PINK   = 0;
const int COL_BLUE   = 1;
const int COL_GREEN  = 2;
const int COL_YELLOW = 3;
const int COL_NONE   = -1;

// ---- Calibration Constants (FIXED: Added missing variable) ----
const int CAL_SAMPLES = 15; 

// =====================================================
// CALIBRATION VALUES
// =====================================================
unsigned long redMin[NUM_SENSORS]   = {0};
unsigned long greenMin[NUM_SENSORS] = {0};
unsigned long blueMin[NUM_SENSORS]  = {0};
unsigned long redMax[NUM_SENSORS]   = {0};
unsigned long greenMax[NUM_SENSORS] = {0};
unsigned long blueMax[NUM_SENSORS]  = {0};

const float HIGH_FRACTION      = 0.75;
const float GAP_FRACTION       = 0.15;
const float NO_COLOUR_FRACTION = 0.30;

// ---- DFPlayer ----
SoftwareSerial mySerial(A0, A1);
DFRobotDFPlayerMini myDFPlayer;
const int busyPin   = A2;
const int DF_VOLUME = 20;

// ---- Pin Assignments ----
const int S2_PINS[NUM_SENSORS]  = {5, 7, 9, 11};
const int S3_PINS[NUM_SENSORS]  = {6, 8, 10, 12};
const int OUT_PINS[NUM_SENSORS] = {2, 3, 4, 13};

// ---- RGB Storage ----
int redVals[NUM_SENSORS]   = {0};
int greenVals[NUM_SENSORS] = {0};
int blueVals[NUM_SENSORS]  = {0};
int avgVals[NUM_SENSORS]   = {0};

const int ROLLING_SIZE = 5;
int rollingRed[NUM_SENSORS][ROLLING_SIZE]   = {0};
int rollingGreen[NUM_SENSORS][ROLLING_SIZE] = {0};
int rollingBlue[NUM_SENSORS][ROLLING_SIZE]  = {0};
int rollingIdx[NUM_SENSORS]                 = {0};

const int CONFIRM_COUNT = 3; 
int confirmColour[NUM_SENSORS]; 
int confirmStreak[NUM_SENSORS] = {0};         
bool inLockout[NUM_SENSORS] = {false};

int highThresh[NUM_SENSORS]     = {0};
int gapThresh[NUM_SENSORS]      = {0};
int noColourThresh[NUM_SENSORS] = {0};

const int trackTable[NUM_SENSORS][4] = {
  {18, 22, 26, 30}, {19, 23, 27, 31}, {20, 24, 28, 32}, {21, 25, 29, 33},
};

// ---- Audio Queue ----
const int QUEUE_SIZE = 8;
int audioQueue[QUEUE_SIZE] = {0};
int queueHead = 0;
int queueTail = 0;

unsigned long lastDebugPrint = 0; 

// ---- Prototypes ----
void calibrateSensors();
void computeThresholds();
void readColour(int sensorIdx);
int classifyColour(int i);
void enqueueTrack(int track);
void serviceQueue();
unsigned long robustRead(int i, bool s2, bool s3, int samples);
String getColorName(int colorIdx);

void setup() {
  Serial.begin(9600); 
  mySerial.begin(9600); 
  
  Serial.println(F("--- System Starting ---"));
  
  pinMode(busyPin, INPUT);
  if (!myDFPlayer.begin(mySerial)) {
    Serial.println(F("DFPlayer Error: Check wiring/SD card"));
  }
  myDFPlayer.volume(DF_VOLUME);

  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(S2_PINS[i], OUTPUT);
    pinMode(S3_PINS[i], OUTPUT);
    pinMode(OUT_PINS[i], INPUT);
    confirmColour[i] = COL_NONE;
  }

  calibrateSensors();
  computeThresholds();
  Serial.println(F("--- Setup Complete ---"));
}

void loop() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    readColour(i);
    int colourIdx = classifyColour(i);

    if (inLockout[i]) {
      if (avgVals[i] < noColourThresh[i]) {
        inLockout[i] = false;
      }
    } else if (colourIdx != COL_NONE) {
      if (colourIdx == confirmColour[i]) {
        confirmStreak[i]++;
      } else {
        confirmColour[i] = colourIdx;
        confirmStreak[i] = 1;
      }

      if (confirmStreak[i] >= CONFIRM_COUNT) {
        Serial.print(F(">>> SENSOR ")); Serial.print(i); 
        Serial.print(F(" DETECTED: ")); Serial.println(getColorName(colourIdx));
        enqueueTrack(trackTable[i][colourIdx]);
        inLockout[i] = true;
        confirmStreak[i] = 0;
      }
    } else {
      confirmStreak[i] = 0;
    }
  }

  if (millis() - lastDebugPrint > 500) {
    for (int i = 0; i < NUM_SENSORS; i++) {
      Serial.print(F("S")); Serial.print(i);
      Serial.print(F(" [R:")); Serial.print(redVals[i]);
      Serial.print(F(" G:")); Serial.print(greenVals[i]);
      Serial.print(F(" B:")); Serial.print(blueVals[i]);
      Serial.print(F(" Avg:")); Serial.print(avgVals[i]);
      Serial.print(F("] -> ")); Serial.println(getColorName(classifyColour(i)));
    }
    Serial.println(F("-----------------------"));
    lastDebugPrint = millis();
  }

  serviceQueue();
  delay(10);
}

// =====================================================
// CALIBRATION (FIXED BRACES)
// =====================================================
void calibrateSensors() {
  myDFPlayer.play(2); delay(750);
  myDFPlayer.play(2); delay(750);
  myDFPlayer.play(18); delay(5000);

  myDFPlayer.play(2); delay(750);
  myDFPlayer.play(2); delay(10000);

  myDFPlayer.play(2);
  for (int i = 0; i < NUM_SENSORS; i++) {
    redMin[i]   = robustRead(i, LOW,  LOW,  CAL_SAMPLES);
    greenMin[i] = robustRead(i, HIGH, HIGH, CAL_SAMPLES);
    blueMin[i]  = robustRead(i, LOW,  HIGH, CAL_SAMPLES);
  }
  delay(1500);

  myDFPlayer.play(5); delay(750);
  myDFPlayer.play(5); delay(10000);

  myDFPlayer.play(5);   
  for (int i = 0; i < NUM_SENSORS; i++) {
    redMax[i]   = robustRead(i, LOW,  LOW,  CAL_SAMPLES);
    greenMax[i] = robustRead(i, HIGH, HIGH, CAL_SAMPLES);
    blueMax[i]  = robustRead(i, LOW,  HIGH, CAL_SAMPLES);
  }
  delay(1500);

  myDFPlayer.play(7); delay(750);
  myDFPlayer.play(8); delay(750);
  myDFPlayer.play(32);

  // Print Calibration Report (NOW INSIDE THE FUNCTION)
  Serial.println(F("\n--- Calibration Results ---"));
  for (int i = 0; i < NUM_SENSORS; i++) {
    Serial.print(F("Sensor ")); Serial.println(i);
    Serial.print(F("  RED:   ")); Serial.print(redMin[i]); Serial.print(F(" to ")); Serial.println(redMax[i]);
    Serial.print(F("  GREEN: ")); Serial.print(greenMin[i]); Serial.print(F(" to ")); Serial.println(greenMax[i]);
    Serial.print(F("  BLUE:  ")); Serial.print(blueMin[i]); Serial.print(F(" to ")); Serial.println(blueMax[i]);
  }
  Serial.println(F("---------------------------\n"));
}

void computeThresholds() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    highThresh[i]     = (int)(255 * HIGH_FRACTION);
    gapThresh[i]      = (int)(255 * GAP_FRACTION);
    noColourThresh[i] = (int)(255 * NO_COLOUR_FRACTION);
  }
}

unsigned long robustRead(int i, bool s2, bool s3, int samples) {
  unsigned long sum = 0;
  int count = 0;
  digitalWrite(S2_PINS[i], s2);
  digitalWrite(S3_PINS[i], s3);
  delayMicroseconds(100);

  for (int n = 0; n < samples; n++) {
    unsigned long val = pulseIn(OUT_PINS[i], LOW, 10000);
    if (val > 0) {
      sum += val;
      count++;
    }
    delay(5);
  }
  return (count > 0) ? (sum / count) : 0;
}

void readColour(int i) {
  int r = constrain(map(robustRead(i, LOW,  LOW,  5), redMin[i], redMax[i], 255, 0), 0, 255);
  int g = constrain(map(robustRead(i, HIGH, HIGH, 5), greenMin[i], greenMax[i], 255, 0), 0, 255);
  int b = constrain(map(robustRead(i, LOW,  HIGH, 5), blueMin[i], blueMax[i], 255, 0), 0, 255);

  int idx = rollingIdx[i];
  rollingRed[i][idx] = r; rollingGreen[i][idx] = g; rollingBlue[i][idx] = b;
  rollingIdx[i] = (idx + 1) % ROLLING_SIZE;

  long sumR = 0, sumG = 0, sumB = 0;
  for (int n = 0; n < ROLLING_SIZE; n++) {
    sumR += rollingRed[i][n]; sumG += rollingGreen[i][n]; sumB += rollingBlue[i][n];
  }
  redVals[i] = sumR / ROLLING_SIZE;
  greenVals[i] = sumG / ROLLING_SIZE;
  blueVals[i] = sumB / ROLLING_SIZE;
  avgVals[i] = (redVals[i] + greenVals[i] + blueVals[i]) / 3;
}

int classifyColour(int i) {
  int r = redVals[i], g = greenVals[i], b = blueVals[i];
  if (avgVals[i] < noColourThresh[i]) return COL_NONE;
  
  if (b > r && b > g && (b - r) > gapThresh[i]) return COL_BLUE;
  if (g > r && g > b && r < highThresh[i])      return COL_GREEN;
  if (r > highThresh[i] && g > highThresh[i] && b < r) return COL_YELLOW;
  if (r > g && r > b)                           return COL_PINK;
  
  return COL_NONE;
}

String getColorName(int colorIdx) {
  switch (colorIdx) {
    case COL_PINK:   return "PINK";
    case COL_BLUE:   return "BLUE";
    case COL_GREEN:  return "GREEN";
    case COL_YELLOW: return "YELLOW";
    default:         return "NONE";
  }
}

void enqueueTrack(int track) {
  int nextTail = (queueTail + 1) % QUEUE_SIZE;
  if (nextTail != queueHead) {
    audioQueue[queueTail] = track;
    queueTail = nextTail;
  }
}

void serviceQueue() {
  if (queueHead != queueTail && digitalRead(busyPin) == HIGH) {
    myDFPlayer.play(audioQueue[queueHead]);
    queueHead = (queueHead + 1) % QUEUE_SIZE;
  }
}