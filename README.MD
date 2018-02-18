# Minitel

Un simple exemple pour expliquer la connectivité d'un Arduino avec un Minitel.

## Connexion

Voici ce que doit avoir le Minitel pour pouvoir être utilisé avec un Arduino : une prise péri-informatique. Elle se cache derrière ce cache, au dos du Minitel, en bas à droite.

![cache prise péri-informatique](https://raw.githubusercontent.com/BorisFR/Minitel/master/GFX/minitel_connect_1.jpg)

Une fois le cache levé, voici ce qu'il y a derrière.

![prise péri-informatique](https://raw.githubusercontent.com/BorisFR/Minitel/master/GFX/minitel_connect_2.jpg)

Et enfin, voici la prise qu'il faut connecter : une fiche Din FM545.

C'est trouvable [ici chez Go Tronic](https://www.gotronic.fr/art-fiche-din-fm545-4747.htm) pour 0,70 €.  

![fiche Din FM545](https://raw.githubusercontent.com/BorisFR/Minitel/master/GFX/minitel_connect_3.jpg)

## Cablage

Il y a 3 fils à connecter.  
Ne tenez pas compte des fils blanc... c'est parce que j'ai utilisé un cable HDMI que j'ai recyclé et j'ai doublé les cables...  
Celui tout en haut (marron) et la Masse / Ground / GND. Il y a ensuite un fil de chaque côté correspondant au port Série de l'Arduino.
Précision : c'est du 5 V ! Donc c'est nickel avec un Arduino Uno ou Mega. Faites attention pour les autres types d'Arduino. C'est ok pour un Teensy 3.5 qui est en 3,3 V mais il est tolérant au 5V.

![fiche Din FM545](https://raw.githubusercontent.com/BorisFR/Minitel/master/GFX/minitel_connect_4.jpg)

Le fil bleu doit être connecté sur le RX du port Série de l'Arduino et le jaune sur le TX.

## Bibliothèque

Pour cet exemple, je l'ai incluse directement avec le programme. C'est une version légèrement modifiée. J'ai ouvert un "issue" sur le projet d'origine.
La bibliothèque utilisée se nomme [Minitel1B_Hard](https://github.com/eserandour/Minitel1B_Hard).
