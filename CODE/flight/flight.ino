#include <SPI.h>
#include <SPIMemory.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_BMP3XX.h>

#define FLASH_CS    10
#define SPI_MOSI    11
#define SPI_MISO    13
#define SPI_CLK     12

#define TRIG_START  1
#define TRIG_STOP   2
#define LED_PIN     21

#define I2C_SDA     8
#define I2C_SCL     9
#define MPU_INT     4

#define MPU_ADDR    0x68
#define BMP_ADDR    0x76

#define META_SECTOR 0
#define DATA_START_ADDR  4096

#define SAMPLE_RATE_US 10000
#define AUTO_STOP_MS   300000

SPIClass SPI_EXT(FSPI);
SPIFlash flash(FLASH_CS, &SPI_EXT);

Adafruit_MPU6050 mpu;
Adafruit_BMP3XX  bmp;

struct __attribute__((packed)) LogEntry {
  uint32_t timestamp;
  int16_t  accel[3];
  int16_t  gyro[3];
  float    pressure;        // Pa
  float    temperature;     // °C
};

struct __attribute__((packed)) SessionInfo {
  bool      dataValid;
  uint32_t  entryCount;
  uint32_t  crc;
};

bool loggingActive = false;
uint32_t logStartTime = 0;
uint32_t lastSampleTime = 0;
uint32_t writeAddress = DATA_START_ADDR;
uint32_t entryCount = 0;
SessionInfo meta;

void startLogging();
void stopLogging();
void logSample();
void dumpCSV();
void eraseFlashData();
void readMeta();
void writeMeta();
void printStatus();

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(TRIG_START, INPUT_PULLUP);
  pinMode(TRIG_STOP,  INPUT_PULLUP);
  SPI_EXT.begin(SPI_CLK, SPI_MISO, SPI_MOSI, FLASH_CS);
  if (!flash.begin()) {
    Serial.println("Flash Init Failed.");
    while(1);
  }
  Serial.println("Flash Nominal.");
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);

  if (!mpu.begin(MPU_ADDR, &Wire)) {
    Serial.println("MPU not found.");
    while(1);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("MPU Nominal.");

  if(!bmp.begin_I2C(BMP_ADDR, &Wire)) {
    Serial.println("BMP not found.");
    while(1);
  }
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);
  Serial.println("BMP Nominal.");

  readMeta();
  if (meta.dataValid && meta.entryCount > 0) {
    Serial.printf("Stored data: %u entries ready to dump. \n",meta.entryCount);
  }
  Serial.println("Ready. dump   erase   status");
}

void loop() {
  if (!loggingActive && digitalRead(TRIG_START) == HIGH) {
    delay(50);
    if (digitalRead(TRIG_START) == HIGH) {
      startLogging();
    }
  }

  if (loggingActive) {
    if (digitalRead(TRIG_STOP) == HIGH || millis() - logStartTime >= AUTO_STOP_MS) {
      delay(50);
      if (digitalRead(TRIG_STOP) == HIGH || millis() - logStartTime >= AUTO_STOP_MS) {
        stopLogging();
      }
    } else {
      uint32_t now = micros();
      if (now - lastSampleTime >= SAMPLE_RATE_US) {
        lastSampleTime = now;
        logSample();
      }
    }
  }

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toLowerCase();
    if (cmd == "dump") {
      if (!loggingActive) dumpCSV();
      else Serial.println("No dump during Log Session.");
    }
    else if (cmd == "erase") {
      if (!loggingActive) eraseFlashData();
      else Serial.println("No erase during Log Session.");
    }
    else if (cmd == "status") {
      printStatus();
    }
  }
}

void startLogging() {
  eraseFlashData();
  loggingActive = true;
  digitalWrite(LED_PIN, HIGH);
  logStartTime = millis();
  lastSampleTime = micros();
  writeAddress = DATA_START_ADDR;
  entryCount = 0;
  Serial.println("Logging Started Nominally.");
}

void stopLogging() {
  loggingActive = false;
  digitalWrite(LED_PIN, LOW);
  meta.dataValid = true;
  meta.entryCount = entryCount;
  writeMeta();
  Serial.printf("Logging Stopped Nominally. %u Samples were Saved. \n", entryCount);
}

void logSample() {
  sensors_event_t a, g, temp_mpu;
  mpu.getEvent(&a, &g, &temp_mpu);

  float pressure = bmp.readPressure();
  float temperature = bmp.readTemperature();
  LogEntry entry;
  entry.timestamp = millis() - logStartTime;
  entry.accel[0] = a.acceleration.x * 1000;
  entry.accel[1] = a.acceleration.y * 1000;
  entry.accel[2] = a.acceleration.z * 1000;
  entry.gyro[0] = g.gyro.x * 1000;
  entry.gyro[1] = g.gyro.y * 1000;
  entry.gyro[2] = g.gyro.z * 1000;
  entry.pressure = pressure;
  entry.temperature = temperature;

  uint8_t* p =(uint8_t*)&entry;
  for (size_t i = 0; i< sizeof(LogEntry); i++) {
    flash.writeByte(writeAddress++, p[i]);
  }

  entryCount++;
}
void dumpCSV() {
  readMeta();
  if (!meta.dataValid || meta.entryCount == 0) {
    Serial.println("No data to dump.");
    return;
  }

  Serial.println("timestamp_ms,accel_x,accel_y,accel_z,gyro_x,gyro_y,gyro_z,pressure_Pa,temperature_C");

  uint32_t addr = DATA_START_ADDR;
  LogEntry entry;

  for (uint32_t i = 0; i < meta.entryCount; i++) {
    uint8_t* p = (uint8_t*)&entry;
    for (size_t j = 0; j < sizeof(LogEntry); j++) {
      p[j] = flash.readByte(addr++);
    }

    Serial.printf("%lu,%d,%d,%d,%d,%d,%d,%.2f,%.2f\n",
      entry.timestamp,
      entry.accel[0],
      entry.accel[1],
      entry.accel[2],
      entry.gyro[0],
      entry.gyro[1],
      entry.gyro[2],
      entry.pressure,
      entry.temperature
      );
  }

  Serial.println("--- END CSV ---");
}

void readMeta() {
  uint8_t* p = (uint8_t*)&meta;
  uint32_t addr = META_SECTOR * 4096;
  for (size_t i = 0; i < sizeof(SessionInfo); i++) {
    p[i] = flash.readByte(addr++);
  }
}

void writeMeta() {
  flash.eraseSector(META_SECTOR);
  uint8_t* p = (uint8_t*)&meta;
  uint32_t addr = META_SECTOR * 4096;
  for (size_t i = 0; i < sizeof(SessionInfo); i++) {
    flash.writeByte(addr++, p[i]);
  }
}

void eraseFlashData() {
  for (uint32_t sector = 1; sector < 4096; sector++) {
    flash.eraseSector(sector);
  }
  meta.dataValid = false;
  meta.entryCount = 0;
  writeMeta();
  Serial.println("Flash data erased Nominally.");
}

void printStatus() {
  readMeta();
  Serial.printf("Logging: %s\n", loggingActive ? "ACTIVE" : "stopped");
  if (meta.dataValid) {
    Serial.printf("Stored entries: %u\n", meta.entryCount);
  } else {
    Serial.println("No stored data.");
  }
}