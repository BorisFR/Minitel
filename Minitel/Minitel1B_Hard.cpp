////////////////////////////////////////////////////////////////////////
/*
   Minitel1B_Hard - Fichier source - Version du 15 juin 2017 à 22h11
   Copyright 2016, 2017 - Eric Sérandour
   http://3615.entropie.org

   Documentation utilisée :
   Spécifications Techniques d'Utilisation du Minitel 1B
   http://543210.free.fr/TV/stum1b.pdf

////////////////////////////////////////////////////////////////////////

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
////////////////////////////////////////////////////////////////////////

#include "Minitel1B_Hard.h"

////////////////////////////////////////////////////////////////////////
/*
   Public
*/
////////////////////////////////////////////////////////////////////////

Minitel::Minitel(HardwareSerial& serial) : mySerial(serial) {
  // A la mise sous tension du Minitel, la vitesse des échanges entre
  // le Minitel et le périphérique est de 1200 bauds par défaut.
  mySerial.begin(1200);
}
/*--------------------------------------------------------------------*/

void Minitel::writeByte(byte b) {
  // Le bit de parité est mis à 0 si la somme des autres bits est paire
  // et à 1 si elle est impaire.
  boolean parite = 0;
  for (int i=0; i<7; i++) {
    if (bitRead(b,i) == 1)  {
	  parite = !parite;
	}
  }
  if (parite) {
    bitWrite(b,7,1);  // Ecriture du bit de parité
  }
  else {
    bitWrite(b,7,0);  // Ecriture du bit de parité
  }
  mySerial.write(b);  // Envoi de l'octet sur le port série
}
/*--------------------------------------------------------------------*/

byte Minitel::readByte() {
  byte b = mySerial.read();
  // Le bit de parité est à 0 si la somme des autres bits est paire
  // et à 1 si elle est impaire.	
  boolean parite = 0;
  for (int i=0; i<7; i++) {
    if (bitRead(b,i) == 1)  {
	  parite = !parite;
	}
  }
  if (bitRead(b,7) == parite) {  // La transmission est bonne, on peut récupérer la donnée.
	if (bitRead(b,7) == 1) {  // Cas où le bit de parité vaut 1.
      b = b ^ 0b10000000;  // OU exclusif pour mettre le bit de parité à 0 afin de récupérer la donnée.
    }
    return b;
  }
  else {
    return 0xFF;  // Pour indiquer une erreur de parité.
  }
}
/*--------------------------------------------------------------------*/

int Minitel::changeSpeed(int bauds) {  // Voir p.141
  // Format de la commande
  writeBytesPRO(2);  // 0x1B 0x3A
  writeByte(PROG);   // 0x6B
  switch (bauds) {
    case  300 : writeByte(0b1010010); mySerial.begin( 300); break;  // 0x52
    case 1200 : writeByte(0b1100100); mySerial.begin(1200); break;  // 0x64
    case 4800 : writeByte(0b1110110); mySerial.begin(4800); break;  // 0x76
    case 9600 : writeByte(0b1111111); mySerial.begin(9600); break;  // 0x7F (pour le Minitel 2 seulement)
  }
  // Acquittement
  return workingSpeed();  // En bauds (voir section Private ci-dessous)
}
/*--------------------------------------------------------------------*/

int Minitel::currentSpeed() {  // Voir p.141
  // Demande
  writeBytesPRO(1);
  writeByte(STATUS_VITESSE);
  // Réponse
  return workingSpeed();  // En bauds (voir section Private ci-dessous)
}
/*--------------------------------------------------------------------*/

int Minitel::searchSpeed() {
  const int SPEED[4] = { 1200, 4800, 300, 9600 };  // 9600 bauds pour le Minitel 2 seulement
  int i = 0;
  int speed;
  do {
    mySerial.begin(SPEED[i]);
    if (i++ > 3) { i = 0; }
    speed = currentSpeed();
  } while (speed < 0);
  return speed;  // En bauds
}
/*--------------------------------------------------------------------*/

void Minitel::newScreen() {
  writeByte(FF);
  currentSize = GRANDEUR_NORMALE;
}
/*--------------------------------------------------------------------*/

void Minitel::newXY(int x, int y) {
  if (x==1 && y==1) {
    writeByte(RS);
  }
  else {
    // Le code US est suivi de deux caractères non visualisés. Si les
    // octets correspondants à ces deux caractères appartiennent tous deux
    // aux colonnes 4 à 7, ils représentent respectivement (sous forme
    // binaire avec 6 bits utiles) le numéro de rangée et le numéro de
    // colonne du premier caractère du sous-article (voir p.96).
    writeByte(US);
    writeByte(0x40 + y);  // Numéro de rangée
    writeByte(0x40 + x);  // Numéro de colonne
  }
  currentSize = GRANDEUR_NORMALE;
}
/*--------------------------------------------------------------------*/

void Minitel::cursor() {
  writeByte(CON);
}
/*--------------------------------------------------------------------*/

void Minitel::noCursor() {
  writeByte(COFF);
}
/*--------------------------------------------------------------------*/

void Minitel::moveCursorXY(int x, int y) {  // Voir p.95
  attributs(CSI);   // 0x1B 0x5B
  writeBytesP(y);   // Pr : Voir section Private ci-dessous
  writeByte(0x3B);
  writeBytesP(x);   // Pc : Voir section Private ci-dessous
  writeByte(0x48);
}
/*--------------------------------------------------------------------*/

void Minitel::moveCursorLeft(int n) {  // Voir p.94 et 95
  if (n==1) { writeByte(BS); }
  else if (n>1) {
	// Curseur vers la gauche de n colonnes. Arrêt au bord gauche de l'écran.
    attributs(CSI);   // 0x1B 0x5B
    writeBytesP(n);   // Pn : Voir section Private ci-dessous
    writeByte(0x44);
  }
}
/*--------------------------------------------------------------------*/

void Minitel::moveCursorRight(int n) {  // Voir p.94
  if (n==1) { writeByte(HT); }
  else if (n>1) {
	// Curseur vers la droite de n colonnes. Arrêt au bord droit de l'écran.
    attributs(CSI);   // 0x1B 0x5B
    writeBytesP(n);   // Pn : Voir section Private ci-dessous
    writeByte(0x43);
  }
}
/*--------------------------------------------------------------------*/

void Minitel::moveCursorDown(int n) {  // Voir p.94
  if (n==1) { writeByte(LF); }
  else if (n>1) {
	// Curseur vers le bas de n rangées. Arrêt en bas de l'écran.
    attributs(CSI);   // 0x1B 0x5B
    writeBytesP(n);   // Pn : Voir section Private ci-dessous
    writeByte(0x42);
  }
}
/*--------------------------------------------------------------------*/

void Minitel::moveCursorUp(int n) {  // Voir p.94
  if (n==1) { writeByte(VT); }
  else if (n>1) {
	// Curseur vers le haut de n rangées. Arrêt en haut de l'écran.
    attributs(CSI);   // 0x1B 0x5B
    writeBytesP(n);   // Pn : Voir section Private ci-dessous
    writeByte(0x41);
  }	
}
/*--------------------------------------------------------------------*/

void Minitel::moveCursorReturn(int n) {  // Voir p.94
  writeByte(CR);
  moveCursorDown(n);  // Pour davantage de souplesse
}
/*--------------------------------------------------------------------*/

int Minitel::getCursorX() {
  return (getCursorXY() & 0x0000FF) - 0x40;
}
/*--------------------------------------------------------------------*/

int Minitel::getCursorY() {
  return ((getCursorXY() & 0x00FF00) >> 8) - 0x40;
}
/*--------------------------------------------------------------------*/

void Minitel::cancel() {  // Voir p.95
  writeByte(CAN);
}
/*--------------------------------------------------------------------*/

void Minitel::clearScreenFromCursor() {  // Voir p.95
  attributs(CSI);  // 0x1B 0x5B
  // writeByte(0x30);  Inutile
  writeByte(0x4A);
}
/*--------------------------------------------------------------------*/

void Minitel::clearScreenToCursor() {  // Voir p.95
  attributs(CSI);  // 0x1B 0x5B
  writeByte(0x31);
  writeByte(0x4A);
}
/*--------------------------------------------------------------------*/

void Minitel::clearScreen() {  // Voir p.95
  attributs(CSI);  // 0x1B 0x5B
  writeByte(0x32);
  writeByte(0x4A);
}
/*--------------------------------------------------------------------*/

void Minitel::clearLineFromCursor() {  // Voir p.95
  attributs(CSI);  // 0x1B 0x5B
  // writeByte(0x30);  Inutile
  writeByte(0x4B);
}
/*--------------------------------------------------------------------*/

void Minitel::clearLineToCursor() {  // Voir p.95
  attributs(CSI);  // 0x1B 0x5B
  writeByte(0x31);
  writeByte(0x4B);
}
/*--------------------------------------------------------------------*/

void Minitel::clearLine() {  // Voir p.95
  attributs(CSI);  // 0x1B 0x5B
  writeByte(0x32);
  writeByte(0x4B);
}
/*--------------------------------------------------------------------*/

void Minitel::deleteChars(int n) {  // Voir p.95
  attributs(CSI);  // 0x1B 0x5B
  writeBytesP(n);  // Voir section Private ci-dessous
  writeByte(0x50);
}
/*--------------------------------------------------------------------*/

void Minitel::insertChars(int n) {  // Voir p.95
  attributs(CSI);  // 0x1B 0x5B
  writeBytesP(n);  // Voir section Private ci-dessous
  writeByte(0x40);
}
/*--------------------------------------------------------------------*/

void Minitel::startInsert() {  // Voir p.95
  attributs(CSI);  // 0x1B 0x5B
  writeByte(0x34);
  writeByte(0x68);
}
/*--------------------------------------------------------------------*/

void Minitel::stopInsert() {  // Voir p.95
  attributs(CSI);  // 0x1B 0x5B
  writeByte(0x34);
  writeByte(0x6C);
}
/*--------------------------------------------------------------------*/

void Minitel::deleteLines(int n) {  // Voir p.95
  attributs(CSI);  // 0x1B 0x5B
  writeBytesP(n);  // Voir section Private ci-dessous
  writeByte(0x4D);
}
/*--------------------------------------------------------------------*/

void Minitel::insertLines(int n) {  // Voir p.95
  attributs(CSI);  // 0x1B 0x5B
  writeBytesP(n);  // Voir section Private ci-dessous
  writeByte(0x4C);
}
/*--------------------------------------------------------------------*/

void Minitel::textMode() {
  writeByte(SI);  // Accès au jeu G0 (voir p.100)
}
/*--------------------------------------------------------------------*/

void Minitel::graphicMode() {
  writeByte(SO);  // Accès au jeu G1 (voir p.101 & 102)
}
/*--------------------------------------------------------------------*/

int Minitel::pageMode() {
  // Commande
  writeBytesPRO(2);   // 0x1B 0x3A
  writeByte(STOP);    // 0x6A
  writeByte(ROULEAU); // 0x43
  // Acquittement
  return workingMode();
}
/*--------------------------------------------------------------------*/

int Minitel::scrollMode() {
  // Commande
  writeBytesPRO(2);   // 0x1B 0x3A
  writeByte(START);   // 0x69
  writeByte(ROULEAU); // 0x43
  // Acquittement
  return workingMode();
}
/*--------------------------------------------------------------------*/

void Minitel::attributs(byte attribut) {
  writeByte(ESC);  // Accès à la grille C1 (voir p.92)
  writeByte(attribut);
  if (attribut == DOUBLE_HAUTEUR || attribut == DOUBLE_GRANDEUR) {
    moveCursorDown(1);
    currentSize = attribut;
  }
  else if (attribut == GRANDEUR_NORMALE || attribut == DOUBLE_LARGEUR) {
    currentSize = attribut;
  }
}
/*--------------------------------------------------------------------*/

void Minitel::print(String chaine) {
  for (int i=0; i<chaine.length(); i++) {
    unsigned char caractere = chaine.charAt(i);
    if (!isDiacritic(caractere)) {
	  printChar(caractere);
	}
	else {
	  i+=1;  // Un caractère accentué prend la place de 2 caractères
	  caractere = chaine.charAt(i);
	  printDiacriticChar(caractere);
	}
  }
}
/*--------------------------------------------------------------------*/

void Minitel::println(String chaine) {
  print(chaine);
  if (currentSize == DOUBLE_HAUTEUR || currentSize == DOUBLE_GRANDEUR) {
    moveCursorReturn(2);
  }
  else {
    moveCursorReturn(1);
  }
}
/*--------------------------------------------------------------------*/

void Minitel::println() {
  if (currentSize == DOUBLE_HAUTEUR || currentSize == DOUBLE_GRANDEUR) {
    moveCursorReturn(2);
  }
  else {
    moveCursorReturn(1);
  }
}
/*--------------------------------------------------------------------*/

void Minitel::printChar(char caractere) {
  byte charByte = getCharByte(caractere);
  if (isValidChar(charByte)) {
    writeByte(charByte);
  }
}
/*--------------------------------------------------------------------*/

void Minitel::printDiacriticChar(unsigned char caractere) {
  writeByte(SS2);  // // Accès au jeu G2 (voir p.103)
  String diacritics = "àâäèéêëîïôöùûüçÀÂÄÈÉÊËÎÏÔÖÙÛÜÇ";
  // Dans une chaine de caractères, un caractère diacritique prend la
  // place de 2 caractères simples, ce qui explique le /2.
  int index = (diacritics.indexOf(caractere)-1)/2;
  char car;
  switch (index) {
    case( 0): car = 'a'; writeByte(ACCENT_GRAVE); break;
    case( 1): car = 'a'; writeByte(ACCENT_CIRCONFLEXE); break;
    case( 2): car = 'a'; writeByte(TREMA); break;
    case( 3): car = 'e'; writeByte(ACCENT_GRAVE); break;
    case( 4): car = 'e'; writeByte(ACCENT_AIGU); break;
    case( 5): car = 'e'; writeByte(ACCENT_CIRCONFLEXE); break;
    case( 6): car = 'e'; writeByte(TREMA); break;
    case( 7): car = 'i'; writeByte(ACCENT_CIRCONFLEXE); break;
    case( 8): car = 'i'; writeByte(TREMA); break;
    case( 9): car = 'o'; writeByte(ACCENT_CIRCONFLEXE); break;
    case(10): car = 'o'; writeByte(TREMA); break;
    case(11): car = 'u'; writeByte(ACCENT_GRAVE); break;
    case(12): car = 'u'; writeByte(ACCENT_CIRCONFLEXE); break;      
    case(13): car = 'u'; writeByte(TREMA); break;
    case(14): car = 'c'; writeByte(CEDILLE); break;
	case (15):
		car = 'A';
		break;
	case (16):
		car = 'A';
		break;
	case (17):
		car = 'A';
		break;
	case (18):
		car = 'E';
		break;
	case (19):
		car = 'E';
		break;
	case (20):
		car = 'E';
		break;
	case (21):
		car = 'E';
		break;
	case (22):
		car = 'I';
		break;
	case (23):
		car = 'I';
		break;
	case (24):
		car = 'O';
		break;
	case (25):
		car = 'O';
		break;
	case (26):
		car = 'U';
		break;
	case (27):
		car = 'U';
		break;
	case (28):
		car = 'U';
		break;
	case (29):
		car = 'C';
		break;
	}
	printChar(car);
}
/*--------------------------------------------------------------------*/

void Minitel::printSpecialChar(byte b) {
  // N'est pas fonctionnelle pour les diacritiques (accents, tréma et cédille)
  writeByte(SS2);  // Accès au jeu G2 (voir p.103)
  writeByte(b);
}
/*--------------------------------------------------------------------*/

byte Minitel::getCharByte(char caractere) {
  // Voir les codes et séquences émis en mode Vidéotex (Jeu G0 p.100).
  // Dans la chaine ci-dessous, on utilise l'échappement (\) :
  // \" rend au guillemet sa signification littérale.
  // \\ donne à l'antislash sa signification littérale .
  String caracteres = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_xabcdefghijklmnopqrstuvwxyz";
  return (byte) caracteres.lastIndexOf(caractere);
}
/*--------------------------------------------------------------------*/

void Minitel::graphic(byte b, int x, int y) {
  moveCursorXY(x,y);
  graphic(b);
}
/*--------------------------------------------------------------------*/

void Minitel::graphic(byte b) {
  // Voir Jeu G1 page 101.
  if (b <= 0b111111) {
    b = 0x20
      + bitRead(b,5) 
      + bitRead(b,4) * 2
      + bitRead(b,3) * 4
      + bitRead(b,2) * 8
      + bitRead(b,1) * 16
      + bitRead(b,0) * 64;
    if (b == 0x7F) {  // 0b1111111
      b= 0x5F;
    }    
  writeByte(b);
  }
}
/*--------------------------------------------------------------------*/

void Minitel::repeat(int n) {  // Voir p.98
  writeByte(REP);
  writeByte(0x40 + n);
}
/*--------------------------------------------------------------------*/

void Minitel::bip() {  // Voir p.98
  writeByte(BEL);
}
/*--------------------------------------------------------------------*/

void Minitel::rect(int x1, int y1, int x2, int y2) {
  hLine(x1,y1,x2,BOTTOM);
  vLine(x2,y1+1,y2,RIGHT,DOWN);
  hLine(x1,y2,x2,TOP);
  vLine(x1,y1,y2-1,LEFT,UP);
}
/*--------------------------------------------------------------------*/

void Minitel::hLine(int x1, int y, int x2, int position) {
  textMode();
  moveCursorXY(x1,y);
  switch (position) {
    case TOP    : writeByte(0x7E); break;
    case CENTER : writeByte(0x60); break;
    case BOTTOM : writeByte(0x5F); break;
  }
  repeat(x2-x1);
}
/*--------------------------------------------------------------------*/

void Minitel::vLine(int x, int y1, int y2, int position, int sens) {
  textMode();
  switch (sens) {
	case DOWN : moveCursorXY(x,y1); break;
    case UP   : moveCursorXY(x,y2); break;
  }
  for (int i=0; i<y2-y1; i++) {
    switch (position) {
      case LEFT   : writeByte(0x7B); break;
      case CENTER : writeByte(0x7C); break;
      case RIGHT  : writeByte(0x7D); break;
    }
    switch (sens) {
	  case DOWN : moveCursorLeft(1); moveCursorDown(1); break;
      case UP   : moveCursorLeft(1); moveCursorUp(1); break;
    }
  }
}
/*--------------------------------------------------------------------*/

unsigned long Minitel::getKeyCode() {
  unsigned long code = 0;
  // Code unique
  if (mySerial.available()>0) {
    code = readByte();
  }  
  // Séquences de deux ou trois codes (voir p.118)
  if (code == 0x19) {  // SS2
    while (!mySerial.available()>0);  // Indispensable
    code = (code << 8) + readByte();
    // Les diacritiques (3 codes)
    if ((code == 0x1941) || (code == 0x1942) || (code == 0x1943) || (code == 0x1948) || (code == 0x194B)) {  // Accents, tréma, cédille
	  while (!mySerial.available()>0);  // Indispensable
	  byte caractere = readByte();
	  code = (code << 8) + caractere;
	  switch (code) {  // On convertit le code reçu en un code que l'on peut visualiser sous forme d'un caractère dans le moniteur série du logiciel Arduino avec la fonction write().
	    case 0x194161 : code = 0xE0; break;  // à 
	    case 0x194165 : code = 0xE8; break;  // è
	    case 0x194175 : code = 0xF9; break;  // ù
	    case 0x194265 : code = 0xE9; break;  // é
	    case 0x194361 : code = 0xE2; break;  // â
	    case 0x194365 : code = 0xEA; break;  // ê
	    case 0x194369 : code = 0xEE; break;  // î
	    case 0x19436F : code = 0xF4; break;  // ô
	    case 0x194375 : code = 0xFB; break;  // û
	    case 0x194861 : code = 0xE4; break;  // ä
	    case 0x194865 : code = 0xEB; break;  // ë
	    case 0x194869 : code = 0xEF; break;  // ï
	    case 0x19486F : code = 0xF6; break;  // ö
	    case 0x194875 : code = 0xFC; break;  // ü
	    case 0x194B63 : code = 0xE7; break;  // ç
	    default : code = caractere; break;
	  }
	}
	// Les autres caractères spéciaux disponibles sous Arduino (2 codes)
	else {
	  switch (code) {  // On convertit le code reçu en un code que l'on peut visualiser sous forme d'un caractère dans le moniteur série du logiciel Arduino avec la fonction write().
	    case 0x1923 : code = 0xA3; break;  // Livre
	    case 0x1927 : code = 0xA7; break;  // Paragraphe
	    case 0x1930 : code = 0xB0; break;  // Degré
	    case 0x1931 : code = 0xB1; break;  // Plus ou moins
	    case 0x1938 : code = 0xF7; break;  // Division
	    case 0x197B : code = 0xDF; break;  // Bêta
	  }
	}
  }
  // Touches de fonction (voir p.123)
  else if (code == 0x13) {
    while (!mySerial.available()>0);  // Indispensable
    code = (code << 8) + readByte();
  }  
  // Touches de gestion du curseur lorsque le clavier est en mode étendu (voir p.124)
  // Pour passer au clavier étendu manuellement : Fnct C + E
  // Pour revenir au clavier vidéotex standard  : Fnct C + V
  else if (code == 0x1B) {
	delay(1);  // Indispensable. 0x1B seul correspond à la touche Esc,
	           // on ne peut donc pas utiliser la boucle while (!available()>0).
    if (mySerial.available()>0) {
      code = (code << 8) + readByte();
      while (!mySerial.available()>0);  // Indispensable
      code = (code << 8) + readByte();
    }	  
  }
// Pour test
/*
  if (code != 0) {
    Serial.print(code,HEX);
    Serial.print(" ");
    Serial.write(code);
    Serial.println("");
  }
*/ 
  return code;
}
/*--------------------------------------------------------------------*/

int Minitel::smallMode() {
  // Commande
  writeBytesPRO(2);       // 0x1B 0x3A
  writeByte(START);       // 0x69
  writeByte(MINUSCULES);  // 0x45
  // Acquittement
  return workingMode();
}
/*--------------------------------------------------------------------*/

int Minitel::capitalMode() {
  // Commande
  writeBytesPRO(2);       // 0x1B 0x3A
  writeByte(STOP);        // 0x6A
  writeByte(MINUSCULES);  // 0x45
  // Acquittement
  return workingMode();
}
/*--------------------------------------------------------------------*/

int Minitel::extendedKeyboard() {
  // Commande
  writeBytesPRO(3);                   // 0x1B 0x3B
  writeByte(START);                   // 0x69
  writeByte(CODE_RECEPTION_CLAVIER);  // 0x59
  writeByte(ETEN);                    // 0x41
  // Acquittement
  return workingKeyboard();	
}
/*--------------------------------------------------------------------*/

int Minitel::standardKeyboard() {
  // Commande
  writeBytesPRO(3);                   // 0x1B 0x3B
  writeByte(STOP);                    // 0x6A
  writeByte(CODE_RECEPTION_CLAVIER);  // 0x59
  writeByte(ETEN);                    // 0x41
  // Acquittement
  return workingKeyboard();	
}
/*--------------------------------------------------------------------*/




////////////////////////////////////////////////////////////////////////
/*
   Private
*/
////////////////////////////////////////////////////////////////////////

boolean Minitel::isValidChar(byte index) {
  // On vérifie que le caractère appartient au jeu G0 (voir p.100).
  // SP (0x20) correspond à un espace et DEL (0x7F) à un pavé plein.
  if (index >= SP && index <= DEL) {
    return true;
  }
  return false;
}
/*--------------------------------------------------------------------*/

boolean Minitel::isDiacritic(unsigned char caractere) {
	String accents = "àâäèéêëîïôöùûüçÀÂÄÈÉÊËÎÏÔÖÙÛÜÇ";
	if (accents.indexOf(caractere) >= 0)
	{
		return true; 
  	}
  return false;
}
/*--------------------------------------------------------------------*/

void Minitel::writeBytesP(int n) {
  // Pn, Pr, Pc : Voir remarques p.95 et 96
  if (n<=9) {
    writeByte(0x30 + n);
  }
  else {
    writeByte(0x30 + n/10);
    writeByte(0x30 + n%10);
  }
}
/*--------------------------------------------------------------------*/

void Minitel::writeBytesPRO(int n) {  // Voir p.134
  writeByte(ESC);  // 0x1B
  switch (n) {
    case 1 : writeByte(0x39); break;
    case 2 : writeByte(0x3A); break;
    case 3 : writeByte(0x3B); break;
  }
}
/*--------------------------------------------------------------------*/

int Minitel::workingSpeed() {
  int bauds = -1;
  while (!mySerial);  // On attend que le port soit sur écoute.
  unsigned long time = millis();
  unsigned long duree = 0;
  unsigned long trame = 0;  // 32 bits = 4 octets
  // On se donne 1000 ms pour récupérer une trame exploitable
  while ((trame >> 8 != 0x1B3A75) && (duree < 1000)) {  // Voir p.141
	if (mySerial.available() > 0) {
      trame = (trame << 8) + readByte();
      //Serial.println(trame, HEX);
    }
    duree = millis() - time;
  }
  switch (trame) {
    case 0x1B3A7552 : bauds =  300; break;
    case 0x1B3A7564 : bauds = 1200; break;
    case 0x1B3A7576 : bauds = 4800; break;
    case 0x1B3A757F : bauds = 9600; break;  // Pour le Minitel 2 seulement
  }
  return bauds;	
}
/*--------------------------------------------------------------------*/

byte Minitel::workingMode() {  // Voir p.143
  // On récupère notamment les 4 bits de poids faibles suivants : ME PC RL F
  // ME : mode minuscules / majuscules du clavier (1 = minuscule)
  // PC : PCE (1 = actif)
  // RL : rouleau (1 = actif)
  // F  : format d'écran (1 = 80 colonnes)
  while (!mySerial);  // On attend que le port soit sur écoute.
  unsigned long trame = 0;  // 32 bits = 4 octets  
  while (trame >> 8 != 0x1B3A73) {
    if (mySerial.available() > 0) {
      trame = (trame << 8) + readByte();
      //Serial.println(trame, HEX);
    }
  }
  return (byte) trame;
}
/*--------------------------------------------------------------------*/

byte Minitel::workingKeyboard() {  // Voir p.142
  // On récupère notamment les 3 bits de poids faibles suivants : C0 0 Eten
  // Eten : mode étendu (1 = actif)
  // C0   : codage en jeu C0 des touches de gestion du curseur (1 = actif)
  while (!mySerial);  // On attend que le port soit sur écoute.
  unsigned long trame = 0;  // 32 bits = 4 octets  
  while (trame != 0x1B3B7359) {
    if (mySerial.available() > 0) {
      trame = (trame << 8) + readByte();
      //Serial.println(trame, HEX);
    }
  }
  while (!mySerial.available()>0);  // Indispensable
  return readByte();
}
/*--------------------------------------------------------------------*/

unsigned long Minitel::getCursorXY() {  // Voir p.98
  // Demande
  writeByte(ESC);
  writeByte(0x61);
  // Réponse
  while (!mySerial);  // On attend que le port soit sur écoute.
  unsigned long trame = 0;  // 32 bits = 4 octets  
  while (trame >> 16 != US) {
    if (mySerial.available() > 0) {
      trame = (trame << 8) + readByte();
    }
  }
  return trame;
}
/*--------------------------------------------------------------------*/
