CC = gcc
LD = gcc
CFLAGS += -Wall -g -c
LDFLAGS += -Wall -g

all: Serveur1 Serveur2 Serveur3

Serveur1: Serveur1.o functions.o
	${LD} ${LDFLAGS} Serveur1.o functions.o -o Serveur1-PcqCestNotreProjet

Serveur1.o: Serveur1-PcqCestNotreProjet.c
	${CC} ${CFLAGS} Serveur1-PcqCestNotreProjet.c -o Serveur1.o

Serveur2: Serveur2.o functions.o
	${LD} ${LDFLAGS} Serveur2.o functions.o -o Serveur2-PcqCestNotreProjet

Serveur2.o: Serveur2-PcqCestNotreProjet.c
	${CC} ${CFLAGS} Serveur2-PcqCestNotreProjet.c -o Serveur2.o

Serveur3: Serveur3.o functions.o
	${LD} ${LDFLAGS} Serveur3.o functions.o -o Serveur3-PcqCestNotreProjet

Serveur3.o: Serveur3-PcqCestNotreProjet.c
	${CC} ${CFLAGS} Serveur3-PcqCestNotreProjet.c -o Serveur3.o

functions.o: functions.c functions.h
	${CC} ${CFLAGS} functions.c -o functions.o

clean:
	rm -f *~ *.o Serveur1-PcqCestNotreProjet Serveur2-PcqCestNotreProjet Serveur3-PcqCestNotreProjet image.jpg copy_nantes.jpg copy_moscow.jpg
