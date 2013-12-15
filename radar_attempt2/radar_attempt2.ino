const uint16_t TICK_CNT = 3; // 255-(16MHz/1024/62Hz)  
static uint16_t freq = 0;
double sped = 0; //"speed" seems to be a reserved term
unsigned long Timer;
const unsigned long T_MIN = 500;
const unsigned long T_MAX = 4000;
const unsigned long T_VALID = 100;
float freq_stored = 0;
unsigned long time_stored = 0;
float freq_last = 0;
const int ledPin = 13;

void setup() {
  
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);

  noInterrupts();                     // disable all interrupts while we configure  
  // init Timer1 - 16-bit timer/counter
  TCNT1   = 0;                                  // start count at zero.        
  TCCR1B  |= _BV(CS12) | _BV(CS11) | _BV(CS10); // Increment T1 input on each positive edge 
                                                // using an external source. Table 16-5, pg 139.
  
  // init Timer2 - 8-bit timer/counter
  TCNT2   = TICK_CNT;                 // preload Timer2 to interrupt every 250 msec
  TIMSK2  = _BV(TOIE2);               // enable the Timer2 overflow interrupt 
  TCCR2B  |= _BV(CS22) |_BV(CS21) | _BV(CS20);   // init clock prescaler to 1024. Table 18-9, page 164.
  interrupts();                       // enable all interrupts
  Serial.println("Ready");
}

ISR(TIMER1_OVF_vect) {
  // do nothing. this is just a dummy ISR in case it actually overflows.
  Serial.println("Inside Timer1 Overflow Interrupt.");
}

ISR(TIMER2_OVF_vect) {
  //Serial.print("TCNT1: ");
  //Serial.println(TCNT1);
  freq = TCNT1;
  //Serial.println(freq);
  TCNT1 = 0;
  TCNT2 = TICK_CNT;
}

void loop() {
  if(freq != 0) {
    if(digitalRead(ledPin)==HIGH) {
      Serial.println(Timer);
      Timer = millis(); //reset Timer
      digitalWrite(ledPin, LOW);            
      // Test for a Valid Swing Loop
      while( millis() - Timer < T_MIN) // Ignore waggles
      {
        //ignore input
        noInterrupts();
        Serial.print("Inside test for valid swing loop. TCNT0=");
        Serial.println(Timer);
      }
            
      // Get some data!
      interrupts();

      // Update Stored Frequency Loop
      while(millis() - Timer < T_MAX) {
        Serial.println("Inside update stored frequency loop.");
          while(freq > freq_stored) {
              freq_stored = freq;
              time_stored = Timer;
          }
      }
      
      Serial.print("Freq:\t");
      Serial.print(freq_stored);
      Serial.print(" Time:\t");
      Serial.println(time_stored);
      if(freq_stored == freq_last) {
          Serial.println("Same speed as last time.");
      }
      freq = 0;
    }
    else {
      delay(1000);
    }
  }
  else {
    digitalWrite(ledPin, HIGH); //turn on Ready light
    Timer = millis(); //reset Timer
  }
}
