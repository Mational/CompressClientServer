#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <math.h>

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
        printf("File doesn't exists!");
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
void write_to(int cfd, char* text, long int len)
{
    long int bytes = 0;
    while(1)
    {
        printf("[[%d]] Wysylam do klienta\n", cfd);
        bytes += write(cfd, text+bytes, len-bytes);
	if(bytes >= len)    break;
    }
    return;
}

/*
    Funkcja wysyłająca odpowiedz "ok" do klienta
    	cfd - deskryptor odbiorcy
    	stage - numer aktualnego etapu
*/
void send_ans(int cfd, char * stage)
{
    char answer[3] = "ok";
    printf("[[%d]] Etap %s - wysylanie potwierdzenia\n", cfd, stage);
    write_to(cfd, answer, 3);
    printf("[[%d]] Etap %s - wyslano potwierdzenie\n", cfd, stage);
}

/*
    Funkcja czytająca odpowiedź "ok" od klienta
        cfd - deskryptor odbiorcy
        stage - numer aktualnego etapu
*/
void read_ans(int cfd, char * stage)
{
    printf("[[%d]] Etap %s - pobieranie odpowiedzi...\n", cfd, stage);
    char buf[BUF_SIZE];
    int bytes = 0;
    while(1)
    {
        bytes += read(cfd, buf+bytes, BUF_SIZE-bytes);
        if(bytes >= 2)
            break;
    }
    printf("[[%d]] Etap %s - odpowiedz: %s\n", cfd, stage, buf);
    return;
}

int main(int argc, char** argv)
{
    /**** USTAWIENIA WSTĘPNE ****/
    socklen_t slt;
    int sfd, cfd, fdmax, rc, i, on = 1;
    struct sockaddr_in saddr, caddr;
    static struct timeval timeout;
    fd_set mask, rmask, wmask, fullReadMask, sizeReadMask, nameReadMask, countReadMask, fullWriteMask, sizeWriteMask;

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(1234);
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
    bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr));
    listen(sfd, 10);
    FD_ZERO(&mask);
    FD_ZERO(&rmask);
    FD_ZERO(&wmask);
    FD_ZERO(&fullReadMask);
    FD_ZERO(&sizeReadMask);
    FD_ZERO(&nameReadMask);
    FD_ZERO(&countReadMask);
    FD_ZERO(&fullWriteMask);
    FD_ZERO(&sizeWriteMask);
    fdmax = sfd;
    /**** USTAWIENIA WSTĘPNE ****/

    /**** ZMIENNE UŻYWANE W PROGRAMIE ****/
    int bytes[BUF_SIZE];
    int fileNum[BUF_SIZE];
    int mode[BUF_SIZE];
    int count[BUF_SIZE];
    char buf[BUF_SIZE][BUF_SIZE];
    char zip_command[BUF_SIZE][BUF_SIZE];
    char* fileName[BUF_SIZE];
    char* fileFullName[BUF_SIZE];
    long int fileLen[BUF_SIZE];
    long int  fileSize[BUF_SIZE];
    char* fileData[BUF_SIZE];
    FILE* file[BUF_SIZE];
    int lenFileSize[BUF_SIZE];
    char cFileSize[BUF_SIZE][BUF_SIZE];
    long int dotPos[BUF_SIZE];
    char char_i[BUF_SIZE][BUF_SIZE];
    /**** ZMIENNE UŻYWANE W PROGRAMIE ****/

    /**** PROGRAM SERWERA ****/
    while(1)
    {
    	printf("Rozpoczecie nowej petli glownej\n");
        FD_SET(sfd, &rmask);
        timeout.tv_sec = 5*60;
        timeout.tv_usec = 0;

        printf("fdmax: %d\n", fdmax);
        rc = select(fdmax+1, &rmask, &wmask, (fd_set*)0, &timeout);
        if(rc == 0)
        {
            printf("Timed out.\n");
            continue;
        }
        if(FD_ISSET(sfd, &rmask))
        {
            slt = sizeof(caddr);
            cfd = accept(sfd, (struct sockaddr*)&caddr, &slt);
            FD_SET(cfd, &rmask);
            if(cfd > fdmax)	fdmax = cfd;
        }
        for(i=sfd+1;i<=fdmax;i++)
        {
            printf("============ %d ============\n", i);
            if(FD_ISSET(i, &rmask)) 		printf("*** R ***\n");
            if(FD_ISSET(i, &wmask)) 		printf("*** W ***\n");
            if(FD_ISSET(i, &countReadMask)) 	printf("*** CRM ***\n");
            if(FD_ISSET(i, &nameReadMask)) 	printf("*** NRM ***\n");
            if(FD_ISSET(i, &sizeReadMask)) 	printf("*** SRM ***\n");
            if(FD_ISSET(i, &fullReadMask)) 	printf("*** FRM ***\n");
            if(FD_ISSET(i, &sizeWriteMask)) 	printf("*** SWM ***\n");
            if(FD_ISSET(i, &fullWriteMask)) 	printf("*** FWM ***\n");

            if(FD_ISSET(i, &wmask))
            {
                if(FD_ISSET(i, &fullWriteMask))
                {
                    // Etap 6 - wysyłanie całego pliku po kompresji i odbieranie odpowiedzi

                    /* Tworzenie i pobieranie danych z pliku po kompresji */
                    file[i] = open_file(fileFullName[i], "rb");
                    fileData[i] = malloc(fileSize[i]);
                    memset(fileData[i], 0, fileSize[i]);
                    for(int j = 0; j < fileSize[i]; j++)    fileData[i][j] = fgetc(file[i]);
                    fclose(file[i]);
                    /* Tworzenie i pobieranie danych z pliku po kompresji */

                    /* Wysylanie pliku po kompresji do klienta */
                    printf("[[%d]] Etap 6 - wysylanie calego pliku zip...\n", i);
                    write_to(i, fileData[i], fileSize[i]);
                    printf("[[%d]] Etap 6 - wyslano caly plik zip.\n", i);
                    /* Wysylanie pliku po kompresji do klienta */

                    /* Usuwanie paczki zip z serwera jesli tryb jest 2 */
                    if(mode[i] == 2)
                    {
                    	/* Usuwanie pliku do kompresji */
		    	memset(buf[i], 0, sizeof buf[i]);			// czyszczenie bufora
		    	strcat(buf[i], "rm ");
		    	strcat(buf[i], fileFullName[i]);
		    	system((const char*)buf[i]);				// usunięcie pliku zip
		    	memset(buf[i], 0, sizeof buf[i]);			// czyszczenie bufora
		    	/* Usuwanie pliku do kompresji */
                    }
                    /* Usuwanie paczki zip z serwera jesli tryb jest 2 */

                    printf("[[%d]] Zakonczylem prace z klientem.\n\n", i);

                    /* Czyszczenie */
                    if(fileFullName[i])	free(fileFullName[i]);
                    if(fileData[i])	free(fileData[i]);
                    /* Czyszczenie */

                    read_ans(i, "6");						// odbieranie odpowiedzi

                    FD_CLR(i, &wmask);						// deaktywacja maski wmask
                    FD_CLR(i, &fullWriteMask);					// deaktywacja maski fullWriteMask

                    close(i);         						// rozłączanie z klientem
                }
                else if(FD_ISSET(i, &sizeWriteMask))
                {
                    // Etap 5 - wysyłanie rozmiaru pliku po kompresji i odbieranie odpowiedzi

                    /* Odczytywanie rozmiaru pliku */
                    file[i] = open_file(fileFullName[i], "rb");
		    fseek(file[i], 0, SEEK_END);
		    fileSize[i] = ftell(file[i]);
                    fclose(file[i]);
                    /* Odczytywanie rozmiaru pliku */

                    lenFileSize[i] = (int)(floor(log10(fileSize[i])))+2;	// długość rozmiaru pliku (+1 (log); +1 ('?'))

		    /* Konwersja rozmiaru pliku do string */
		    memset(cFileSize[i], 0, sizeof cFileSize[i]);		// czyszczenie cFileSize z artefaktów
		    sprintf(cFileSize[i],"%ld", fileSize[i]);			// konwersja
		    cFileSize[i][lenFileSize[i]-1] = '?';			// dodawanie na koncu znaku '?'
		    /* Konwersja rozmiaru pliku do string */

		    /* Wysylanie rozmiaru pliku po kompresji do klienta */
                    printf("[[%d]] Etap 5 - wysylanie rozmiaru pliku zip...\n", i);
		    write_to(i, cFileSize[i], lenFileSize[i]+1);
		    printf("[[%d]] Etap 5 - wyslano rozmiar pliku zip.\n", i);
		    /* Wysylanie rozmiaru pliku po kompresji do klienta */

		    /* Informacje */
		    printf("[[%d]] cFileSize: %s\n", i, cFileSize[i]);
		    printf("[[%d]] Length of size of file in char* form: %d\n", i, lenFileSize[i]);
                    printf("[[%d]] Char* form of size after compression %s\n", i, cFileSize[i]);
                    /* Informacje */

                    read_ans(i, "5");						// odbieranie odpowiedzi

		    FD_SET(i, &fullWriteMask);  				// aktywacja maski fullWriteMask
                    FD_CLR(i, &sizeWriteMask);					// deaktywacja maski sizeWriteMask
                }
                else
                {
                    // Etap 4.5 - wysyłanie pliku nazwy pliku po kompresji i odebranie odpowiedzi

		    /* Tworzenie i uruchomienie polecenia do kompresji pliku */
		    printf("[[%d]] Etap 4.5 - Tworzenie polecenia do kompresji pliku...\n", i);
		    memset(buf[i], 0, sizeof buf[i]);
		    strcat(buf[i], "rm -f package");
		    strcat(buf[i], char_i[i]);
		    strcat(buf[i], ".zip");
                    system((const char*)buf[i]);					// usuwanie wcześniejszego zipa
                    system((const char*)zip_command[i]);				// uruchamianie polecenia kompresji
                    printf("[[%d]] Etap 4.5 - Utworzono polecenie do kompresji pliku.\n", i);
                    /* Tworzenie i uruchomienie polecenia do kompresji pliku */

		    if(fileFullName[i])	free(fileFullName[i]);				// czyszczenie pamięci dla nazwy
		    memset(buf[i], 0, sizeof buf[i]);					// czyszczenie bufora

		    if(mode[i]==1)
		    {
		    	/* konczenie pracy z klientem jezeli tryb 1 */
		    	close(i);
		    	FD_CLR(i, &wmask);
		    	/* konczenie pracy z klientem jezeli tryb 1 */
		    }
		    else
		    {
		        /* Tworzenie nazwy pliku z rozszerzeniem po kompresji */
		        strcat(buf[i], "package");
		        strcat(buf[i], char_i[i]);
		        strcat(buf[i], ".zip");
		        printf("[[%d]] buf: %s\n", i, buf[i]);
                        /* Tworzenie nazwy pliku z rozszerzeniem po kompresji */

                        /* Przypisanie nazwy pliku do zmiennej */
                        fileFullName[i] = malloc(strlen(buf[i]));
                        memset(fileFullName[i], 0, strlen(buf[i]));			// czyszczenie fileFullName z artefaktów
                        strcpy(fileFullName[i], buf[i]);
                        printf("[[%d]] fileFullName: %s\n", i, fileFullName[i]);
                        /* Przypisanie nazwy pliku do zmiennej */

		        printf("[[%d]] Name of zip file: %s\n", i, fileFullName[i]);

                        FD_SET(i, &sizeWriteMask);					// aktywacja maski sizeWriteMask
                    }
                }
                if(i == fdmax)
                {
		    printf("[[%d]] Stan sizeWriteMask: %d\n", i, !FD_ISSET(fdmax, &sizeWriteMask));
		    printf("[[%d]] Stan fullWriteMask: %d\n\n", i, !FD_ISSET(fdmax, &fullWriteMask));
                    while(fdmax > sfd && !FD_ISSET(fdmax, &rmask) &&
                    		!FD_ISSET(fdmax, &fullWriteMask) && !FD_ISSET(fdmax, &sizeWriteMask))
                        fdmax -= 1;
                }
            }
            else if(FD_ISSET(i, &rmask))
            {
		if(FD_ISSET(i, &fullReadMask))
		{
		    // Etap 4 - odczytanie całego pliku i wysłanie odpowiedzi

		    bytes[i] = 0;							// czyszczenie zmiennej pomocniczej
		    fileData[i] = malloc(fileSize[i]);					// alokacji pamięci dla pliku
		    memset(fileData[i], 0, fileSize[i]);

		    /* pełne odczytanie całego pliku */
		    printf("[[%d]] Etap 4 - odczytywanie calego pliku...\n", i);
		    while(1)
		    {
			bytes[i] += read(i, fileData[i]+bytes[i], fileSize[i]-bytes[i]);
			if(bytes[i]>=fileSize[i])
			{
			    printf("[[%d]] Etap 4 - zakończono odczytywanie pliku.\n", i);
			    break;
			}
		    }
		    /* pełne odczytanie całego pliku */

		    /* utworzenie pliku i zapisanie w nim danych */
                    printf("[[%d]] Etap 4 - Tworzenie pliku do kompresji...\n", i);
                    file[i] = open_file(fileFullName[i], "w+");
            	    for (long int j = 0; j < fileSize[i]; j++)	fputc((int)fileData[i][j], file[i]);
            	    fclose(file[i]);
            	    if(fileData[i])	free(fileData[i]);
            	    printf("[[%d]] Etap 4 - Utworzono plik do kompresji.\n", i);
            	    /* utworzenie pliku i zapisanie w nim danych */

		    strcat(zip_command[i], fileFullName[i]);

		    /* Odczytywanie kolejnego pliku albo przejście do wysyłania */
		    if(count[i]<fileNum[i])
		    {
		    	strcat(zip_command[i], " ");
		    	FD_SET(i, &nameReadMask);					// aktywacja maski nameReadMask
		    }
		    else
		    {
		    	FD_SET(i, &wmask);						// aktywacja maski wmask
		    	FD_CLR(i, &rmask);						// deaktywacja maski rmask
		    }
		    /* Odczytywanie kolejnego pliku albo przejście do wysyłania */

		    /* Informacje */
		    printf("[[%d]] zip_command: %s\n", i, zip_command[i]);
		    printf("[[%d]] count: %d\n", i, count[i]);
		    /* Informacje */

		    send_ans(i, "4");							// wysyłanie odpowiedzi

		    FD_CLR(i, &fullReadMask);						// deaktywacja maski fullReadMask
		}
		else if(FD_ISSET(i, &sizeReadMask))
		{
		    // Etap 3 - odbieranie rozmiaru pliku do kompresji i wysyłanie odpowiedzi

		    bytes[i] = 0;						// czyszczenie zmiennej pomocniczej
		    memset(buf[i], 0, sizeof buf[i]);				// czyszczenie bufora

		    /* pełny odczyt rozmiaru pliku */
		    printf("[[%d]] Etap 3 -odczytywanie rozmiaru...\n", i);
		    while(1)
		    {
			bytes[i] += read(i, buf[i]+bytes[i], BUF_SIZE-bytes[i]);
			if(buf[i][bytes[i]-2] == '?')
		 	{
			    printf("[[%d]] Etap 3 - zakonczono odczyt rozmiaru.\n", i);
			    break;
			}
		    }
		    /* pełny odczyt rozmiaru pliku */

		    fileSize[i] = atoi(buf[i]);					// zamiana rozmiaru na long int

		    printf("[[%d]] Size of file: %ld\n", i, fileSize[i]);

		    send_ans(i, "3");						// wysyłanie odpowiedzi

		    FD_SET(i, &fullReadMask);					// aktywacja maski fullReadMask
		    FD_CLR(i, &sizeReadMask);					// deaktywacja maski sizeReadMask
		}
		else if(FD_ISSET(i, &nameReadMask))
       		{
       		    // Etap 2 - odbieranie nazwy pliku do kompresji i wysyłanie odpowiedzi

       		    bytes[i] = 0;						// czyszczenie zmiennej pomocniczej
		    memset(buf[i], 0, sizeof buf[i]);				// czyszczenie bufora

		    /* pełne odczytanie nazwy pliku do kompresji */
		    printf("[[%d]] Etap 2 - odczytywanie nazwy...\n", i);
		    while(1)
		    {
			bytes[i] += read(i, buf[i]+bytes[i], BUF_SIZE-bytes[i]);
			if(buf[i][bytes[i]-2] == '?')
			{
			    printf("[[%d]] Etap 2 - zakonczono odczyt nazwy.\n", i);
			    break;
			}
		    }
		    printf("[[%d]] odczytano %d bytes\n", i, bytes[i]);
		    printf("[[%d]] Buf %s\n", i, buf[i]);
		    /* pełne odczytanie nazwy pliku do kompresji */

		    /* odczyt długości nazwy pliku i pozycji '.' */
		    fileLen[i] = strlen(buf[i])-1;
		    printf("[[%d]] FileLen: %ld\n", i, fileLen[i]);
		    dotPos[i] = 0;
		    while(1)
		    {
			if(buf[i][dotPos[i]] == '.')    break;
			dotPos[i]++;
		    }
		    /* odczyt długości nazwy pliku i pozycji '.' */

		    count[i]++;

		    /* przepisywanie nazw pliku */
		    fileFullName[i] = malloc(fileLen[i]);			// alokacja pamięci dla nazwy pliku
		    memset(fileFullName[i], 0, fileLen[i]);			// czyszczenie fileFullName z artefaktów
		    fileName[i] = malloc(dotPos[i]);				// alokacja pamięci dla nazwy bez roz.
		    memset(fileName[i], 0, dotPos[i]);				// czyszczenie fileName z artefaktów
		    for(long int k=0;k<fileLen[i];k++)
		    {
		    	fileFullName[i][k]=buf[i][k];				// przepisanie nazwy pliku
		    	if(k<dotPos[i])
		    	{
		    	    fileName[i][k] = buf[i][k];				// przepisywanie nazwy bez rozszerzenia
		    	    fileName[i][k+1] = '\0';
		    	}
		    }
		    fileFullName[i][fileLen[i]] = '\0';
		    printf("[[%d]] fileName: %s\n", i, fileName[i]);
		    /* przepisywanie nazw pliku */

		    send_ans(i, "2");						// wysyłanie odpowiedzi

		    printf("[[%d]] Name of file: %s\n", i, fileFullName[i]);

		    FD_SET(i, &sizeReadMask);					// aktywacja maski sizeReadMask
		    FD_CLR(i, &nameReadMask);					// deaktywacja maski nameReadMask
		}
		else if(FD_ISSET(i, &countReadMask))
		{
		    // Etap 1 - odczytywanie liczby plików do kompresji i wysyłanie odpowiedzi

		    bytes[i] = 0;						// czyszczenie zmiennej pomocniczej
		    memset(buf[i], 0, sizeof buf[i]);				// czyszczenie bufora

		    /* pełne odczytanie liczby plików do kompresji */
		    printf("[[%d]] Etap 1 - odczytywanie liczby plików...\n", i);
		    while(1)
		    {
			bytes[i] += read(i, buf[i]+bytes[i], BUF_SIZE-bytes[i]);
			if(buf[i][bytes[i]-2] == '?')
			{
			    printf("[[%d]] Etap 1 - zakonczono odczyt liczby plików.\n", i);
			    break;
			}
		    }
		    printf("[[%d]] odczytano %d bytes\n", i, bytes[i]);
		    printf("[[%d]] Buf %s\n", i, buf[i]);
		    /* pełne odczytanie liczby plików do kompresji */

		    fileNum[i] = atoi(buf[i]);					// zamiana rozmiaru na long int
		    count[i] = 0;
		    printf("[[%d]] Numer of files: %d\n", i, fileNum[i]);

		    memset(char_i[i], 0, sizeof char_i[i]);
		    sprintf(char_i[i],"%d", i);

		    strcat(zip_command[i], "zip -m package");			// początek tworzenia komendy zip
		    strcat(zip_command[i], char_i[i]);
		    strcat(zip_command[i], ".zip ");

		    send_ans(i, "1");						// wysyłanie odpowiedzi

		    FD_SET(i, &nameReadMask);					// aktywacja maski nameReadMask
		    FD_CLR(i, &countReadMask);					// deaktywacja maski countReadMask
		}
		else
		{
		    // Etap 0 - odczytywanie trybu pracy serwera i wysyłanie odpowiedzi

		    bytes[i] = 0;						// czyszczenie zmiennej pomocniczej
		    memset(buf[i], 0, sizeof buf[i]);				// czyszczenie bufora

		    /* odczytanie trybu pracy serwera */
		    printf("\n[[%d]] Etap 0 - odczytywanie trybu pracy serwera...\n", i);
		    while(1)
		    {
			bytes[i] += read(i, buf[i]+bytes[i], BUF_SIZE-bytes[i]);
			if(bytes[i]>=2)
			{
			    printf("[[%d]] Etap 0 - odczytano tryb pracy serwera.\n", i);
			    break;
			}
		    }
		    /* odczytanie trybu pracy serwera */

		    mode[i] = atoi(buf[i]);
		    printf("[[%d]] Server work mode: %d\n", i, mode[i]);

		    send_ans(i, "0");						// wysyłanie odpowiedzi

		    FD_SET(i, &countReadMask);					// aktywacja maski countReadMask
		}
                if(i == fdmax)
		{
                    while(fdmax > sfd && !FD_ISSET(fdmax, &wmask) &&
                    	  !FD_ISSET(fdmax, &fullReadMask) && !FD_ISSET(fdmax, &sizeReadMask) &&
                    	  !FD_ISSET(fdmax, &nameReadMask) && !FD_ISSET(fdmax, &countReadMask))
		         fdmax -= 1;
                }
		printf("[[%d]] Zakonczylem jedna akcje w read\n\n", i);
	    }

	    /* Uzupełnienie rmask i wmask */
	    if(FD_ISSET(i, &fullReadMask) || FD_ISSET(i, &sizeReadMask) ||
    	       FD_ISSET(i, &nameReadMask) || FD_ISSET(i, &countReadMask))
    	    	FD_SET(i, &rmask);

    	    if(FD_ISSET(i, &fullWriteMask) || FD_ISSET(i, &sizeWriteMask))
    	    	FD_SET(i, &wmask);
    	    /* Uzupełnienie rmask i wmask */
        }
    }
    close(sfd);
    return EXIT_SUCCESS;
    /**** PROGRAM SERWERA ****/
}


