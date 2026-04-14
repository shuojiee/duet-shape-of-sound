#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ---- Sensor count ----
const int NUM_SENSORS = 4;

// ---- Color Definitions (MOVED TO TOP) ----
const int COL_PINK   = 0;
const int COL_BLUE   = 1;
const int COL_GREEN  = 2;
const int COL_YELLOW = 3;
const int COL_NONE   = -1;

// =====================================================
// CALIBRATION VALUES — fallback defaults.
// =====================================================
unsigned long redMin[NUM_SENSORS]   = {0, 0, 502, 754};
unsigned long greenMin[NUM_SENSORS] = {0, 0, 1297, 2131};
unsigned long blueMin[NUM_SENSORS]  = {0, 0, 2160, 1132};

unsigned long redMax[NUM_SENSORS]   = {0, 0, 558, 1443};
unsigned long greenMax[NUM_SENSORS] = {0, 0, 1076, 1264};
unsigned long blueMax[NUM_SENSORS]  = {0, 0, 2182, 1459};

// =====================================================
// CLASSIFICATION THRESHOLDS
// =====================================================
const float HIGH_FRACTION      = 0.75;
const float GAP_FRACTION       = 0.15;
const float NO_COLOUR_FRACTION = 0.30;

// ---- DFPlayer ----
SoftwareSerial mySerial(A0, A1);
DFRobotDFPlayerMini myDFPlayer;
const int busyPin   = A2;
const int DF_VOLUME = 25;

// ---- Pin Assignments ----
const int S2_PINS[NUM_SENSORS]  = {5, 7, 9, 11};
const int S3_PINS[NUM_SENSORS]  = {6, 8, 10, 12};
const int OUT_PINS[NUM_SENSORS] = {2, 3, 4, 13};

// ---- RGB Storage ----
int redVals[NUM_SENSORS]   = {0};
int greenVals[NUM_SENSORS] = {0};
int blueVals[NUM_SENSORS]  = {0};
int avgVals[NUM_SENSORS]   = {0};

// ---- Rolling Average Buffers ----
const int ROLLING_SIZE = 5;
int rollingRed[NUM_SENSORS][ROLLING_SIZE]   = {0};
int rollingGreen[NUM_SENSORS][ROLLING_SIZE] = {0};
int rollingBlue[NUM_SENSORS][ROLLING_SIZE]  = {0};
int rollingIdx[NUM_SENSORS]                 = {0};

// ---- Consecutive Confirmation ----
const int CONFIRM_COUNT = 3; 
int confirmColour[NUM_SENSORS]; // Initialized in setup()
int confirmStreak[NUM_SENSORS] = {0};         

// ---- Classification Threshold Storage ----
int highThresh[NUM_SENSORS]     = {0};
int gapThresh[NUM_SENSORS]      = {0};
int noColourThresh[NUM_SENSORS] = {0};

// ---- Track Lookup Table ----
const int trackTable[NUM_SENSORS][4] = {
  {18, 22, 26, 30},
  {19, 23, 27, 31},
  {20, 24, 28, 32},
  {21, 25, 29, 33},
};

// ---- Dynamic Lockout ----
bool inLockout[NUM_SENSORS] = {false};

// ---- Audio Queue ----
const int QUEUE_SIZE = 8;
int audioQueue[QUEUE_SIZE] = {0};
int queueHead = 0;
int queueTail = 0;

// ---- Pulse Timeout ----
const unsigned long PULSE_TIMEOUT = 5000;
const int SENSOR_SETTLE_US = 200;
const int CAL_SAMPLES  = 15; 
const int LIVE_SAMPLES = 5;  

// ---- Forward Declarations ----
void calibrateSensors();
void computeThresholds();
void readColour(int sensorIdx);
int classifyColour(int i);
void enqueueTrack(int track);
void serviceQueue();
unsigned long readPulseWidth(int sensorIdx, bool s2Val, bool s3Val);
unsigned long stableRead(int i, bool s2, bool s3, int samples);
unsigned long robustRead(int i, bool s2, bool s3, int samples);

void setup() {
  mySerial.begin(9600);
  pinMode(busyPin, INPUT);
  
  if (!myDFPlayer.begin(mySerial)) {
    while(true); // Halt if DFPlayer not found
  }
  
  myDFPlayer.volume(DF_VOLUME);

  // Properly initialize arrays
  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(S2_PINS[i], OUTPUT);
    pinMode(S3_PINS[i], OUTPUT);
    pinMode(OUT_PINS[i], INPUT);
    confirmColour[i] = COL_NONE;
    confirmStreak[i] = 0;
    inLockout[i] = false;
  }

  calibrateSensors();
  computeThresholds();
}

void loop() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    delayMicroseconds(SENSOR_SETTLE_US);
    readColour(i);
  }

  for (int i = 0; i < NUM_SENSORS; i++) {
    int colourIdx = classifyColour(i);

    if (inLockout[i]) {
      if (avgVals[i] < noColourThresh[i]) {
        inLockout[i] = false;
        confirmStreak[i]  = 0;
        confirmColour[i]  = COL_NONE;
      }
      continue; 
    }

    if (colourIdx == COL_NONE) {
      confirmStreak[i] = 0;
      confirmColour[i] = COL_NONE;
      continue;
    }

    if (colourIdx == confirmColour[i]) {
      confirmStreak[i]++;
    } else {
      confirmColour[i] = colourIdx;
      confirmStreak[i] = 1;
    }

    if (confirmStreak[i] >= CONFIRM_COUNT) {
      enqueueTrack(trackTable[i][colourIdx]);
      inLockout[i]     = true;
      confirmStreak[i] = 0;
      confirmColour[i] = COL_NONE;
    }
  }

  serviceQueue(); 
  delay(10);
}

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
}

void computeThresholds() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    highThresh[i]     = (int)(255 * HIGH_FRACTION);
    gapThresh[i]      = (int)(255 * GAP_FRACTION);
    noColourThresh[i] = (int)(255 * NO_COLOUR_FRACTION);
  }
}

unsigned long readPulseWidth(int sensorIdx, bool s2Val, bool s3Val) {
  digitalWrite(S2_PINS[sensorIdx], s2Val);
  digitalWrite(S3_PINS[sensorIdx], s3Val);
  delayMicroseconds(50);
  return pulseIn(OUT_PINS[sensorIdx], LOW, PULSE_TIMEOUT);
}

unsigned long robustRead(int i, bool s2, bool s3, int samples) {
  unsigned long sum = 0;
  int validSamples = 0;
  for (int n = 0; n < samples; n++) {
    unsigned long val = readPulseWidth(i, s2, s3);
    if(val > 0) {
        sum += val;
        validSamples++;
    }
    delay(2);
  }
  return (validSamples > 0) ? (sum / validSamples) : 0;
}

void readColour(int sensorIdx) {
  int r = constrain(map(robustRead(sensorIdx, LOW,  LOW,  LIVE_SAMPLES), redMin[sensorIdx],   redMax[sensorIdx],   255, 0), 0, 255);
  int g = constrain(map(robustRead(sensorIdx, HIGH, HIGH, LIVE_SAMPLES), greenMin[sensorIdx], greenMax[sensorIdx], 255, 0), 0, 255);
  int b = constrain(map(robustRead(sensorIdx, LOW,  HIGH, LIVE_SAMPLES), blueMin[sensorIdx],  blueMax[sensorIdx],  255, 0), 0, 255);

  int idx = rollingIdx[sensorIdx];
  rollingRed[sensorIdx][idx]   = r;
  rollingGreen[sensorIdx][idx] = g;
  rollingBlue[sensorIdx][idx]  = b;
  rollingIdx[sensorIdx] = (idx + 1) % ROLLING_SIZE;

  long sumR = 0, sumG = 0, sumB = 0;
  for (int n = 0; n < ROLLING_SIZE; n++) {
    sumR += rollingRed[sensorIdx][n];
    sumG += rollingGreen[sensorIdx][n];
    sumB += rollingBlue[sensorIdx][n];
  }
  redVals[sensorIdx]   = sumR / ROLLING_SIZE;
  greenVals[sensorIdx] = sumG / ROLLING_SIZE;
  blueVals[sensorIdx]  = sumB / ROLLING_SIZE;
  avgVals[sensorIdx]   = (redVals[sensorIdx] + greenVals[sensorIdx] + blueVals[sensorIdx]) / 3;
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

void enqueueTrack(int track) {
  int nextTail = (queueTail + 1) % QUEUE_SIZE;
  if (nextTail == queueHead) return; 
  audioQueue[queueTail] = track;
  queueTail = nextTail;
}

void serviceQueue() {
  if (queueHead == queueTail) return;           
  if (digitalRead(busyPin) == LOW) return;      
  myDFPlayer.play(audioQueue[queueHead]);
  queueHead = (queueHead + 1) % QUEUE_SIZE;
}