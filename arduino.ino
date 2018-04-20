int fsrPin = 0;     // the FSR and 10K pulldown are connected to a0
int fsrReading;     // the analog reading from the FSR resistor divider
int fsrVoltage;     // the analog reading converted to voltage
unsigned long fsrResistance;  // The voltage converted to resistance, can be very big so make "long"
unsigned long fsrConductance;
long fsrForce;       // Finally, the resistance converted to force

void setup(void) {
  Serial.begin(9600);   // We'll send debugging information via the Serial monitor
}

void loop(void) {
  fsrReading = analogRead(fsrPin);
  //Serial.print("Analog reading = ");
  //Serial.println(fsrReading);

  // analog voltage reading ranges from about 0 to 1023 which maps to 0V to 5V (= 5000mV)
  //fsrVoltage = map(fsrReading, 0, 1023, 0, 5000);
    fsrVoltage = fsrReading*(5000/1023);
  //Serial.print("Voltage reading in mV = ");
  //Serial.println(fsrVoltage);

  if (fsrVoltage == 0) {
    Serial.print("I0");
    Serial.print("\n");
    delay(20);
  } else {
    // The voltage = Vcc * R / (R + FSR) where R = 10K and Vcc = 5V
    // so FSR = ((Vcc - V) * R) / V        yay math!
    fsrResistance = 5000 - fsrVoltage;     // fsrVoltage is in millivolts so 5V = 5000mV
    fsrResistance *= 10000;                // 10K resistor
    fsrResistance /= fsrVoltage;
    //Serial.print("FSR resistance in ohms = ");
    //Serial.println(fsrResistance);

    fsrConductance = 1000000;           // we measure in micromhos so
    fsrConductance /= fsrResistance;
    //Serial.print("Conductance in microMhos: ");
    //Serial.println(fsrConductance);

    // Use the two FSR guide graphs to approximate the force
    if (fsrConductance <= 1000) {
      fsrForce = fsrConductance / 80;
      Serial.print("I");
      Serial.print(fsrForce);
      Serial.print("\n");
      delay(20);
    } else {
      fsrForce = fsrConductance - 1000;
      fsrForce /= 30;
      Serial.print("I");
      Serial.print(fsrForce);
      Serial.print("\n");
      delay(20);
    }
  }
  //Serial.println("--------------------");
  delay(1000);
}
