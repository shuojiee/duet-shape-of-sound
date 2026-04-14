// Colour sensors pin assignments
const int NUM_SENSORS = 4;
const int S2_PINS[NUM_SENSORS]  = {5, 7, 9, 11};
const int S3_PINS[NUM_SENSORS]  = {6, 8, 10, 12};
const int OUT_PINS[NUM_SENSORS] = {2, 3, 4, 13};

const unsigned long PULSE_TIMEOUT = 5000;

// Store RGB values
unsigned long redVals[NUM_SENSORS] = {0};
unsigned long greenVals[NUM_SENSORS] = {0};
unsigned long blueVals[NUM_SENSORS] = {0};

// Store calibration RGB values
unsigned long redMin[NUM_SENSORS] = {0};
unsigned long greenMin[NUM_SENSORS] = {0};
unsigned long blueMin[NUM_SENSORS] = {0};
unsigned long redMax[NUM_SENSORS] = {0};
unsigned long greenMax[NUM_SENSORS] = {0};
unsigned long blueMax[NUM_SENSORS] = {0};

void setup() {
  // baud rate 11520
  Serial.begin(115200);

/* 
  set S2 & S3 pins of each sensor as output
  set OUT pins of each sensor as input
*/
  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(S2_PINS[i], OUTPUT);
    pinMode(S3_PINS[i], OUTPUT);
    pinMode(OUT_PINS[i], INPUT);
  }
}

void loop() {
/*
  calibrate and print calibration values
*/
  calibrateSensors();
  printVals();
}

/*
  set S2 & S3 pins HIGH or LOW to read each RGB value:
  (LOW, LOW)    --> Red
  (HIGH, HIGH)  --> Green
  (LOW, HIGH)   --> Blue
  (HIGH, LOW)   --> Clear (No Filter)

  measure pulse width (frequency) in microseconds to determine light intensity
*/
unsigned long readPulseWidth(int sensorIdx, bool s2Val, bool s3Val) {
  
  digitalWrite(S2_PINS[sensorIdx], s2Val);
  digitalWrite(S3_PINS[sensorIdx], s3Val);
  
  delayMicroseconds(100);

  // pulseIn() starts reading pulse width after pin goes LOW
  return pulseIn(OUT_PINS[sensorIdx], LOW, PULSE_TIMEOUT);
}

unsigned long stableRead(int i, bool s2, bool s3) {
  unsigned long sum = 0;

  for (int n = 0; n < 5; n++) {
    sum += readPulseWidth(i, s2, s3);
    delay(10);
  }
  return sum / 5;
}

void calibrateSensors() {
  Serial.println("Calibrating sensors...");

  // WHITE calibration
  Serial.println("Place a WHITE marble in front of every sensor, then press ENTER...");
  while (!Serial.available());
  Serial.read();

  for (int i = 0; i < NUM_SENSORS; i++) {
    redMin[i] = stableRead(i, LOW, LOW);
    greenMin[i] = stableRead(i, HIGH, HIGH);
    blueMin[i] = stableRead(i, LOW, HIGH);

    Serial.print("Sensor "); Serial.print(i); Serial.println(" white calibrated.");
  }

  // BLACK calibration
  Serial.println("Place a BLACK marble in front of every sensor, then press ENTER...");
  while (!Serial.available());
  Serial.read();

  for (int i = 0; i < NUM_SENSORS; i++) {
    redMax[i] = stableRead(i, LOW, LOW);
    greenMax[i] = stableRead(i, HIGH, HIGH);
    blueMax[i] = stableRead(i, LOW, HIGH);

    Serial.print("Sensor "); Serial.print(i); Serial.println(" black calibrated.");
  }

  Serial.println("Calibration complete!");
}

/*
  WHITE values (lower value indicates higher light intensity)
  redMin:   {14, 15, 13, 14}
  greenMin: {13, 13, 10, 15}
  blueMin:  {11, 11, 8, 12}

  BLACK values
  redMax:   {50, 59, 61, 56}
  greenMax: {46, 50, 56, 58}
  blueMax:  {39, 46, 43, 48}
*/
void printVals() {
  Serial.println("\n// ===== PASTE THIS BLOCK INTO duet.ino =====\n");

  Serial.println("// WHITE calibration (min pulse width = brightest)");
  Serial.print("unsigned long redMin[NUM_SENSORS]   = {");
  for (int i = 0; i < NUM_SENSORS; i++) { Serial.print(redMin[i]);   if (i < NUM_SENSORS-1) Serial.print(", "); } Serial.println("};");
  Serial.print("unsigned long greenMin[NUM_SENSORS] = {");
  for (int i = 0; i < NUM_SENSORS; i++) { Serial.print(greenMin[i]); if (i < NUM_SENSORS-1) Serial.print(", "); } Serial.println("};");
  Serial.print("unsigned long blueMin[NUM_SENSORS]  = {");
  for (int i = 0; i < NUM_SENSORS; i++) { Serial.print(blueMin[i]);  if (i < NUM_SENSORS-1) Serial.print(", "); } Serial.println("};");

  Serial.println();

  Serial.println("// BLACK calibration (max pulse width = darkest)");
  Serial.print("unsigned long redMax[NUM_SENSORS]   = {");
  for (int i = 0; i < NUM_SENSORS; i++) { Serial.print(redMax[i]);   if (i < NUM_SENSORS-1) Serial.print(", "); } Serial.println("};");
  Serial.print("unsigned long greenMax[NUM_SENSORS] = {");
  for (int i = 0; i < NUM_SENSORS; i++) { Serial.print(greenMax[i]); if (i < NUM_SENSORS-1) Serial.print(", "); } Serial.println("};");
  Serial.print("unsigned long blueMax[NUM_SENSORS]  = {");
  for (int i = 0; i < NUM_SENSORS; i++) { Serial.print(blueMax[i]);  if (i < NUM_SENSORS-1) Serial.print(", "); } Serial.println("};");

  Serial.println("\n// ===== END PASTE BLOCK =====\n");
}
