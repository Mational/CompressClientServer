#include <unistd.h>
#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <libgen.h>

#define BUF_SIZE 1024

/*
    Funkcja otwierająca plik
    	path - ścieżka do pliku
    	mode - tryb w jakim plik ma zostac otwarty
*/
FILE* open_file(char* path, char* mode)
{
    FILE* fd = fopen(path, mode);
    if(fd == NULL)
    {
        printf("File doesn't exists!\n");
        exit(1);
    }
    return fd;
}

/*
    Funkcja wysylajaca dane we fragmentach
	sfd - deskryptor odbiorcy
	text - dane do przeslania
	len - dlugosc danych
*/
void write_to(int sfd, char* text, long int len)
{
    long int bytes = 0;
    while(1)
    {
	printf("Wysylam na serwer\n");
        bytes += send(sfd, text+bytes, len-bytes, 0);
        if(bytes>=len)    break;
    }
    printf("wysłano %ld bytes\n", bytes);
    return;
}

/*
    Funkcja czytająca odpowiedź "ok" od serwera
        sfd - deskryptor odbiorcy
        stage - numer aktualnego etapu
*/
void read_ans(int sfd, char * stage)
{
    printf("Etap %s - pobieranie odpowiedzi...\n", stage);
    char buf[BUF_SIZE];
    int bytes = 0;
    while(1)
    {
        bytes += recv(sfd, buf+bytes, BUF_SIZE-bytes, 0);
        if(bytes>=2)
            break;
    }
    printf("Etap %s - odpowiedz: %s\n\n", stage, buf);
    return;
}

/*
    Funkcja wysyłająca odpowiedz "ok" do serwera
    	sfd - deskryptor odbiorcy
    	stage - numer aktualnego etapu
*/
void send_ans(int sfd, char * stage)
{
    char answer[3] = "ok";
    printf("Etap %s - wysylanie potwierdzenia\n", stage);
    send(sfd, answer, 3, 0);
    printf("Etap %s - wyslano potwierdzenie\n", stage);
}

int main(int argc, char** argv)
{
    /*** Łączenie z serwerem ***/
	WSADATA wsaData;
    SOCKET sfd;
    // Server/receiver address
    SOCKADDR_IN          ServerAddr;
    // Server/receiver port to connect to
    unsigned int         Port = atoi(argv[2]);
    int  RetCode;
    // Initialize Winsock version 2.2
    WSAStartup(MAKEWORD(2,2), &wsaData);
    printf("Client: Winsock DLL status is %s.\n", wsaData.szSystemStatus);
    // Create a new socket to make a client connection.
    // AF_INET = 2, The Internet Protocol version 4 (IPv4) address family, TCP protocol
    sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(sfd == INVALID_SOCKET)
    {
        printf("Client: socket() failed! Error code: %d\n", WSAGetLastError());
        // Do the clean up
        WSACleanup();
        // Exit with error
        return -1;
    }
    else
        printf("Client: socket() is OK!\n");

    // IPv4
    ServerAddr.sin_family = AF_INET;

    // Port no.
    ServerAddr.sin_port = htons(Port);

    // The IP address
    ServerAddr.sin_addr.s_addr = inet_addr(argv[1]);

    // Make a connection to the server with socket sfd.
    RetCode = connect(sfd, (SOCKADDR *) &ServerAddr, sizeof(ServerAddr));

    if(RetCode != 0)
    {
        printf("Client: connect() failed! Error code: %d\n", WSAGetLastError());
        // Close the socket
        closesocket(sfd);
        // Do the clean up
        WSACleanup();
        // Exit with error
        sleep(4);
        return -1;
    }
    else
    {
        printf("Client: connect() is OK, got connected...\n");
        printf("Client: Ready for sending and/or receiving data...\n");
    }
    /*** Łączenie z serwerem ***/


    /**** PROGRAM KLIENTA ****/

    // Etap 0 - wysyłanie trybu pracy serwera i odbieranie potwierdzenia

    /* Informacje */
    printf("Server work mode: %s\n", argv[3]);
    printf("Sciezka zapisu pliku zip: %s\n", argv[4]);
    printf("Liczba plików do kompresji: %s\n", argv[5]);
    printf("argv[6]: %s\n", argv[6]);
    printf("argv[7]: %s\n", argv[7]);
    /* Informacje */

    /* wysylanie trybu pracy serwera */
    printf("Etap 0 - wysyłanie trybu pracy serwera...\n");
    write_to(sfd, (char*) argv[3], 2);
    printf("Etap 0 - wysłano tryb pracy serwera.\n");
    /* wysylanie trybu pracy serwera */

    read_ans(sfd, "0");					// odczytanie odpowiedzi od serwera dla etapu 0.

    // Etap 1 - wysyłanie liczby plików do kompresji i odbieranie potwierdzenia

    long int fileNum = atoi(argv[5]);
    int lenFileSize = (int)(floor(log10(fileNum)))+2;	// liczba plików (+1, bo log; +1, bo '?' na końcu)
    char cFileNum[lenFileSize+1];
    memset(cFileNum, 0, sizeof cFileNum);		// czyszczenie cFileNum z artefaktów
    sprintf(cFileNum, "%ld", fileNum);			// konwersja liczby plików na string
    cFileNum[lenFileSize-1] = '?';			// dodanie na koniec liczby plików '?'

    printf("Num of files: %s\n", cFileNum);

    /* wysylanie liczby plików do kompresji */
    printf("Etap 1 - wysylanie liczby plików do kompresji...\n");
    write_to(sfd, cFileNum, lenFileSize+1);
    printf("Etap 1 - wyslano liczby plików do kompresji.\n");
    /* wysylanie liczby plików do kompresji */

    read_ans(sfd, "1");					// odczytanie odpowiedzi od serwera dla etapu 1.

    long int fileSize;
    char * fileName="\0";
    long int fileLen;
    char * fileData;
    FILE * file;

    for(long int count=fileNum-1;count>=0;count--)
    {
    	// Etap 2 - wysyłanie nazwy pliku i odbieranie potwierdzenia.
    	printf("Name of file %ld: %s\n", count+1, argv[6+count]);
    	file = open_file(argv[6+count], "rb");	// otwarcie pliku

    	fseek(file, 0, SEEK_END);		// przejście na koniec pliku
    	fileSize = ftell(file);			// ftell zwraca aktualną pozycję w pliku (w tym przypadku rozmiar pliku)

    	fileName = basename(argv[6+count]);	// tutaj będzie przypisana sama nazwa pliku bez ścieżki
    	strcat(fileName, "?");		        // dopisanie do nazwy pliku '?'

    	fileLen = strlen(fileName);		// sprawdzanie dlugosci nazwy pliku

    	/* Informacje o pliku */
    	printf("\n\n");
    	printf("Size of file to compression: %ld\n", fileSize);
    	printf("Name of file to compression: %s\n", fileName);
    	printf("Length of name: %ld\n", fileLen);
    	/* Informacje o pliku */

    	/* wysylanie nazwy pliku do kompresji */
    	printf("Etap 2 - wysylanie nazwy pliku do kompresji...\n");
    	write_to(sfd, fileName, fileLen+1);
    	printf("Etap 2 - wyslano nazwe pliku do kompresji.\n");
    	/* wysylanie nazwy pliku do kompresji */

    	read_ans(sfd, "2");			// odczytanie odpowiedzi od serwera dla etapu 2.

  	sleep(4);

    	// Etap 3 - wysylanie rozmiaru pliku do kompresji i odebranie potwierdzenia

    	fseek(file, 0, SEEK_SET);			// powrót na początek pliku

    	lenFileSize = (int)(floor(log10(fileSize)))+2;	// długość rozmiaru pliku (+1, bo log; +1, bo '?' na końcu)

    	char cFileSize[lenFileSize+1];
    	memset(cFileSize, 0, sizeof cFileSize);		// czyszczenie cFileSize z artefaktów
    	printf("tesst: %s\n", cFileSize);
    	sprintf(cFileSize, "%ld", fileSize);		// konwersja rozmiaru pliku na string
    	cFileSize[lenFileSize-1] = '?';			// dodanie na koniec rozmiaru pliku '?'

    	/* Informacje o pliku */
    	printf("Length of char* form of size of file: %d\n", lenFileSize);
    	printf("Char* form of size of file: %s\n", cFileSize);
    	/* Informacje o pliku */

    	/* wysylanie rozmiaru pliku do kompresji */
    	printf("Etap 3 - wysylanie rozmiaru...\n");
    	write_to(sfd, cFileSize, lenFileSize+1);
    	printf("Etap 3 - wyslano rozmiar.\n");
    	/* wysylanie rozmiaru pliku do kompresji */

    	read_ans(sfd, "3");					// odczytanie odpowiedzi od serwera dla etapu 3.

    	// Etap 4 - wysylanie calego pliku do kompresji i odebranie odpowiedzi

    	fileData = malloc(fileSize);					// alokacja pamięci dla danych z pliku do kompresji
    	for(int i = 0; i < fileSize; i++)    fileData[i] = fgetc(file);	// odczytywanie zawartości pliku do kompresji
    	fclose(file);							// zamykanie pliku do kompresji

    	/* wysylanie calego pliku do kompresji */
    	printf("Etap 4 - wysylanie pliku...\n");
    	write_to(sfd, fileData, fileSize);
    	printf("Etap 4 - wyslano plik.\n");
    	/* wysylanie calego pliku do kompresji */

    	read_ans(sfd, "4");						// odczytanie odpowiedzi od serwera dla etapu 4.

    	if(fileData)	free(fileData);					// zwalnianie miejsca dla danych z pliku
    }

    if(atoi(argv[3])!=1)
    {
    	char buf[BUF_SIZE] = {0};				// bufor na rozmiar pliku po kompresji
    	int bytes =0;						// zmienna pomagająca w pełnym odczycie danych

    	// Etap 5 - odbieranie rozmiaru pliku po kompresji i wysyłanie odpowiedzi

    	memset(buf, 0, sizeof buf);				// czyszczenie bufora przed odczytem rozmiaru
    	bytes = 0;						// zerowanie zmiennej pomocniczej

    	/* pełny odczyt rozmiary pliku */
    	printf("\nEtap 5 - pobieranie rozmiaru pliku zip...\n");
    	while(1){
            bytes += recv(sfd, buf+bytes, BUF_SIZE-bytes, 0);
            if(buf[bytes-2] == '?')
            {
            	printf("Etap 5 - pobrano rozmiar pliku zip.\n");
            	break;
            }
    	}
    	/* pełny odczyt rozmiary pliku */

    	fileSize = atoi(buf);					// zamiana rozmiaru pliku z formatu char* na long int

    	send_ans(sfd, "5");					// wysyłanie odpowiedzi

    	/* Informacje o pliku */
    	printf("Size of zip file: %ld\n", fileSize);
    	/* Informacje o pliku */

    	// Etap 6 - odbieranie całego pliku po kompresji i wysyłanie odpowiedzi

    	fileData = malloc(fileSize);						// alokacja pamięci dla pliku po kompresji
    	memset(fileData, 0, fileSize);						// czyszczenie fileData z artefaktów
    	bytes = 0;								// zerowanie zmiennej pomocniczej

    	/* pełny odczyt całego pliku */
    	printf("\nEtap 6 - pobieranie calego pliku zip...\n");
    	while(1)
    	{
	    bytes += recv(sfd, fileData+bytes, fileSize-bytes, 0);
	    if(bytes>=fileSize)
	    {
	    	printf("Etap 6 - zakończono pobieranie calego pliku zip.\n");
	    	break;
	    }
    	}
    	/* pełny odczyt całego pliku */

    	file = open_file(argv[4], "wb+");					// tworzenie uchwytu dla pliku po kompresji
    	for (long int j=0; j<fileSize; j++)	fputc((int)fileData[j], file);	// uzupelnianie pliku danymi
    	fclose(file);								// zamykanie pliku

    	send_ans(sfd, "6");							// wysyłanie odpowiedzi

    	/* czyszczenie zmiennych */
    	if(fileData)	free(fileData);
    	/* czyszczenie zmiennych */
    }


    closesocket(sfd);									// rozłączenie z serwerem
    WSACleanup();
    printf("\nZakonczylem prace z serwerem.\n");
    /**** PROGRAM KLIENTA ****/
    return 0;
}
