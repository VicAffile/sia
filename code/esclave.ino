#include <Wire.h> // Librairie utilisée pour la communication des deux Arduino
#include <LiquidCrystal.h> // Librairie utilisée pour le LCD


# define ADRESSE_ESCLAVE 1 // Adresse de l'Arduino esclave

int valeur_recu = 0; // C'est dans cette variable que l'esclave stocke les valeurs envoyés par le maître

// Déclaration des variables qui contiendront les caractères tapés sur le Keypad
char entrer1 = 'E'; // J'y affecte E par défaut car c'est un caractère qui n'est pas présent sur le Keypad
char entrer2 = 'E';
char entrer3 = 'E';
char entrer4 = 'E';

// Déclaration des variables contenant le mot de passe
const char caratere1 = '0';
const char caratere2 = '1';
const char caratere3 = '0';
const char caratere4 = '4';

// Déclaration des pins de la Led RVB
const byte PIN_LED_ROUGE = 8;
const byte PIN_LED_VERTE = 9;
const byte PIN_LED_BLEUE = 10;

// Déclaration des pins du LCD
LiquidCrystal lcd(12,11,5,4,3,2);

// Déclaration du pin du Buzzer
const int PIN_BUZZER = 13;

// Déclaration des variables propres au code
unsigned long duree_actuelle = 0; // Contient la durée actuel
unsigned long moment_mot_de_passe_saisi = 0; // Contient le moment où un mot de passe valide a été saisi
unsigned long moment_alarme_declanchee = 0; // Contient le moment où l'alarme se déclanche
int mot_de_passe_saisi = 0; // Indique si un mot de passe valide ou non a été saisi
int intrusion = 0; // Indique si une intrusion est en cours
int nombre_erreur = 0; // Indique le nombre d'erreur à la saisi au Keypad à la suite
const int nombre_erreur_max = 3; // Indique le nombre d'erreur à la saisi au Keypad à la suite avant le déclenchement du Buzzer
int alarme_led = 0; // Statu de la Led RVB lorsque l'alarme est déclenchée
int nombre_intrusion = 0; // Comptabilise le nombre de foi où l'alarme est déclenchée
float sinVal; // Variable utilisé pour le son du Buzzer
int toneVal; // Variable utilisé pour le son du Buzzer

void setup() {
  // Initialisation moniteur
  Serial.begin(9600);
  Serial.println("Je suis l'esclave, et fière de l'être mdr");
  delay(1000);
  
  // Initialisation esclave
  Wire.begin(ADRESSE_ESCLAVE); // Se déclare comme esclave et donne son adresse
  Wire.onReceive(message_recu); // Déclenche la fonction message_recu() lorsque l'arduino reçoit une communication du maître
  
  // Initialisation LCD
  Serial.begin(9600);
  lcd.begin(16,2);
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("MOT DE PASSE");
  lcd.setCursor(6,1);
  lcd.print("----");

  // Initialise Led RVB
  pinMode(PIN_LED_ROUGE, OUTPUT);
  pinMode(PIN_LED_VERTE, OUTPUT);
  pinMode(PIN_LED_BLEUE, OUTPUT);
  allumer_led(0, 0, 0);

  // Initialisation Buzzer
  pinMode(PIN_BUZZER, OUTPUT);
}

void loop() {
  // Se déclanche lorsqu'un mot de passe à été saisi
  if (mot_de_passe_saisi == 1) {
    mot_de_passe_accepte(); // S'il est valable
  } else if (mot_de_passe_saisi == -1) {
    mot_de_passe_refuse(); // S'il est faux
  }

  // Si le nombre d'erreur lors de la saisie au Keypad atteint la limite autorisé ou tout simplement qu'il y a une intrusion en cours
  if (nombre_erreur >= nombre_erreur_max || intrusion == 1) {
    alarme(); // Déclenche l'alarme
  } else {
    noTone(PIN_BUZZER); // Eteind le Buzzer
  }
}

// Fonction déclenché lorsque l'Arduino reçoit un message du maître
void message_recu(int nombre_byte) { 
  valeur_recu = Wire.read(); // Récupère la valeur envoyé par l'Arduino maître
  Serial.print("Valeur reçu : ");
  Serial.println(valeur_recu);
  
  // Si c'est une valeur correspondant au touche du Keypad
  if ((valeur_recu == 35 || valeur_recu == 42 || (valeur_recu >= 48 && valeur_recu <= 57) || (valeur_recu >= 65 && valeur_recu <= 68)) && mot_de_passe_saisi == 0) {
    mot_de_passe(valeur_recu); // On appelle la fonction qui gère les mots de passe

  // Si le maître veut déclencher l'arme et qu'il n'y a pas déjà une intrusion en cours
  } else if (valeur_recu == 1 && intrusion != valeur_recu) {
    intrusion = valeur_recu; // On passe en état d'intrusion
    nombre_intrusion++; // On incremente le nombre d'intrusion
    Serial.print("Nombre d'intrusion : ");
    Serial.println(nombre_intrusion);
  }
}

// Fonction appelée lorque le maître a envoyé un message et que c'est une touche du Keypad
void mot_de_passe (char charactere) {
  Serial.print("Touche tapée : ");
  Serial.println(charactere);
  // Ces conditions déterminent la position du caratère dans le mot de passe
  if (entrer1 == 69) {
      entrer1 = charactere;
      afficher_charatere(entrer1, 6);
    } else if (entrer2 == 69) {
      entrer2 = charactere;
      afficher_charatere(entrer2, 7);
    } else if (entrer3 == 69) {
      entrer3 = charactere;
      afficher_charatere(entrer3, 8);
    } else if (entrer4 == 69) {
      entrer4 = charactere;
      afficher_charatere(entrer4, 9);
    }
    // Si quatre caratères ont été tapés 
    if (entrer4 != 69) {
      // On enregistre le moment où le mot de passe à été sais
      moment_mot_de_passe_saisi = millis();
      moment_alarme_declanchee = millis();
      verification_mot_de_passe(); // On appelle la fonction vérifiant la validité du mot de passe saisi
      // Réinitialisation du mot de passe
      entrer1 = 'E';
      entrer2 = 'E';
      entrer3 = 'E';
      entrer4 = 'E';
    }
}


// Fonction vérifiant le mot de passe saisi
void verification_mot_de_passe () {
  // Si le mot de passe est valide
    if (entrer1 == caratere1 && entrer2 == caratere2 && entrer3 == caratere3 && entrer4 == caratere4) {
      lcd.setCursor(5, 1); // Déplace le cuseur du LCD
      lcd.print("VALIDE"); // Affiche validé sur le LCD
      Serial.println("Accès autorisé");
      allumer_led(0, 255, 0); // Appelle la fonction qui allume la Led RVB pour l'allumer, ici en vert
      mot_de_passe_saisi = 1; // Indique qu'un mot de passe valide a été saisi
      nombre_erreur = 0; // Ramène le nombre d'erreur à 0
      intrusion = 0; //On enlève l'état d'intrusion
      return; // On arrète la fonction ici
    }
    // Si le mot de passe ne correspond pas 
    Serial.println("Accès refusé");
    lcd.setCursor(3, 1); // Déplace le cuseur du LCD
    lcd.print("NON VALIDE"); // Affiche non validé sur le LCD
    allumer_led(255, 0, 0); // Appelle la fonction qui allume la Led RVB pour l'allumer, ici en rouge
    mot_de_passe_saisi = -1; // Indique qu'un mot de passe non valide a été saisi
    nombre_erreur++; // On incrémente le nombre d'erreur
}

// Fonction affichant sur le LCD le caratère tapé sur le Keypad
void afficher_charatere (char charactere, int x) {
  lcd.setCursor(x, 1); // Déplace le cuseur du LCD
  lcd.print(charactere); // Affiche le caratère tapé sur le LCD
}

// Fonction allumant la Led RVB
void allumer_led (int r, int v, int b) {
  digitalWrite(PIN_LED_ROUGE, r); // Paramètre la Led rouge
  digitalWrite(PIN_LED_VERTE, v); // Paramètre la Led verte
  digitalWrite(PIN_LED_BLEUE, b); // Paramètre la Led bleue
}

// Fonction éteignant la Led RVB
void eteindre_led () {
  digitalWrite(PIN_LED_ROUGE, 0);
  digitalWrite(PIN_LED_VERTE, 0);
  digitalWrite(PIN_LED_BLEUE, 0);
}

// Se déclenche si le mot de passe a été accepté
void mot_de_passe_accepte () {
  duree_actuelle = millis(); // Stocke le moment actuel
  // Compare avec le moment où le mot de passe à été saisi
  if (duree_actuelle-moment_mot_de_passe_saisi > 10000) {
    mot_de_passe_saisi = 0; // Enlève le mot de passe saisi de la mémoire
    eteindre_led(); // On éteind la Led RVB
    lcd.setCursor(5,1); // Déplace le cuseur du LCD
    lcd.print(" ---- "); // Remet l'affichage initial sur le LCD
  }
}

// Se déclenche si le mot de passe a été refusé
void mot_de_passe_refuse () {
  duree_actuelle = millis(); // Stocke le moment actuel
  // Compare avec le moment où le mot de passe à été saisi
  if (duree_actuelle - moment_mot_de_passe_saisi > 1500) {
    mot_de_passe_saisi = 0; // Enlève le mot de passe saisi de la mémoire
    eteindre_led(); // On éteind la Led RVB
    lcd.setCursor(3,1); // Déplace le cuseur du LCD
    lcd.print("   ----   "); // Remet l'affichage initial sur le LCD
  }
}

void alarme () {
  duree_actuelle = millis(); // Stocke le moment actuel
  
  //On allume et étaind la led à intervalle régulier
  if (duree_actuelle - moment_alarme_declanchee >= 500) {
    // Selon l'état de la Led RVB
    if (alarme_led == 1) {
      eteindre_led(); // On éteind la led
    } else {
      allumer_led(255, 0, 0); // On l'allume en Rouche
    }
    alarme_led = !alarme_led; // Inversion de l'état de la Led LED
    moment_alarme_declanchee = duree_actuelle; // Mise à jour du moment
  }
  // Activation du Buzzer en produisant un son type alarme
  for (int i = 0; i < 180; i++) {
    sinVal = (sin(i*(3.1412/180)));
    toneVal = 2000+(int(sinVal*1000));
    tone(PIN_BUZZER, toneVal); // Le Buzzer joue cette fréquence
    delay(2);
  }
}
