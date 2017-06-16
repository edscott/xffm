HEADERS = stdout.diag.h
SOURCES = gtk_words_gui.c stdout.diag.c

tablas: tablas.c
	gcc -o tablas  -Wall -g -O2 -Wno-implicit-int tablas.c -lm `pkg-config dbh2 --cflags --libs` `pkg-config gtk+-3.0 --cflags --libs`

 
dumpsmd: dumpsmd.c
	gcc -o dumpsmd  -Wall -g -O2 -Wno-implicit-int dumpsmd.c smd_5.1.7.o -lm `pkg-config dbh2 --cflags --libs`

gtk-words: $(SOURCES) $(HEADERS) makefile
	gcc -o ../gtk-words  -Wall -g -O2 -Wno-implicit-int $(SOURCES)   `pkg-config gtk+-2.0 --cflags --libs`  -lm -lsmd_5.1.7

smd3_2_smd5+: smd3_2_smd5+.c
	cc -o smd3_2_smd5+ smd3_2_smd5+.c ../smd_3.2/smd_3_2.o -lm -lsmd_5.1.5

