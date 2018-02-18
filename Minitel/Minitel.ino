#include <Arduino.h>
// on ajoute la librairie qui va gérer le minitel
#include <Minitel1B_Hard.h>

// on déclare la variable qui va nous permettre de jouer avec le Minitel
// on indique sur quel port série la connexion est faite, ici Serial1
Minitel minitel(Serial1);

void setup() {
	Serial.begin(115200);
	// on cherche le Minitel et on récupère la vitesse de communication
	int speed = minitel.searchSpeed();
	Serial.println("Communication : " + String(speed) + " bauds");
	// on efface l'écran
	minitel.newScreen();
	// on met le clavier en minuscule
	minitel.smallMode();
	// on affiche un text à l'écran
	minitel.print("Bonjour le monde !");
}

void loop() {
	// lecture du clavier du Minitel
	unsigned long key = minitel.getKeyCode();
	if (key != 0) // une touche a été pressée
	{
		if (key == ENVOI)
			Serial.println("[ENVOI]");
		if (key == RETOUR)
			Serial.println("[RETOUR]");
		if (key == REPETITION)
			Serial.println("[REPETITION]");
		if (key == GUIDE)
			Serial.println("[GUIDE]");
		if (key == ANNULATION)
			Serial.println("[ANNULATION]");
		if (key == SOMMAIRE)
			Serial.println("[SOMMAIRE]");
		if (key == CORRECTION)
			Serial.println("[CORRECTION]");
		if (key == SUITE)
			Serial.println("[SUITE]");
		if (key == CONNEXION_FIN)
			Serial.println("[CONNEXION FIN]");
	}
}