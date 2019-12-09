/* Tässä on yhdistetty ultraäänianturia ohjaava 'ultrasonic' ja servomoottoria ohjaava 'servo' koodi.
 * 
 * Ultraäänianturin kytkentä: Vcc +5V, Trig pinniin 7, Echo pinniin 8, GND maahan.
 * Servomoottorin kytkentä: punainen johto +5V, ruskea johto GND, oranssi johto PIN 9.
 * 
 * Tämä koodi toimii Arduino Unossa. Arduino Megaa varten tee kommenteissa kerrotut muutokset tai poista kommenttimerkit.
 */

#include <Servo.h>
//#include <SoftwareSerial.h>     // Arduino Megan porttien määrittely

Servo myservo;                  // luodaan servo objekti servomoottorin ohjaamista varten

char nappi = 'x';               // muuttuja johon tallennetaan Processingilta tuleva tieto onko nappia painettu (1) vai ei (0).
int pos = 10;                   // muuttuja servon sijaintitietoa varten, eli mihin kulmaan tutka osoittaa
int suunta = 0;                 // tutkan liikkeen suunta: 0 vasemmalta oikealle, 1 oikealta vasemmalle
unsigned long duration = 0;     // muuttuja duration äänen kulkuaikaa varten, alustetaan nollaksi
unsigned long distance = 0;     // muuttuja distance etäisyyden laskemista varten, alustetaan nollaksi
int tutkaMode = 0;              // muuttujaan tallennetaan onko tutka päällä (true) vai pois päältä (false). Bluetooth voi muuttaa tutkan tilaa.

const int trigPin = 7;                // anturin output pin 7
const int echoPin = 8;                // anturin input pin 8
unsigned long maximumRange = 100;     // tutkan mittaama suurin etäisyys oletuksena 100 cm. Käyttäjä voi määrittää lyhyemmän etäisyyden.
#define soundSpeed 0.03432            // ultraäänen nopeus yksikkönä cm/us (eli 343.2 m/s) 

void setup() {
  Serial.begin(9600);
//Serial1.begin(9600);                  // Arduino Megan sarjaliikenne
  pinMode(echoPin, INPUT);            // echo-pinni inputiksi
  pinMode(trigPin, OUTPUT);           // trig-pinni outputiksi
  myservo.attach(9);                  // servo Arduinon 9-pinniin
}

void loop() {
    if (Serial.available()) {         // Jos dataa on luettavissa sarjaportista....   // HUOM: Muuta Serial1 jos käytössä Arduino Mega
      nappi = Serial.read();          // luetaan ja tallennetaan nappi:iin            // HUOM: Muuta Serial1 jos käytössä Arduino Mega
      } 
    delay(10);                        // delay "varmuuden vuoksi"

    switch (nappi) {
        case '0':                        // KÄYTTÄJÄ PAINAA STOP-NAPPIA ANDROID-SOVELLUKSESSA
          if (tutkaMode == 1) {          // Jos tutka ON käynnissä....
            tutkaMode = 0;               // asetetaan tutkan tila 'pysähtyneeksi'...
            delay(20);                   // ja pysäytetään tutka eli ei kutsuta servoMove-funktiota. Lisätään pikku viive huvin vuoksi.
          } else if (tutkaMode == 0) {   // Jos tutka EI ole käynnissä...
            delay(20);                   // ei käynnistetä tutkaa. Pikku viive kaiken varalta.
          }
          nappi = 'x';                    // Nollataan nappi seuraavaa kierrosta varten (merkki x, koska 0 oli jo varattu).
          break;
        
        case '1':                         // KÄYTTÄJÄ PAINAA START-NAPPIA ANDROID-SOVELLUKSESSA
          if (tutkaMode == 0) {           // Jos tutka EI ole käynnissä....
            tutkaMode = 1;                // asetetaan tutkan tila 'käynnissäolevaksi' ja...
            servoMove();                  // käynnistetään tutka.
          } else if (tutkaMode == 1) {    // Jos tutka ON käynnissä...
            servoMove();                  // jatketaan mittaamista.
          }          
          nappi = 'x';                    // Nollataan nappi seuraavaa kierrosta varten (merkki x, koska 0 oli jo varattu).
          break;
        
        default:                          // KÄYTTÄJÄ EI PAINA MITÄÄN NAPPIA
          if (tutkaMode == 0) {           // Jos tutka ei ole päällä...
            delay(20);                    // ei tehdä muuta kuin odotellaan.
          } else if (tutkaMode == 1) {    // Jos tutka on päällä...
            servoMove();                  // annetaan tutkan olla päällä.
          }
          nappi = 'x';                    // Nollataan nappi seuraavaa kierrosta varten (merkki x, koska 0 oli jo varattu).
        break;
    }     
}


void servoMove(void) {
  /*
   * Liikutetaan servomoottoria askeleittain ensin yhteen suuntaan ja sitten takaisin.
   * Joka askeleella kutsutaan ultraäänianturia ohjaavaa funktiota, joka mittaa äänipulssin kulkuajan ja laskee etäisyyden kohteeseen.
   * Sitten lähetetään sarjaporttiin servon suuntaustieto ja laskettu etäisyys.
   */

  if (pos <= 170 && suunta == 0) {
    myservo.write(pos);       // liikutetaan servo muuttujan 'pos' mukaiseen sijaintiin
    delay(5);
    distance = getDistance(); // etäisyyden laskeminen getDistance-funktiolla
    delay(30);                // 30ms viive ennen seuraavaa askelta (äänen kulkuaika anturin max.kantaman päähän ja takaisin noin 24ms)
    pos = pos + 2;            
    if (pos >= 170) {         // Jos on saavutettu servon oikeanpuolimmaisin piste, käännetään kulkusuunta oikealta vasemmalle.
      suunta = 1;
    }

    Serial.print(pos);        // lähettää sarjaporttiin tutkan suuntakulman
    Serial.print(",");
    Serial.print(distance);   // lähettää sarjaporttiin etäisyyden senttimetreinä 
    Serial.print(".");

    delay(1);
  } 
  else if (pos >= 10 && suunta == 1) {
    myservo.write(pos);
    delay(5);
    distance = getDistance();
    delay(30);
    pos = pos - 2;
    if (pos <= 10) {
      suunta = 0;
    }

    Serial.print(pos);        // lähettää sarjaporttiin tutkan suuntakulman
    Serial.print(",");        
    Serial.print(distance);   // lähettää sarjaporttiin etäisyyden senttimetreinä 
    Serial.print(".");

    delay(1);
  }    
}

unsigned long getDistance() {         // TÄMÄ FUNKTIO LÄHETTÄÄ ULTRAÄÄNIPULSSIN, MITTAA VASTAANOTETUN PULSSIN KULKUAJAN, LASKEE KULKUAJAN PERUSTEELLA ETÄISYYDEN, JA PALAUTTAA ETÄISYYDEN
  /* Lähetetään äänipulssi asettamalla trig HIGH:ksi. Sitä ennen varmistetaan
   *  siisti HIGH pulssi asettamalla trig LOW:ksi 4us ajaksi. 
   */
  digitalWrite(echoPin, LOW);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(4);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(15);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);  // kohteesta heijastuneen pulssin kulkuaika saadaan kun echo-pinni on HIGH, kulkuaika talteen muuttujaan duration
  
  /* Lasketaan ajan perusteella etäisyys kohteeseen.
   * Etäisyys on äänen nopeus kerrottuna kulkuajalla jaettuna kahdella.
   * Jaetaan kahdella, koska mitattu kulkuaika on kulkuaika kohteeseen ja takaisin anturille.
   */
  distance = soundSpeed * duration / 2;
  if (distance > maximumRange) {          // Jos etäisyys on käyttäjän valitseman mittausalueen ulkopuolella...
    distance = 0;                         // merkitään etäisyydeksi 0.
  }
  return distance; 
}


