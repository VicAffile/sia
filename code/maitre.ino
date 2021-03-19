#include <Wire.h> // Librairie utilisée pour la communication des deux Arduino
#include <Key.h> // Librairie utilisée pour le Keypad
#include <Keypad.h> // Librairie utilisée pour le Keypad


# define ADRESSE_ESCLAVE 1 // Adresse de l'Arduino esclave

// Déclaration et initialisation du Keypad
const byte LIGNES = 4; // Déclaration du nombre de ligne du Keypad
const byte COLONNES = 4; // Déclaration du nombre de colonne du Keypad
char entrer[LIGNES][COLONNES] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
}; // Déclaration du contenu du Keypad
byte lignes_pins[LIGNES] = {9, 8, 7, 6}; // Déclaration des pins utilisées pour déterminer la ligne sur laquelle de la touche appuyée
byte colonnes_pins[COLONNES] = {5, 4, 3, 2}; // Déclaration des pins utilisées pour déterminer la colonne sur laquelle de la touche appuyée
Keypad keypad = Keypad( makeKeymap(entrer), lignes_pins, colonnes_pins, LIGNES, COLONNES ); // Déclaration de l'objet Keypad

// Déclaration des pins du Capteur à Ultrasons
const int PIN_TRIG = 11;
const int PIN_ECHO = 10;

//Déclaration des variables liées au Capteur à Ultrasons
long temps;
float distance;

//Déclaration du Bouton Poussoir
const int PIN_BOUTON = 13;

// Déclaration de la variable qui gère la sortie de la pièce
boolean sorti = 0; // Variable utilisé pour savoir si une sorti de la pièce est demandée
boolean porte_ouverte = 0; // Variable utilisé pour savoir si la porte à été ouverte

void setup(){
  // Initialisation moniteur
  Serial.begin(9600);
  Serial.println("Je suis le maître");

  // Initialisation maître
  Wire.begin(ADRESSE_ESCLAVE);

  // Initialisation Capteur à Ultrasons
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  // Initialisation Bouton Poussoir
  pinMode(PIN_BOUTON, INPUT_PULLUP);
}
  
void loop(){  
  boolean etat_bouton = digitalRead(PIN_BOUTON); // Récupère l'état du bouton, 0 s'il est enfoncé et 1 sinon

  // Fonction permettant de sortir par la porte en désactivant l'alarme pour un passage lorsque le bouton est enfoncé
  if (etat_bouton == 0) {
    sorti = 1;
    Serial.println("Alarme débranchée");
  }

  char entrer = keypad.getKey(); // Récupère la valeur de la touche préssée sur le keypad

  // Fonction se déclenchant lorsqu'une touche est préssée
  if (entrer){
    envoie(entrer); // Appelle la fonction d'envoie de valeur à l'arduino esclave et pour lui envoyer la touche préssée
    Serial.print("La touche préssée est : ");
    Serial.println(entrer);
  }

  sonnar(); // Appel de la fonction gérant le sonar
}


// Fonction permettant à l'arduino maître d'envoyer une information à l'arduino esclave
void envoie (int paquet) {
  Wire.beginTransmission(ADRESSE_ESCLAVE); // Démarre la transmission avec l'esclave
  Wire.write(paquet); // Envoie de "paquet"  
  Wire.endTransmission(); // Arrêtre la transmission avec l'esclave
  Serial.print("Envoie de la valeur : ");
  Serial.println(paquet);      
}

// Fonction gérant le sonnar
void sonnar () {
   digitalWrite(PIN_TRIG, HIGH); // Emission d'ultrasons
   delayMicroseconds(10);
   digitalWrite(PIN_TRIG, LOW); // Fin d'émission d'ultrasons
   
   temps = pulseIn(PIN_ECHO, HIGH); // Récuperation du temps qu'on mient les ultrasons à atteindre un obstacle et à revenir

   // Interprètation du temps mit par les ultrasons
   if (temps > 25000) {
     Serial.println("Echec de la mesure"); // Si le temps dépasse 25 secondes on cpnsidère que la mesure a échoué
   } else {
    temps /= 2; // Détermine le temps qu'on mit les ultrasons à atteindre l'obstacle
    distance = (temps*340)/10000.0; // Produit en croix pour déterminer à quel distance en cm se situe l'obstacle
    // L'arme se déclenche si le cpateur ne perçoit plus la porte au delà de 8cm
    if (distance > 8) {
      // Si on a pas appuyé sur le bouton pour sortir et que la porte s'ouvre
      if (sorti == 0) {
        envoie(1); // Appelle la fonction d'envoie de valeur à l'arduino esclave et pour lui dire que la porte a été ouverte sans avoir appué sur le bouton
        Serial.print("Distance : ");
        Serial.print(distance);
        Serial.println(" cm.");
        Serial.println("La porte a été ouverte");

      // Si on a appuyé sur le bouton et que la porte n'avait pas encore été ouverte
      } else if (sorti == 1 && porte_ouverte == 0) {
        porte_ouverte = sorti; // On stocke l'ouverture de la porte
        Serial.println("Porte ouverte");
      }
    // Si la porte a été ouverte et qu'elle est maintenant fermé
    } else if (distance <= 10 && porte_ouverte == 1) {
      sorti = !sorti; // La personne qui appuyé sur le bouton est sorti
      porte_ouverte = !porte_ouverte; // La porte est fermée
      Serial.println("Porte fermée");
      Serial.println("Alarme rebranchée");
    }
  }
}
