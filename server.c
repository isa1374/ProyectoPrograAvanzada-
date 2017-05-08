#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>
#include <sys/wait.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

#define PORT 80
#define SIZE 8
#define MSGSIZE 1024
#define WRITE 1


static void Deamon(){
    pid_t pid;
    pid = fork(); 
    
    if(pid<0){
        exit(EXIT_FAILURE);
    }else if(pid>0){
        exit(EXIT_SUCCESS);
    }
    
    
    
}

char * endings[] ={"gif","image/gif","txt","text/plain",
                   "jpg","image/jpg","jpeg","image/jpeg",
                   "png", "image/png","ico", "image/ico",
                   "zip", "image/zip","gz","image/gz","tar", 
                   "image/tar","htm", "text/html","html",
                   "text/html","css","text/css","php", 
                   "text/html","pdf","application/pdf",
                   "zip","application/octet-stream","rar",
                   "application/octet-stream","js","application/javascript"};

int readLine(int s, char *line, int *result_size) {
    int acum=0, size;
    char buffer[SIZE];

    while( (size=read(s, buffer, SIZE)) > 0) {
        if (size < 0) return -1;
        strncpy(line+acum, buffer, size);
        acum += size;
        if(line[acum-1] == '\n' && line[acum-2] == '\r') {
            break;
        } 
    }

    *result_size = acum;

    return 0;
}

int writeLine(int s, char *line, int total_size) {
    int acum = 0, size;
    char buffer[SIZE];

    if(total_size > SIZE) {
        strncpy(buffer, line, SIZE);
        size = SIZE;
    } else  {
        strncpy(buffer, line, total_size);
        size = total_size;
    }

    while( (size=write(s, buffer, size)) > 0) {
        if(size<0) return size;
        acum += size;
        if (acum >= total_size) break;

        size = ((total_size-acum)>=SIZE)?SIZE:(total_size-acum)%SIZE;
        strncpy(buffer, line+acum, size);
    }

    return 0;
}

int serve(int s) {
    char command[MSGSIZE];
    int size, r, nlc = 0, fd, read_bytes;

    // Lee lo que pide el cliente
    while(1) {
        r = readLine(s, command, &size);
        command[size-2] = 0;
        size-=2;
        printf("[%s]\n", command);
        if(command[size-1] == '\n' && command[size-2] == '\r') {
            break;
        }
	// Esto esta mal mal mal 
	if(strlen(command) == 0) {
	    break;
	}
    }
    sleep(1);

    sprintf(command, "HTTP/1.0 200 OK\r\n");
    writeLine(s, command, strlen(command));

    sprintf(command, "Date: Fri, 31 Dec 1999 23:59:59 GMT\r\n");
    writeLine(s, command, strlen(command));

    sprintf(command, "Content-Type: image/jpeg\r\n");
    writeLine(s, command, strlen(command));

    sprintf(command, "Content-Length: 29936\r\n");
    writeLine(s, command, strlen(command));

    sprintf(command, "\r\n");
    writeLine(s, command, strlen(command));

    FILE *fin = fopen("mainiso_forcampus.jpg", "r");
	FILE *fout = fdopen(s, "w");

	struct stat buf;

	stat("mainiso?forcampus.jpg", &buf);
	printf("Size -----------> %d\n", buf.st_size);

	char file[32*1024];
	int suma = 0;
	size = fread(file, 1, 29936, fin);
	printf("Archivo: %d\n", size);

    while( (size=write(s, &file[suma], MSGSIZE)) > 0) {
		suma += size;
		if (suma >= 29936) break;
	}



    sync();
}

int main() {
    int sd, sdo, addrlen, size, r;
    struct sockaddr_in sin, pin;

    // 1. Crear el socket
    sd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(PORT);

    // 2. Asociar el socket a un IP/puerto
    r = bind(sd, (struct sockaddr *) &sin, sizeof(sin));
	if (r < 0) {
		perror("bind");
		return -1;
	}
    // 3. Configurar el backlog
    listen(sd, 5);

    addrlen = sizeof(pin);
    // 4. aceptar conexión
    while( (sdo = accept(sd, (struct sockaddr *)  &pin, &addrlen)) > 0) {
        //if(!fork()) {
            printf("Conectado desde %s\n", inet_ntoa(pin.sin_addr));
            printf("Puerto %d\n", ntohs(pin.sin_port));

            serve(sdo);

            close(sdo);
            exit(0);
        //}
    }
    close(sd);

    sleep(1);

}