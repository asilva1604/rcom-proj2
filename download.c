#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_STR_SIZE 256

int main (int argc, char **argv) {
    if (argc == 1) {
        puts("Welcome to our FTP client. Supply the url.");
        return(0);
    }
    if (argc > 2) {
        puts("No need to supply more than 1 argument. Just the URL is fine :)");
        return(0);
    }

    char url[MAX_STR_SIZE];
    
    if (strlen(argv[1]) >= MAX_STR_SIZE) {
        perror("URL too large...\n");
        exit(1);
    }

    strcpy(url, argv[1]);

    char ftp[7];
    memcpy(ftp, &url, 6);
    ftp[6] = '\0';
    
    if (strcmp("ftp://", ftp) != 0) {
        puts("Beggining of url should be : ftp://");
        exit(1);
    }

    char tmp[MAX_STR_SIZE] = {0};
    char user[MAX_STR_SIZE] = {0};
    char passw[MAX_STR_SIZE] = {0};
    char host[MAX_STR_SIZE] = {0};
    char path[MAX_STR_SIZE] = {0};

    int counter = 6; 

    if (strchr(url, '@')) {
        while (url[counter] != ':') {
            user[counter - 6] = url[counter];
            ++counter;
        }
        user[counter - 6] = '\0';
        ++counter;
        int pcounter = 0;
        while (url[counter] != '@') {
            passw[pcounter] = url[counter];
            ++counter;
            ++pcounter;
        }
        passw[pcounter] = '\0';
    } else {
        strcpy(user, "anonymous");
        strcpy(passw, "anonymous");
    }

    //WE HAVE USER AND PASSWORD

    printf("%s\n", (char *)&user);
    printf("%s\n", (char *)&passw);

    int hcounter = 0;

    while (url[counter] != '/') {
        host[hcounter] = url[counter];
        ++counter;
        ++hcounter;
    }

    ++counter;
    int pathcounter = 0;
    while (url[counter] != '\0') {
        path[pathcounter] = url[counter];
        ++pathcounter;
        ++counter;
    }

    printf("%s\n", host);
    printf("%s\n", path);

    struct hostent *h;

    if ((h=gethostbyname(host)) == NULL) {
        puts("Could not get host by name...");
        exit(1);
    }

    char *ip_address = inet_ntoa(*((struct in_addr *) h->h_addr));

    printf("Host Name : %s\n", h->h_name);
    printf("IP Address : %s\n", ip_address);

    return(1);
}