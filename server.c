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
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#define PORT 4001 
#define SIZE 8
#define MSGSIZE 1024
#define READ 0
#define WRITE 1

char * extensions[] ={"gif","image/gif","txt","text/plain","jpg","image/jpg","jpeg",
                      "image/jpeg","png", "image/png","ico", "image/ico","zip", 
                      "image/zip","gz","image/gz", "tar", "image/tar","htm", 
                      "text/html","html","text/html","css","text/css","php", 
                      "text/html","pdf","application/pdf","zip","application/octet-stream",
                      "rar","application/octet-stream","js","application/javascript"};

static void demonizar(){
    pid_t pid;
    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);
    if (setsid() < 0)
        exit(EXIT_FAILURE);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);
    umask(0);
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>0; x--)
    {
        close (x);
    }
    openlog ("firstdaemon", LOG_PID, LOG_DAEMON);
}
char * obtenerTipo(char *token){
    int i;
    for(i=0;i<33;i+=2){
        if(strcmp(extensions[i],token)==0){
            printf("Tipo: %s\n",extensions[i+1]);
            return extensions[i+1];
        }
    }
    openlog("Pag no válida", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "No se encontró el archivo...\n");
    closelog();
    exit(0);
}

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
    int size, r, nlc = 0, fd, read_bytes, tamanio,tipo_metodo;
    char method[255];
    char url[255];
    char path[255];
    char *token;
    char uri[255];
    char buff[2048];
    pid_t pid; 
    uri[0]='\0';
    strcat(uri,"./files");

    // Lee lo que pide el cliente
    
    while(1) {
        r = readLine(s, command, &size);
        command[size-2] = 0;
        size-=2;
        
        //Guarda toda la informacion de las peticiones en el log ubicado en /var/log/syslog
        openlog("Peticiones_al_servidor", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "Header de la peticion: %s\n",command);
        closelog();
        
        printf("[%s]\n", command);
        
        //Guardar todos los comandos para su manipulacion posterior
        strcat(buff,command);
        strcat(buff,"\n");
        
        if(size==0||(command[size-1] == '\n' && command[size-2] == '\r')) {
            break;
        }
    }
    int i = 0, j = 0, k = 1;
    while (!isspace(buff[j]) && (i < sizeof(method) - 1)){
        method[i] = buff[j];
        i++;
        j++;
    }
    method[i] = '\0';
    i = 0;
    
    while (isspace(buff[j]) && (j < sizeof(url))){
        j++;
    }
    
    while (!isspace(buff[j]) && (i < sizeof(url) - 1) && (buff[j]!='\n')){
        url[i] = buff[j];
        i++;
        j++;
    }
    url[i] = '\0';
    if(strcmp(url,"/")==0){
        strcat(uri,"/index.html");
    }else{
        strcat(uri,url);
    }
    if(strstr(uri, "?")>0){
        if(strcmp(method,"GET")==0){
            tipo_metodo=1;
        }
    }else{
        if(strcmp(method,"POST")==0){
            tipo_metodo=2;
        }else{
            tipo_metodo=3;
        }
    }
    printf("MODO CGI: %d\n",tipo_metodo);
    if(tipo_metodo==3){
        printf("URL sin path: %s\n",url);
        path[0]= '\0';
	//strcat(path,"./files");
        strcat(path,uri);
        printf("Path: %s\n",path);
        token = strtok(uri,".");
        token = strtok(NULL,".");
	

        FILE *fin = fopen(path,"r");
        if(fin!=NULL){
            fseek(fin,0,SEEK_END);
            tamanio = ftell(fin);
            fseek(fin,0,SEEK_SET);
            
            sprintf(command, "HTTP/1.0 200 OK\r\n");
            writeLine(s, command, strlen(command));
            sprintf(command, "Date: Fri, 31 Dec 1999 23:59:59 GMT\r\n");
            writeLine(s, command, strlen(command));
            sprintf(command, "Content-Type:%s\r\n",obtenerTipo(token));
            writeLine(s, command, strlen(command));
            sprintf(command, "Content-Length:%d\r\n",tamanio);
            writeLine(s, command, strlen(command));
            sprintf(command, "\r\n");
            writeLine(s, command, strlen(command));
            
            FILE *fout = fdopen(s, "w");
            char file[tamanio];
            int suma = 0;
            size = fread(file, 1,tamanio, fin);
            printf("Archivo: %d\n", size);
            while( (size=write(s, &file[suma],size)) > 0) {
                suma += size;
                if (suma >= tamanio) break;
                size = fread(file, 1,tamanio, fin);
            }
        }
        else{
            openlog("Pag no válida", LOG_PID | LOG_CONS, LOG_USER);
            syslog(LOG_INFO, "No se encontró el archivo...\n");
            closelog();
            sprintf(command, "HTTP/1.0 200 OK\r\n");
            writeLine(s, command, strlen(command));
            sprintf(command, "Date: Fri, 31 Dec 1999 23:59:59 GMT\r\n");
            writeLine(s, command, strlen(command));
            sprintf(command, "Content-Type:text/plain\r\n");
            writeLine(s, command, strlen(command));
            sprintf(command, "Content-Length:9\r\n");
            writeLine(s, command, strlen(command));
            sprintf(command, "\r\n");
            writeLine(s, command, strlen(command));
            sprintf(command, "Error 404\r\n");
            writeLine(s, command, strlen(command));   
        }
    }
    if(tipo_metodo==1){
                char *token_query;
                char semantic[255];
                char *token_int;
                char *token_igual;
                printf("URL%s\n",url);
                semantic[0]='\0';
                token_int=strtok(url,"?");
                token_int=strtok(NULL,"?");
                token_igual=strtok(token_int,"=");
                strcat(semantic,"SEMANTIC=http://127.0.0.1/");
                strcat(semantic,token_igual);
                while((token_igual=strtok(NULL,"="))!=NULL){
                     strcat(semantic,"/");
                     strcat(semantic,token_igual);  
                }
                printf("URL semantico 1: %s\n",semantic);
                int pipe_read[2];
                int pipe_write[2];
                pipe(pipe_read);
                pipe(pipe_write);
                printf("Uri: %s\n",uri);
                
                token_query="QUERY_STRING=";
                token = strtok(uri,"?");
                token = strtok(NULL,"?");
                printf("Token: %s\n",token);
                char query[1024];
                strcat(query,token_query);
                strcat(query,token);
                printf(query);
                if(!fork()){
                    close(pipe_read[0]);
                    close(pipe_write[1]);

                    dup2(pipe_read[1], 1);
                    dup2(pipe_write[0], 0);
                    putenv(semantic);
                    putenv("REQUEST_METHOD=GET");
                    putenv("REDIRECT_STATUS=True");
                    putenv(query);
                    putenv("SCRIPT_FILENAME=test.php");

                    if(execlp("php-cgi", "php-cgi","test.php", 0)<0){
                        openlog("ErrorEXECLP", LOG_PID | LOG_CONS, LOG_USER);
                        syslog(LOG_INFO, "Error: %s\n", strerror(errno));
                        closelog();
                        perror("execlp");
                    }
                }
                close(pipe_read[1]);
                close(pipe_write[0]);

                char c;

                int t=0;
                char buffx[100000];
                while (read(pipe_read[0], &c, 1) > 0){                    
                    buffx[t]=c;
                    t++;
                }

                char buffer[32];
                int size = 0;

                sprintf(command, "HTTP/1.0 200 OK\r\n");
                writeLine(s, command, strlen(command));

                
                sprintf(command, "Date: Fri, 31 Dec 1999 23:59:59 GMT\r\n");
                writeLine(s, command, strlen(command));

                sprintf(command, "Content-Type:text/html\r\n");
                writeLine(s, command, strlen(command));

                sprintf(command, "Content-Length: %d\r\n",t-50);
                writeLine(s, command, strlen(command));

                sprintf(command, "\r\n");
                writeLine(s, command, strlen(command));

                int aux=50;                
                while(aux<t){      
                    write(s,&buffx[aux],1);
                    aux++;
                }

            }
    sync();
}

int main() {
    int sd, sdo, addrlen, size,r;
    struct sockaddr_in sin, pin;
	
	//demonizar();

   // 1. Crear el socket
	printf("Socket\n");
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
	printf("Puerto Asociado\n");
    // 3. Configurar el backlog
    listen(sd, 5);

    addrlen = sizeof(pin);
    // 4. aceptar conexión
    signal(SIGCHLD,SIG_IGN);
	printf("Aceptando peticiones\n");
    while( (sdo = accept(sd, (struct sockaddr *)  &pin, &addrlen)) > 0) {
        if(!fork()) {
            printf("Conectado desde %s\n", inet_ntoa(pin.sin_addr));
            printf("Puerto %d\n", ntohs(pin.sin_port));
            serve(sdo);
            close(sdo);
            exit(0);
        }
    }
    close(sd);
    sleep(1);
}


