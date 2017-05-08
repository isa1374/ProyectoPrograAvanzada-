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


static void Daemon(){
    pid_t pid;
    pid = fork(); 
    
    if(pid<0){
        exit(EXIT_FAILURE);
    }else if(pid>0){
        exit(EXIT_SUCCESS);
    }
    
    if(setsid<0){
        exit(EXIT_FAILURE);
    }
    
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    
    pid = fork(); 
    
    if(pid<0){
        exit(EXIT_FAILURE);
    }else if(pid>0){
        exit(EXIT_SUCCESS);
    }
    
    umask(0);
    
    for(int n = sysconf(_SC_OPEN_MAX); n>0; n--){
        close(n);
    }
    
    openlog("start", LOG_PID, LOG_DAEMON);
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

char * getEnding(char *t){
    
    for( int o = 0; o<33; o+=2){
        if(strcmp(endings[o], t) == 0){
            printf("Type: %s\n", endings[o]);
            return endings[o+1];
        }
    }
    
    syslog(LOG_INFO, "File not found \n");
    openlog("Invalid page \n", LOG_PID, LOG_USER);
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
    int size, r, nlc = 0, fd, read_bytes, met, tam;
    
    //Tamaños predeterminados 
    char url[255];
    char path[255];
    char uri[255]; 
    char buff[255]; 
    char method[255];
    char *t; 
    
    pid_t pid; 
    uri[0] = '\0';
    
    //path dentro de la instancia donde está el proyecto 
    starcat(uri, " ");
    
    // Lee lo que pide el cliente
    while(1) {
        r = readLine(s, command, &size);
        command[size-2] = 0;
        size-=2;
        
        openlog("Petitions", LOG_PID, LOG_USER);
        syslog(LOG_INFO, "Header %s\n", command);
        closelog();
        
        strcat(buff,command);
        strcat(buff, "\n");
        
        printf("[%s]\n", command);
        if(size==0||(command[size-1] == '\n' && command[size-2] == '\r')) {
            break;
        }
        
    }
    
    int h=0, o=0, p=1;
    
    while(!isspace(buff[h]) && (o<sizeof(method)-1)){
        method[o]=buff[h];
        h++; 
        o++;
    }
    
    method[h] = '\0'; 
    h=0;
    
    while(isspace(buff[o]) && (o<sizeof(url)-1 && (buff[o] != '\n'))){
        url[h] = buff[o];
        o++;
        h++;
    }
    url[h] = '\0';
    
    if(strcmp(url, "/")==0){
        strcat(uri, "/index.html");
    }else{
        strcat(uri, url);
    }
    
    //tipo de acción 
    if(strstr(uri, "?")>0){
        if(strcmp(method, "GET")==0){
            met = 1;
        }
    }else if(strcmp(method, "POST")==0){
        met =2;
    }else{
        met=3;
    }
    
    printf("CGI: %d\n", met);
    
    switch(met){
        case 1:
            
            
            char k; 
            char sem[255];
            char *t_in; 
            char *t_eq; 
            char *t_query; 
            
            sem[0]='\0';
            printf("URL%s\n",url); 
            
            t_in =strtok(url,"?");
            t_in=strtok(NULL,"?");
            t_eq=strtok(t_in,"="); 
            
            strcat(sem,"SEMANTIC=http://54.17.5.168.195/");
            strcat(sem,t_eq);
            
            while((t_eq=strtok(NULL,"="))!=NULL){
                strcat(sem,"/");
                strcat(sem,t_eq);
            }
            
            printf("SEMANTIC URL: %s\n",sem);
            
            //get query and token 
            int p_read[2];
            int p_write[2];
            pipe(p_read);
            pipe(p_write);
            printf("URI: %s\n",uri);
            
            t_query ="QUERY_STRING=";
            t = strtok(uri,"?");
            t = strtok(NULL,"?");
            printf("Token: %s\n", t);
            
            char que[1024];
            strcat(que,t_query);
            strcat(que,t);
            printf("%s",que);
            
            if(!fork()){
                close(p_read[0]);
                close(p_write[1]);
                
                dup2(p_read[1], 1);
                dup2(p_write[0], 0);
                
                putenv(sem);
                putenv("REQUEST_METHOD=GET");
                putenv("REQUEST_STATUS=True");
                putenv(que);
                
                //place where php is saved
                putenv("SCRIPT_FILENAME=./files/test.php");
                
                if(execlp("php-cg1", "php-cgi","./files/test.php", 0)<0){
                    openlog("ErrorEXECLP", LOG_PID, LOG_USER);
                    syslog(LOG_INFO, "Error: %s\n", strerror(errno));
                    closelog(); 
                    perror("execlp");
                }
            }
            close(p_read[1]);
            close(p_write[0]);
            
            char aux; 
            int  te =0; 
            char buffx[100000];
            
            while(read(p_read[0], &aux, 1)>0){
                buffx[te] = aux;
                te++;
            }
            
            char buffer[32]; 
            int size = 0; 
            
            sprintf(command, "HTTP/1.0 200 OK\r\n");
            writeLine(s, command, strlen(command));
            
             sprintf(command, "Date: Fri, 31 Dec 1999 23:59:59 GMT\r\n");
            writeLine(s, command, strlen(command));
            
            sprintf(command, "Content-Type:text/html\r\n");
            writeLine(s, command, strlen(command));

            sprintf(command, "Content-Length: %s\r\n",t-50);
            writeLine(s, command, strlen(command));

            sprintf(command, "\r\n");
            writeLine(s, command, strlen(command));
            
            int au = 50; 
            while(au<te){
                write(s, &buff[au], 1);
                au++;
            }
            
            break;
            
        case 2: 
            break; 
            
        case 3: 
            printf("No path in URL: %s\n", url);
            path[0] = '\0';
            strcat(path,uri);
            printf("Path: %s\n", path);
            t = strtok(uri, ".");
            t = strtok(NULL, ".");
            FILE *f = fopen(path, "r");
            
            if(f != NULL){
                fseek(f, 0, SEEK_END);
                tam =ftell(f);
                fseek(f, 0, SEEK_SET);
                
                sprintf(command, "HTTP/1.0 200 OK\r\n");
                writeLine(s, command, strlen(command));
                
                 sprintf(command, "Date: Fri, 31 Dec 1999 23:59:59 GMT\r\n");
                writeLine(s, command, strlen(command));
                
                sprintf(command, "Content-Type: %s\r\n", getEnding(t));
                writeLine(s, command, strlen(command));
                
                sprintf(command, "Content-Length: %d\r\n", tam);
                writeLine(s, command, strlen(command));
                
                sprintf(command, "\r\n");
                writeLine(s, command, strlen(command));
                
                FILE *out = fdopen(s, "w"); 
                char file[tam];
                int su =0; 
                size = fread(file, 1,tam,f); 
                printf("File: %d\n", size);
                while((size=write(s, &file[su], size))>0){
                    su +=size; 
                    if(su>=tam){
                        break;
                    }
                    size = fread(file, 1,tam,f);
                }
            }else{
                openlog("Invalid page. ", LOG_PID, LOG_USER);
                syslog(LOG_INFO, "File not found.\n");
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
            break;
    }
    sync();
}

int main(int argc, char **argv) {
    int sd, sdo, addrlen, size, r;
    struct sockaddr_in sin, pin;
    
    int activar =0; 
    activar = atoi(argv[1]);
    
    if(activar == 1){
        Daemon();
    }

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
    signal(SIGCHLD,SIG_IGN);
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