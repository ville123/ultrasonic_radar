/* Ultraäänitutkan etäisyys- ja kulmatiedon graafinen näyttö.

(tarkista com-portin numero)    */

import processing.serial.*;      // serial communication -kirjaston tuonti
import java.awt.event.KeyEvent;
import java.io.IOException;

Serial myPort;                   
PFont myFont;                    

int radius = 350;                // muuttuja tutkasektorin sädettä varten
int angle = 10;                  // servomoottorin asentokulma (arduinossa asetettu alkamaan 10 asteesta)
int distance = 0;                // kohteen etäisyys 
int motion = 0;                  // servomoottorin liikkeen suunta
float x = 0.0;
float y = 0.0;
int w = 300;                      // keilan pituus
int radarDist = 0;                // tutkakuvan etäisyysmerkintöjä varten
int[] newValue = new int[161];    // uusi mittausarvo jokaiselta servon askeleelta
int[] oldValue = new int[161];    // edellinen mittausarvo
int firstRun = 0;

/* SETUP ************************************* */
void setup() {
  /* tässä asetetaan ruudun koko, taustaväri ja fontti */
  size(750, 450);
  background (0);  // taustaväri mustaksi (0)
  myFont = createFont("CalibriBold", 20);
  textFont(myFont);
/* sarjaportti ja puskuri */
  myPort = new Serial(this, "COM1", 9600);
  myPort.bufferUntil('.');    // lukee sarjaportin datan '.'-merkkiin asti
}



/* DRAW ************************************** */

void draw() {
  fill(0);                            // täyteväriksi musta
  noStroke();                         // ei ääriviivoja
  ellipse(radius, radius, 750, 750);  // piirretään ympyrä jonka halkaisija 750 ja keskipisteen määrää 'radius'
  rectMode(CENTER);                   // nelikulmion orientaatio (keskipisteen (x,y) ympärille)
  rect(350,402,800,100);              // piirretään nelikulmio (x,y,leveys,korkeus)
  
  if (angle >= 170) {                 // tutkakeilan liikkeen suunta
    motion = 1;                       // oikealta vasemmalle
  }
  if (angle <= 10) {
    motion = 0;                       // vasemmalta oikealle
  }
  
  /* Tutkakeilan setup */
  
  strokeWeight(7);                    // viivan paksuus
  if (motion == 0) {                  // keila etenee vasemmalta oikealle
    for (int i=0; i<=15; i++) {       // piirrä 15 viivaa joiden väri himmenee asteittain  (visuaalinen efekti)
      stroke(0,(10*i),0);             // värin asetus (RGB) i:n arvon pohjalta
      line(radius, radius, radius + cos(radians(angle+(180+i)))*w, radius + sin(radians(angle+(180+i)))*w);  // viiva: line(start x, start y, end x, end y)
    }
} else {                              // keila etenee oikealta vasemmalle
    for (int i = 15; i >= 0; i--) {
      stroke(0,150-(10*i),0);
      line(radius, radius, radius + cos(radians(angle+(180+i)))*w, radius + sin(radians(angle+(180+i)))*w); 
    }
  }


/* Tutkan havaitsemien kohteiden piirto */
  noStroke();                           // ei ääriviivoja
  /* 1. pyyhkäisy */
  fill(0,50,0);                         // ympyrän täyteväri RGB
  beginShape();                         // piirretään...
    for (int i = 0; i < 160; i++) {     // taulukon jokaiselle kulman arvolle
      x = radius + cos(radians((180+i)))*((oldValue[i])); // x koordinaatti
      y = radius + sin(radians((180+i)))*((oldValue[i])); // y koordinaatti
      ellipse(x, y, 5, 5);                                // ympyrän piirto
    }
  endShape();                           // piirron lopetus
  /* 2. pyyhkäisy */
  fill(0,110,0);
  beginShape();
    for (int i = 0; i < 160; i++) {
      x = radius + cos(radians((180+i)))*(newValue[i]);
      y = radius + sin(radians((180+i)))*(newValue[i]);
      ellipse(x, y, 5, 5);
    }
  endShape();
  /* keskiarvo */
  fill(110,110,0);
  beginShape();
    for (int i = 0; i < 160; i++) {
      x = radius + cos(radians((180+i)))*((newValue[i]+oldValue[i])/2); // lasketaan 1. ja 2. pyyhkäisyn pisteiden keskiarvo
      y = radius + sin(radians((180+i)))*((newValue[i]+oldValue[i])/2);
      ellipse(x, y, 10, 10);
    }
  endShape();
  /* liikkuvan kohteen merkkaus kahden ensimmäisen pyyhkäisyn jälkeen */
  if (firstRun >= 360) {        // kaksi ensimmäistä pyyhkäisyä tehty, aletaan merkkaamaan mahdollinen liike
    stroke(150,0,0);            // väriksi punainen          
    strokeWeight(1);            // ja ohut viiva
    noFill();                   // ei täyttöä
      for (int i = 0; i < 160; i++) {  // käydään kulmat läpi
        if (oldValue[i] - newValue[i] > 35 || newValue[i] - oldValue[i] > 35) {  // jos pyyhkäisyjen välillä riittävästi eroa..
          x = radius + cos(radians((180+i)))*(newValue[i]);
          y = radius + sin(radians((180+i)))*(newValue[i]);
        ellipse(x, y, 10, 10);         // ... piirretään ympyrä jonka keskipiste x,y 
        }
      }
  }
  
  
  /* Tutkakuvan etäisyyskaarten piirto */
  
  for (int i = 0; i <= 6; i++) {
    noFill();
    strokeWeight(1);
    stroke(0,255-(30*i),0);
    ellipse(radius,radius,(100*i),(100*i));
    fill(0,100,0);
    noStroke();
  } 
  radarDist = 0;
  for (int i = 0; i <= 6; i++) {
    strokeWeight(1);
    stroke(0,55,0);
    line(radius, radius, radius + cos(radians(180+(30*i)))*w, radius + sin(radians(180+(30*i)))*w);
    fill(0,55,0);
    noStroke();
  }
  

// Tekstin kirjoitus
  
  noStroke();
  fill(0);
  rect(350,402,800,100);
  fill(0, 200, 0);
  text("Suunta: " + angle, 130,380,200,50);  //
  text("Etäisyys: " + distance, 130,410,200,50);   // text(string,x,y,leveys,korkeus) 
  text("Ohjelmoinnin sovellusprojekti",600,410,450,50); 
  fill(0);
  rect(70,60,150,100);
  fill(0,50,0);
  ellipse(30,53,5,5);
  text("1. keilaus", 115, 70, 150, 50);
  fill(0,110,0);
  ellipse(30,73,5,5);
  text("2. keilaus", 115, 90, 150, 50);
  fill(110,110,0);
  ellipse(30,93,10,10);
  text("Keskiarvo", 115, 110, 150, 50);
  noFill();
  stroke(150,0,0);
  strokeWeight(1);
  ellipse(29, 113, 10, 10);
  fill(150,0,0);
  text("Kohde liikkunut", 115, 130, 150, 50);
} 



/* sarjaportin data */
void serialEvent (Serial myPort) {
  try {
    String xString = myPort.readStringUntil('.');    // luetaan kunnes tulee piste.
    if (xString != null) {
      String getAngle = xString.substring(0,xString.indexOf(","));  // kulmatieto
      String getDistance = xString.substring(xString.indexOf(",")+1, xString.indexOf("."));  // etäisyystieto
      angle = Integer.parseInt(getAngle);
      distance = Integer.parseInt(getDistance);
      oldValue[angle] = newValue[angle];    // talletetaan arvot taulukkoon
      newValue[angle] = 3*distance;         // distance kerrotaan kolmella että saadaan helposti skaalattua kohteiden näyttöä tutkassa sopivampaan kohtaan
      firstRun++;
      if (firstRun > 320) {
        firstRun = 320;  // pidetään arvona 320 kun pyyhkäisty kahdesti sektorin läpi
      }
    }
  }
    catch(Exception e) {
     println("Ohjelmoinnin sovellusprojekti 2016");
    }
}