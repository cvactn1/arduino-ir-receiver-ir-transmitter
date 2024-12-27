const int IrLedPin =  2;    // LED connected to digital pin 2
const int CopyButtonPin = 6;
const int SendButtonPin = 5;

enum Mode {
  COPY,
  SEND,
};

Mode mode = Mode::SEND;

#define IRpin_PIN      PIND
#define IRpin          7
// the maximum pulse we'll listen for - 65 milliseconds is a long time
#define MAXPULSE 65000
 
// what our timing resolution should be, larger is better
// as its more 'precise' - but too large and you wont get
// accurate timing
#define RESOLUTION 20 
 
// we will store up to 100 pulse pairs (this is -a lot-)
uint16_t pulses[100][2];  // pair is high and low pulse 
uint8_t currentpulse = 0; // index for pulses we're storing

void setup()   {               
//  pinMode(IrLedPin, OUTPUT);     
  pinMode(CopyButtonPin, INPUT_PULLUP);
  pinMode(SendButtonPin, INPUT_PULLUP);
  Serial.begin(9600);
}                     

void loop()                    
{
  bool copy_button_click = (digitalRead(CopyButtonPin) == LOW);
  if (copy_button_click && mode != Mode::COPY) {
    mode = Mode::COPY;
    currentpulse = 0; 
    Serial.println("Mode switched to COPY");
  }

  if (mode == Mode::COPY) {
    bool finished = CopyIrSignal();
    if (finished) {
      mode = Mode::SEND;
      Serial.println("Mode switched to SEND");
    }
  }
  
  if (mode == Mode::SEND){
    bool send_button_click = (digitalRead(SendButtonPin) == LOW);
    if (send_button_click) {
      SendIrSignal();
      Serial.println("Signal was sent successfully!!!");
    }
  }
}

// This procedure sends a 38KHz pulse to the IrLedPin
// for a certain # of microseconds. We'll use this whenever we need to send codes
void pulseIR(long microsecs) {
  // we'll count down from the number of microseconds we are told to wait

  cli();  // this turns off any background interrupts

  while (microsecs > 0) {
    // 38 kHz is about 13 microseconds high and 13 microseconds low
   digitalWrite(IrLedPin, HIGH);  // this takes about 3 microseconds to happen
   delayMicroseconds(10);         // hang out for 10 microseconds
   digitalWrite(IrLedPin, LOW);   // this also takes about 3 microseconds
   delayMicroseconds(10);         // hang out for 10 microseconds

   // so 26 microseconds altogether
   microsecs -= 26;
  }

  sei();  // this turns them back on
}

void SendIrSignal() {

  for (uint8_t i = 0; i < currentpulse; i++) {
//    Serial.print(pulses[i][0] * RESOLUTION, DEC);
//    Serial.print(" usec, ");
//    Serial.print(pulses[i][1] * RESOLUTION, DEC);
//    Serial.println(" usec");
    delayMicroseconds(pulses[i][0] * RESOLUTION);
    pulseIR(pulses[i][1] * RESOLUTION);
  }
  // This is the code for the CHANNEL + for the TV COMCAST
 
//  delayMicroseconds(29600);      //Time off (LEFT column)       
//  pulseIR(920);                               //Time on (RIGHT column)    <-------DO NOT MIX THESE UP
//  delayMicroseconds(900);
//  pulseIR(1800);
}

bool CopyIrSignal(void) {
  uint16_t highpulse, lowpulse;  // temporary storage timing
  highpulse = lowpulse = 0; // start out with no pulse length
 
 
//  while (digitalRead(IRpin)) { // this is too slow!
    while (IRpin_PIN & (1 << IRpin)) {
     // pin is still HIGH
 
     // count off another few microseconds
     highpulse++;
     delayMicroseconds(RESOLUTION);
 
     // If the pulse is too long, we 'timed out' - either nothing
     // was received or the code is finished, so print what
     // we've grabbed so far, and then reset
     if ((highpulse >= MAXPULSE) && (currentpulse != 0)) {
       printpulses();
//       currentpulse=0;
       return true;
     }
  }
  // we didn't time out so lets stash the reading
  pulses[currentpulse][0] = highpulse;
 
  // same as above
  while (! (IRpin_PIN & _BV(IRpin))) {
     // pin is still LOW
     lowpulse++;
     delayMicroseconds(RESOLUTION);
     if ((lowpulse >= MAXPULSE)  && (currentpulse != 0)) {
       printpulses();
//       currentpulse=0;
       return true;
     }
  }
  pulses[currentpulse][1] = lowpulse;
 
  // we read one high-low pulse successfully, continue!
  currentpulse++;
  return false;
}

void printpulses(void) {
  Serial.println("\n\r\n\rReceived: \n\rOFF \tON");
  for (uint8_t i = 0; i < currentpulse; i++) {
    Serial.print(pulses[i][0] * RESOLUTION, DEC);
    Serial.print(" usec, ");
    Serial.print(pulses[i][1] * RESOLUTION, DEC);
    Serial.println(" usec");
  }
}
