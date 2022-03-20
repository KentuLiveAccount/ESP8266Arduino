const int ledPin = 2; //4 works too!
 
void setup() {
  Serial.begin(9600);
  Serial.println("Setup");
  // put your setup code here, to run once:
  ledcSetup(0 /*channel*/, 200 /*PWM frequency*/, 8 /*resolution*/);
  ledcAttachPin(ledPin, 0 /*channel*/);
  Serial.println("SetupEnd");
}

void lowtohi()
{
  ledcWriteTone(0, 100);
  delay(1000);
  ledcWriteTone(0, 500);
  delay(1000);
  ledcWriteTone(0, 1000);
  delay(1000);
  ledcWriteTone(0, 2000);
  delay(1000);
  ledcWriteTone(0, 3000);
  delay(1000);
  ledcWriteTone(0, 4000);
  delay(1000);
  ledcWriteTone(0, 5000);
  delay(1000);
  ledcWriteTone(0, 6000);
  delay(1000);
  ledcWriteTone(0, 7000);
  
}

void notes()
{
  //octave 0 and 1 fails
  for (size_t i = 2; i < 9; i++)
  {
    Serial.print("Write NoteC in Octave: ");
    Serial.println(String(i));
    ledcWriteNote(0, NOTE_C, i);
    delay(1000);
  }
}

void twinkle()
{
  note_t note[]={NOTE_C,NOTE_C,NOTE_G,NOTE_G,NOTE_A,NOTE_A,NOTE_G,NOTE_F,NOTE_F,NOTE_E,NOTE_E,NOTE_D,NOTE_D,NOTE_C};
  int len[]={1000,1000,1000,1000,1000,1000,2000,1000,1000,1000,1000,1000,1000,2000};
  for (size_t i = 0; i < (sizeof(note)/sizeof(note_t)); i++)
  {
    ledcWriteNote(0, note[i], 6);
    delay(len[i]);
    ledcWriteTone(0, 0);
    delay(50);
  }
  ledcWriteTone(0, 0);
}

void alternate()
{
  ledcWriteTone(0, 3000);
  delay(200);
  ledcWriteTone(0, 4000);
  delay(200);
  ledcWriteTone(0, 3000);
  delay(200);
  ledcWriteTone(0, 4000);
  delay(200);
  ledcWriteTone(0, 3000);
  delay(200);
  ledcWriteTone(0, 4000);
  delay(200);
  ledcWriteTone(0, 3000);
  delay(200);
  ledcWriteTone(0, 4000);
  delay(200);
}

void loop() {
  // put your main code here, to run repeatedly:
  static uint8_t brightness = 0;
  static int diff = 5;
 
  Serial.println("WriteTone 1000");
  //lowtohi();
  //delay(1000);
  //notes();
  //delay(1000);
  twinkle();
  delay(1000);
  //alternate();
  Serial.println("WriteTone 0");
  ledcWriteTone(0, 0);
  delay(50);
}
