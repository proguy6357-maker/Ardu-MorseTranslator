const int buzzerPin = 8; // Left Pin (S / Signal)

// Timing settings (will be updated dynamically by commands)
int dotDuration = 90;           
int dashDuration = dotDuration * 3; 
int symbolGap = dotDuration;     
int letterGap = dotDuration * 3; 
int wordGap = dotDuration * 6;   
const int glitchFreq = 1200; 

// String buffer to hold dots and dashes for a single letter (Manual decoding)
String currentLetter = "";

// Morse Code Lookup Tables
const char* morseLetters[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..",    // A-I
  ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.",  // J-R
  "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.."         // S-Z
};
const char* morseNumbers[] = {
  "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---...", "----."
};

// Updates all dependent gaps based on the current dotDuration
void updateTimings() {
  dashDuration = dotDuration * 3;
  symbolGap = dotDuration;
  letterGap = dotDuration * 3;
  wordGap = dotDuration * 6;
}

void setup() {
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("Morse Translator 1.0");
  Serial.println("Instructions:");
  Serial.println("1. Type '.' or '-' manually (End letters with Space, Words with '/')");
  Serial.println("2. Commands available:");
  Serial.println("   /speed [ms]      -> e.g., /speed 40");
  Serial.println("   /wpm [words/min] -> e.g., /wpm 20");
  Serial.println("   /wordmorse [msg] -> e.g., /wordmorse HELLO WORLD");
  Serial.println("-------------------------Enjoy!");
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim(); // Clean trailing spaces/newlines

    if (input.length() == 0) return;

    // --- COMMAND: /speed ---
    if (input.startsWith("/speed ")) {
      int newSpeed = input.substring(7).toInt();
      if (newSpeed > 0) {
        dotDuration = newSpeed;
        updateTimings();
        Serial.print("Base speed updated to: ");
        Serial.print(dotDuration);
        Serial.println(" ms");
      }
    } 
    // --- COMMAND: /wpm ---
    else if (input.startsWith("/wpm ")) {
      int wpm = input.substring(5).toInt();
      if (wpm > 0) {
        // Standard formula: Dot duration (ms) = 1200 / WPM
        dotDuration = 1200 / wpm;
        updateTimings();
        Serial.print("Speed set to ");
        Serial.print(wpm);
        Serial.print(" WPM (Dot: ");
        Serial.print(dotDuration);
        Serial.println(" ms)");
      }
    } 
    // --- COMMAND: /wordmorse ---
    else if (input.startsWith("/wordmorse ")) {
      String phrase = input.substring(11);
      phrase.toUpperCase();
      Serial.print("Encoding phrase: ");
      Serial.println(phrase);
      encodePhraseToMorse(phrase);
    } 
    // --- MANUAL INPUT: Single chars/strings (. - / space) ---
    else {
      for (unsigned int i = 0; i < input.length(); i++) {
        char inputChar = input.charAt(i);
        
        if (inputChar == '.') {
          currentLetter += ".";
          dot();
        } 
        else if (inputChar == '-') {
          currentLetter += "-";
          dash();
        } 
        else if (inputChar == ' ') {
          decodeMorseLetter();
          delay(letterGap);
        } 
        else if (inputChar == '/') {
          decodeMorseLetter(); 
          Serial.print(" ");
          delay(wordGap);
        }
      }
      // Automatically decode at the end of the manual line
      decodeMorseLetter();
      Serial.println();
    }
  }
}

// Decodes dots/dashes into a character (Manual Mode)
void decodeMorseLetter() {
  if (currentLetter.length() == 0) return;

  bool found = false;

  for (int i = 0; i < 26; i++) {
    if (currentLetter == morseLetters[i]) {
      Serial.print((char)('A' + i));
      found = true;
      break;
    }
  }

  if (!found) {
    for (int i = 0; i < 10; i++) {
      if (currentLetter == morseNumbers[i]) {
        Serial.print((char)('0' + i));
        found = true;
        break;
      }
    }
  }

  if (!found) {
    Serial.print("?");
  }

  currentLetter = ""; 
}

// Encodes English text into Morse audio and prints signals to Serial
void encodePhraseToMorse(String text) {
  for (unsigned int i = 0; i < text.length(); i++) {
    char c = text.charAt(i);

    if (c >= 'A' && c <= 'Z') {
      const char* morse = morseLetters[c - 'A'];
      Serial.print(morse);
      Serial.print(" ");
      playMorseSequence(morse);
      delay(letterGap - symbolGap); // Adjusted since playMorseSequence already adds a symbolGap
    } 
    else if (c >= '0' && c <= '9') {
      const char* morse = morseNumbers[c - '0'];
      Serial.print(morse);
      Serial.print(" ");
      playMorseSequence(morse);
      delay(letterGap - symbolGap);
    } 
    else if (c == ' ') {
      Serial.print("/ ");
      delay(wordGap - letterGap); // Adjusted gap for complete word
    }
  }
  Serial.println(); // Done encoding phrase
}

// Helper to play a sequence of dots/dashes from text encoding
void playMorseSequence(const char* sequence) {
  int i = 0;
  while (sequence[i] != '\0') {
    if (sequence[i] == '.') {
      dot();
    } else if (sequence[i] == '-') {
      dash();
    }
    i++;
  }
}

void dot() {
  tone(buzzerPin, glitchFreq); 
  digitalWrite(buzzerPin, HIGH); 
  delay(dotDuration);
  noTone(buzzerPin);
  digitalWrite(buzzerPin, LOW);
  delay(symbolGap);
}

void dash() {
  tone(buzzerPin, glitchFreq);
  digitalWrite(buzzerPin, HIGH);
  delay(dashDuration);
  noTone(buzzerPin);
  digitalWrite(buzzerPin, LOW);
  delay(symbolGap);
}
