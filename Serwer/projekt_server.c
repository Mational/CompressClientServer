#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h>
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
        printf("Wysylam do klienta\n");  
        bytes += write(cfd, text+bytes, len-bytes);
	if(bytes >= len)    break;
    }
    return;
}

/*
    Funkcja wysyłająca odpowiedz do serwera ('ok')
    	sfd - deskryptor odbiorcy
    	stage - numer aktualnego etapu
*/
void send_ans(int cfd, char * stage)
{
    char answer[3] = "ok";
    printf("Etap %s - wysylanie potwierdzenia\n", stage);
    write_to(cfd, answer, 3);
    printf("Etap %s - wyslano potwierdzenie\n", stage);
}

/*
    Funkcja czytająca odpowiedź od serwera ('ok')
        cfd - deskryptor odbiorcy
        stage - numer aktualnego etapu
*/
void read_ans(int cfd, char * stage)
{
    printf("Etap %s - pobieranie odpowiedzi...\n", stage);
    char buf[BUF_SIZE];
    int bytes = 0;
    while(1)
    {
        bytes += read(cfd, buf+bytes, BUF_SIZE-bytes);
        if(bytes>=2)
            break;
    }
    printf("Etap %s - odpowiedz: %s\n", stage, buf);
    return;
}

int main(int argc, char** argv)
{
    /**** USTAWIENIA WSTĘPNE ****/
    socklen_t slt;
    int sfd, cfd, fdmax, fds, rc, i, on = 1;
    struct sockaddr_in saddr, caddr;
    static struct timeval timeout;
    fd_set mask, rmask, wmask, fullReadMask, sizeReadMask, nameReadMask, fullWriteMask, sizeWriteMask;		

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
    FD_ZERO(&fullWriteMask);
    FD_ZERO(&sizeWriteMask);
    fdmax = sfd;
    /**** USTAWIENIA WSTĘPNE ****/
    
    /**** PROGRAM SERWERA ****/
    while(1)
    {
        FD_SET(sfd, &rmask);
        timeout.tv_sec = 5*60;
        timeout.tv_usec = 0;
        rc = select(fdmax+1, &rmask, &wmask, (fd_set*)0, &timeout);
        if(rc == 0)
        {
            printf("Timed out.\n");
            continue;
        }
        fds = rc;
        if(FD_ISSET(sfd, &rmask))
        {
            slt = sizeof(caddr);
            cfd = accept(sfd, (struct sockaddr*)&caddr, &slt);
            FD_SET(cfd, &rmask);
            if(cfd > fdmax) fdmax = cfd;
        }
        for(i=sfd+1; i <= fdmax && fds > 0; i++)
        {
            printf("hej\n");
            int bytes = 0;
            int mode;
            char buf[BUF_SIZE];			
	    char* fileName;
	    char* fileFullName;
	    long int fileLen;
	    long int  fileSize;
	    char* fileData;     	    
            FILE *file;
            if(FD_ISSET(i, &wmask))
            {
                if(FD_ISSET(i, &fullWriteMask))
                {
                    // Etap 6 - wysyłanie całego pliku po kompresji i odbieranie odpowiedzi

                    /* Tworzenie i pobieranie danych z pliku po kompresji */
                    file = open_file(fileFullName, "rb");
                    fileData  = malloc(fileSize);   
                    memset(fileData, 0, fileSize);  
                    for(int j = 0; j < fileSize; j++)
                    	fileData[j] = fgetc(file);
                    fclose(file);
                    /* Tworzenie i pobieranie danych z pliku po kompresji */
                    
                    printf("Etap 6 - wysylanie calego pliku zip...\n");
                    write_to(i, fileData, fileSize);
                    printf("Etap 6 - wyslano caly plik zip.\n");
                    
                    if(mode==2)
                    {
                    	/* Usuwanie pliku do kompresji */
		    	memset(buf, 0, sizeof buf);						// czyszczenie bufora
		    	strcat(buf, "rm ");
		    	strcat(buf, fileFullName);
		    	system((const char*)buf);						// usunięcie pliku zip
		    	/* Usuwanie pliku do kompresji */
		    	memset(buf, 0, sizeof buf);
                    }
                    
                    read_ans(i, "6");						// odbieranie odpowiedzi
                    
                    close(i);         						// rozłączanie z klientem
                     
                    printf("Zakonczylem prace z klientem.\n\n"); 
                    
                    /* czyszczenie */
                    if(fileFullName)	free(fileFullName);
                    //if(fileName)	free(fileName);
                    if(fileData)	free(fileData);
                    /* czyszczenie */
      
                    FD_CLR(i, &wmask);						// deaktywacja maski wmask
                    FD_CLR(i, &fullWriteMask);					// deaktywacja maski fullWriteMask    
                }
                else if(FD_ISSET(i, &sizeWriteMask))
                {
                    // Etap 5 - wysyłanie rozmiaru pliku po kompresji i odbieranie odpowiedzi
                    
                    /* Pobieranie rozmiaru pliku */
                    file = open_file(fileFullName, "rb");
		    fseek(file, 0, SEEK_END);
		    fileSize = ftell(file);
                    fclose(file);
                    /* Pobieranie rozmiaru pliku */
                    
                    int lenFileSize = (int)(floor(log10(fileSize)))+2;		// długość rozmiaru pliku (+1 (log); +1 ('?'))
		    
		    /* Konwersja rozmiaru pliku do string */
		    char cFileSize[lenFileSize+1];
		    memset(cFileSize, 0, sizeof cFileSize);			// czyszczenie cFileSize z artefaktów
		    sprintf(cFileSize,"%ld", fileSize);
		    cFileSize[lenFileSize-1] = '?';
		    /* Konwersja rozmiaru pliku do string */
		    
                    printf("Etap 5 - wysylanie rozmiaru pliku zip...\n");
		    write_to(i, cFileSize, lenFileSize+1);
		    printf("Etap 5 - wyslano rozmiar pliku zip.\n");
		    
		    read_ans(i, "5");						// odbieranie odpowiedzi
		    
		    printf("Length of size of file in char* form: %d\n", lenFileSize);	
                    printf("Char* form of size after compression %s\n", cFileSize);
                    
                    FD_SET(i, &fullWriteMask);  				// aktywacja maski fullWriteMask  
                    FD_CLR(i, &sizeWriteMask);					// deaktywacja maski sizeWriteMask
                }
                else
                {
                    // Etap 4 - wysyłanie pliku nazwy pliku po kompresji i odebranie odpowiedzi
                    
                    /* utworzenie pliku i zapisanie w nim danych */
                    printf("Etap 4 - Tworzenie pliku do kompresji...\n");
                    file = open_file(fileFullName, "w+");
            	    for (long int j = 0; j < fileSize; j++)
			fputc((int)fileData[j], file);
            	    fclose(file);
            	    if(fileData)	free(fileData);
            	    printf("Etap 4 - Utworzono plik do kompresji.\n");
            	    /* utworzenie pliku i zapisanie w nim danych */

		    /* Tworzenie i uruchomienie polecenia do kompresji pliku */
		    printf("Etap 4 - Tworzenie polecenia do kompresji pliku...\n");
		    memset(buf, 0, sizeof buf);
                    strcat(buf, "zip ./");
                    strcat(buf, fileName);
                    strcat(buf, ".zip ");
                    strcat(buf, fileFullName);
                    system((const char*)buf);						// uruchamianie polecenia kompresji
                    printf("Etap 4 - Utworzono polecenie do kompresji pliku.\n");
                    /* Tworzenie i uruchomienie polecenia do kompresji pliku */

		    /* Usuwanie pliku do kompresji */
		    memset(buf, 0, sizeof buf);						// czyszczenie bufora
		    strcat(buf, "rm ");
		    strcat(buf, fileFullName);
		    system((const char*)buf);						// usunięcie pliku do kompresji
		    /* Usuwanie pliku do kompresji */

		    if(fileFullName)	free(fileFullName);				// czyszczenie pamięci dla nazwy
		    memset(buf, 0, sizeof buf);						// czyszczenie bufora
		    
		    if(mode==1)
		    {
		    	close(i);
		    	FD_CLR(i, &wmask);
		    }
		    else
		    {
		        /* Tworzenie nazwy pliku z rozszerzeniem po kompresji */
		        strcat(buf, fileName);
		        printf("buf: %s\n", buf);
		        strcat(buf, ".zip");
                        /* Tworzenie nazwy pliku z rozszerzeniem po kompresji */	
                    
                        /* Przypisanie nazwy pliku do zmiennej */
                        fileFullName = malloc(strlen(buf));
                        memset(fileFullName, 0, strlen(buf));				// czyszczenie fileFullName z artefaktów
                        strcpy(fileFullName, buf);
                        printf("fileFullName: %s\n", fileFullName);
                        /* Przypisanie nazwy pliku do zmiennej */
                    
		        strcat(buf, "?");						// dopisanie '?' do nazwy pliku
	
		        printf("Etap 4 - wysyłanie nazwy pliku zip...\n");
                        write_to(i, buf, strlen(buf)+1);
                        printf("Etap 4 - wyslano nazwe pliku zip.\n");
                    
		        read_ans(i, "4");						// odbieranie odpowiedzi
		    
		        printf("Name of zip file: %s\n", fileFullName);
                    
                        FD_SET(i, &sizeWriteMask);					// aktywacja maski sizeWriteMask
                    }
                }
                if(i == fdmax)
                {
		    printf("Stan sizeWriteMask: %d\n", !FD_ISSET(fdmax, &sizeWriteMask));
		    printf("Stan fullWriteMask: %d\n\n", !FD_ISSET(fdmax, &fullWriteMask));
                    while(fdmax > sfd && !FD_ISSET(fdmax, &rmask) && 
                    		!FD_ISSET(fdmax, &fullWriteMask) && !FD_ISSET(fdmax, &sizeWriteMask))
                        fdmax -= 1;
                }
                continue;
            }
            else if(FD_ISSET(i, &rmask))
            {
		if(FD_ISSET(i, &fullReadMask))
		{
		    // Etap 3 - odczytanie całego pliku i wysłanie odpowiedzi
		    
		    fds -= 1;
		    
		    bytes = 0;								// czyszczenie zmiennej pomocniczej
		    fileData = malloc(fileSize);					// alokacji pamięci dla pliku
		    memset(fileData, 0, fileSize);
		     
		    /* pełne odczytanie całego pliku */
		    printf("Etap 3 - odczytywanie calego pliku...\n");
		    while(1)
		    {
			bytes += read(i, fileData+bytes, fileSize-bytes);
			if(bytes>=fileSize) 
			{
			    printf("Etap 3 - zakończono odczytywanie pliku.\n");
			    break; 
			}
		    }
		    /* pełne odczytanie całego pliku */
		    
		    send_ans(i, "3");							// wysyłanie odpowiedzi
		    
		    FD_SET(i, &wmask);							// aktywacja maski wmask
		    FD_CLR(i, &rmask);							// deaktywacja maski rmask
		    FD_CLR(i, &fullReadMask);						// deaktywacja maski fullReadMask
		}
		else if(FD_ISSET(i, &sizeReadMask))
		{
		    // Etap 2 - odbieranie rozmiaru pliku do kompresji i wysyłanie odpowiedzi
		    
		    bytes = 0;							// czyszczenie zmiennej pomocniczej
		    memset(buf, 0, sizeof buf);					// czyszczenie bufora
		    
		    /* pełny odczyt rozmiaru pliku */
		    printf("Etap 2 -odczytywanie rozmiaru...\n");
		    while(1)
		    {
			bytes += read(i, buf+bytes, BUF_SIZE-bytes);
			if(buf[bytes-2] == '?')
		 	{
			    printf("Etap 2 - zakonczono odczyt rozmiaru.\n");
			    break;
			}
		    }
		    /* pełny odczyt rozmiaru pliku */
		    
		    fileSize = atoi(buf);					// zamiana rozmiaru na long int
		    
		    send_ans(i, "2");						// wysyłanie odpowiedzi
		    
		    printf("Size of file: %ld\n", fileSize);
		    
		    FD_SET(i, &fullReadMask);					// aktywacja maski fullReadMask
		    FD_CLR(i, &sizeReadMask);					// deaktywacja maski sizeReadMask
		}
		else if(FD_ISSET(i, &nameReadMask))
       		{
       		    // Etap 1 - odbieranie nazwy pliku do kompresji i wysyłanie odpowiedzi
       		    
       		    bytes = 0;							// czyszczenie zmiennej pomocniczej
		    memset(buf, 0, sizeof buf);					// czyszczenie bufora
		    
		    /* pełne odczytanie nazwy pliku do kompresji */
		    printf("Etap 1 - odczytywanie nazwy...\n"); 
		    while(1)
		    {
			bytes += read(i, buf+bytes, BUF_SIZE-bytes);
			if(buf[bytes-2] == '?')
			{
			    printf("Etap 1 - zakonczono odczyt nazwy.\n");
			    break;
			}
		    }
		    printf("odczytano %d bytes\n", bytes);
		    printf("Buf %s\n", buf);
		    /* pełne odczytanie nazwy pliku do kompresji */
		    
		    /* odczyt długości nazwy pliku i pozycji '.' */
		    fileLen = strlen(buf)-1;
		    printf("FileLen: %ld\n", fileLen);   
		    long int dotPos = 0;
		    while(1)
		    {
			if(buf[dotPos] == '.')    break;
			dotPos++;
		    }
		    /* odczyt długości nazwy pliku i pozycji '.' */
		    
		    /* przepisywanie nazw pliku */
		    fileFullName = malloc(fileLen);					// alokacja pamięci dla nazwy pliku
		    memset(fileFullName, 0, fileLen);					// czyszczenie fileFullName z artefaktów
		    fileName = malloc(dotPos);						// alokacja pamięci dla nazwy bez roz.
		    memset(fileName, 0, dotPos);					// czyszczenie fileName z artefaktów
		    for(long int k=0;k<fileLen;k++)
		    {
		    	fileFullName[k]=buf[k];						// przepisanie nazwy pliku
		    	if(k<dotPos)
		    	{
		    	    fileName[k] = buf[k];					// przepisywanie nazwy bez rozszerzenia
		    	    fileName[k+1] = '\0';
		    	}
		    }	
		    fileFullName[fileLen] = '\0';
		    printf("fileName: %s\n", fileName);
		    /* przepisywanie nazw pliku */
		    
		    send_ans(i, "1");							// wysyłanie odpowiedzi
		    
		    printf("Name of file: %s\n", fileFullName);		
		    
		    FD_SET(i, &sizeReadMask);						// aktywacja maski sizeReadMask
		    FD_CLR(i, &nameReadMask);						// deaktywacja maski nameReadMask
		}
		else
		{
		    // Etap 0 - odczytywanie trybu pracy serwera i wysyłanie odpowiedzi
		    
		    bytes = 0;							// czyszczenie zmiennej pomocniczej
		    memset(buf, 0, sizeof buf);					// czyszczenie bufora
		    
		    /* odczytanie trybu pracy serwera */
		    printf("\nEtap 0 - odczytywanie trybu pracy serwera...\n");
		    while(1)
		    {
			bytes += read(i, buf+bytes, BUF_SIZE-bytes);
			if(bytes>=2)
			{
			    printf("Etap 0 - odczytano tryb pracy serwera.\n");
			    break;
			}
		    }
		    /* odczytanie trybu pracy serwera */
		    
		    mode = atoi(buf);
		    printf("Server work mode: %d\n", mode);
		    send_ans(i, "0");							// wysyłanie odpowiedzi
		    
		    FD_SET(i, &nameReadMask);						// aktywacja maski nameReadMask
		}
                if(i == fdmax)
		{
		    printf("Stan wmask: %d\n", !FD_ISSET(fdmax, &wmask));
		    printf("Stan fullRead: %d\n", !FD_ISSET(fdmax, &fullReadMask));
		    printf("Stan sizeRead: %d\n", !FD_ISSET(fdmax, &sizeReadMask));
		    printf("Stan nameRead: %d\n", !FD_ISSET(fdmax, &nameReadMask));
                    while(fdmax > sfd && !FD_ISSET(fdmax, &wmask) && 
                    	  !FD_ISSET(fdmax, &fullReadMask) && !FD_ISSET(fdmax, &sizeReadMask) && !FD_ISSET(fdmax, &nameReadMask))
		    {
                         printf("Nastepny klient\n");
		         fdmax -= 1;
		    }
                }
		printf("Zakonczylem jedna akcje w read\n\n");
	    }
        }
    }
    close(sfd);
    return EXIT_SUCCESS;
    /**** PROGRAM SERWERA ****/
}


