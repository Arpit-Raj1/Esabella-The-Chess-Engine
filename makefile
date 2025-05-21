all:
	gcc -Ofast esabella.c -o esabella

debug:
	gcc esabella.c -o esabella

start:
	gcc -Ofast esabella.c -o esabella && ./esabella.exe

start-debug:
ifdef WIN64
	gcc esabella.c -o esabella && ./esabella.exe
else
	gcc esabella.c -o esabella && ./esabella
endif