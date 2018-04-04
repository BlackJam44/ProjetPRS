serveur2: serveur2-PcqCestNotreProjet.o fonctions.o
	gcc -Wall serveur2-PcqCestNotreProjet.o fonctions.o -o serveur2

serveur2-PcqCestNotreProjet.o: serveur2-PcqCestNotreProjet.c fonctions.h
	gcc -Wall -c serveur2-PcqCestNotreProjet.c -o serveur2-PcqCestNotreProjet.o

fonctions.o: fonctions.c fonctions.h
	gcc -Wall -c fonctions.c -o fonctions.o

clean:
	rm -f *.o serveur2
