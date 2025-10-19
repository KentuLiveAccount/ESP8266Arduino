const int ledPin = 2; //4 works too!
 
void setup() {
  Serial.begin(9600);
  Serial.println("Setup");
  // put your setup code here, to run once:
  // ledcSetup(0 /*channel*/, 200 /*PWM frequency*/, 8 /*resolution*/);
  // ledcAttachPin(ledPin, 0 /*channel*/);

  ledcAttach(ledPin, 200 /* PWM frequency*/, 8 /* resolution*/);
  Serial.println("SetupEnd");
}

void lowtohi()
{
  ledcWriteTone(ledPin, 100);
  delay(1000);
  ledcWriteTone(ledPin, 500);
  delay(1000);
  ledcWriteTone(ledPin, 1000);
  delay(1000);
  ledcWriteTone(ledPin, 2000);
  delay(1000);
  ledcWriteTone(ledPin, 3000);
  delay(1000);
  ledcWriteTone(ledPin, 4000);
  delay(1000);
  ledcWriteTone(ledPin, 5000);
  delay(1000);
  ledcWriteTone(ledPin, 6000);
  delay(1000);
  ledcWriteTone(ledPin, 7000);
  
}

void notes()
{
  //octave 0 and 1 fails
  for (size_t i = 2; i < 9; i++)
  {
    Serial.print("Write NoteC in Octave: ");
    Serial.println(String(i));
    ledcWriteNote(ledPin, NOTE_C, i);
    delay(1000);
  }
}

void twinkle()
{
  note_t note[]={NOTE_C,NOTE_C,NOTE_G,NOTE_G,NOTE_A,NOTE_A,NOTE_G,NOTE_F,NOTE_F,NOTE_E,NOTE_E,NOTE_D,NOTE_D,NOTE_C};
  int len[]={1000,1000,1000,1000,1000,1000,2000,1000,1000,1000,1000,1000,1000,2000};
  for (size_t i = 0; i < (sizeof(note)/sizeof(note_t)); i++)
  {
    ledcWriteNote(ledPin, note[i], 6);
    delay(len[i]);
    ledcWriteTone(ledPin, 0);
    delay(50);
  }
  ledcWriteTone(ledPin, 0);
}

void alternate(int f1, int f2)
{
  for (int i = 0; i < 4; i++)
  {
    ledcWriteTone(ledPin, f1);
    delay(200);
    ledcWriteTone(ledPin, f2);
    delay(200);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  static uint8_t brightness = 0;
  static int diff = 5;
 
  Serial.println("WriteTone 1000");
  //lowtohi();
  //delay(1000);
  //notes();
  alternate(500, 1000);
  delay(1000);
  alternate(3000, 4000);
  delay(1000);
  alternate(500, 2000);
  delay(1000);
  alternate(1000, 3000);
  delay(1000);
  twinkle();
  delay(1000);
  //alternate();
  Serial.println("WriteTone 0");
  ledcWriteTone(ledPin, 0);
  delay(50);
}
