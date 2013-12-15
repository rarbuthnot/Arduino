const int ledPin = 13;
const uint16_t TICK_CNT = 3; // 255-(16MHz/1024/62Hz)  
const unsigned long TIME_VALID =  150;
const unsigned long TIME_MIN   =  500;
const unsigned long TIME_MAX   = 3000;

unsigned long time_init;
static uint16_t freq = 0;
float speed = 0;
float stored_speed = 0;
float stored_time = 0;
float speed_last_time = 0;

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
  Serial.println("Ready...");
}

ISR(TIMER2_OVF_vect) {
  freq = TCNT1;     // save the count
  speed = freq; // 
  //freq = 0; // added this line to clear out freq.
  TCNT1 = 0;        // reset the count
  TCNT2 = TICK_CNT; // preload Timer2
  //Serial.println(freq*62);
}

void loop() {
  BEGINNING:
  turn_ready_on();

  time_init = millis(); // millis is our timer
                        // get elapsed time by millis()-time_init
  CHECK_MOTION:
  if( freq != 0 ) { 
    if( is_ready_on() ) {
      turn_ready_off();
      time_init = millis();
      CHECK_VALID:
      if(millis()-time_init > TIME_VALID) {
        CHECK_MIN:
        if(millis()-time_init > TIME_MIN) {
          stored_speed = 0;
          turn_ready_off();
          COMPARE_SPEED:
          if(speed > stored_speed) { 
            stored_speed = speed;
            speed = 0; // added this line to clear out speed.
            stored_time = float(millis()-time_init) / 1000;
            CHECK_MAX:
            if(millis()-time_init > TIME_MAX) {  
              stored_speed = stored_speed * 62 * .03225;
              Serial.print(stored_speed, 0);
              Serial.print("mph; Tempo: ");
              Serial.println(stored_time, 2);
              freq = 0;
              
              if(same_speed_as_last_time()) {
                Serial.println("Same speed as last time.");
              } 
              else {
                speed_last_time = stored_speed;
              }
              // do we need to clear some variables here?
              goto BEGINNING;
            } 
            else { 
              goto COMPARE_SPEED; 
            }
          } 
          else { 
            goto CHECK_MAX; 
          }
        } 
        else { 
          goto CHECK_MIN; 
        }
      } 
      else { 
        goto CHECK_MOTION; 
      }
    } 
    else { 
      goto CHECK_VALID; 
    }
  } 
  else {
    if( is_ready_on() ) {
      goto CHECK_MOTION;
    } 
    else {
      HOLD_LOOP:
      if (millis()-time_init > TIME_MAX) {
        goto BEGINNING;
      } 
      else { 
        goto HOLD_LOOP; 
      }
    }
  }    
}

void turn_ready_on() {
  digitalWrite(ledPin, HIGH);
}

void turn_ready_off() {
  digitalWrite(ledPin, LOW);
}

boolean is_ready_on() {
  return digitalRead(ledPin);
}

boolean motion() {
  if(freq != 0) {
    return true;
  } else {
    return false;
  }
}

boolean same_speed_as_last_time() {
  if (stored_speed == speed_last_time) {
    return true;
  } else {
    return false;
  }
}

ISR(TIMER1_OVF_vect) {
  // do nothing. this is just a dummy ISR in case it actually overflows.
  Serial.println("Inside Timer1 Overflow Interrupt.");
}

