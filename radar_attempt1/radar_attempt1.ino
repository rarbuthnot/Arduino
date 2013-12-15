const uint16_t TICK_CNT = 3; // 255-(16MHz/1024/62Hz)  
static uint16_t freq = 0;
double sped = 0; //"speed" seems to be a reserved term

void setup() {

  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  
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
  
  Serial.println("Ready...");
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
  
  if (freq != 0) {
      freq = freq * 62;      // multiple the frequency * 62 to get "true" frequency. 62Hz from TICK_CNT times 62 = 1 sec.
      sped = freq * .03225;  // multiplying "true" freq * 0.03225 will give speed in mph. 31Hz == 1 mph.
                             // see: http://www.microwave-solutions.com/contents/en-uk/d13_System_Design.html
      Serial.print("Freq: ");
      Serial.print(freq, DEC);
      Serial.print(" Hz,\t Speed: ");
      Serial.print(sped, 0);
      Serial.println(" mph");
      freq = 0;
  }
}


