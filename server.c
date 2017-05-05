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

int readLine(int s, char*line, int *result_size){
	int acum = 0, size; 
	char buffer[SIZE];
	
		
	while( (size=read(s, buffer, SIZE)) > 0){
		if (size < 0) return -1; 
		strncopy(line+acum, buffer, size);
		acum += size; 
		if(line[acum-1] == '\n' && line[acum-2] == '\r'){
			break;
		}
	}
	
	*result_size = acum; 
	
	return 0;
}

int writeLine(int s, char *line, int total_size){
	int acum =0, size; 
	char buffer[SIZE];

	if(total_size > SIZE){ 
		strncpy(buffer, line, SIZE); 
		size = SIZE; 
	}else{
		strncpy(buffer, line, total_size); 
		size = total_size; 
	}
	
	while( (size = write(s, buffer, size)) >0){
		if(size < 0) return size; 
		acum += size; 
		if( acum >= total_size) break;
		
		size = ((total_size-acum)>=SIZE)?SIZE:(total_size-acum)%SIZE; 
		strncpy(buffer, line+acum, size);
	}

	return 0; 
}

int serve(int s) {
	char command[MSGSIZE]; 
	int size, r, nlc = 0, fd, read_bytes, t_m, tam; 
	char url[255]; 
	char method[255]; 
	char buff[2048];
	char uri[255];	

	pid_t pid; 
	uri[0] = '\0'; 
	strcat(uri, " "); 

	while(1){
		r = readLine(s, command, &size); 
		command[size-2] = 0; 
		size -= 2; 
		
		openlog("Peticion_servidor", LOG_PID | LOG_CONS, LOG_USER);
		syslog(LOG_INFO, "Header : %s\n", command); 
		closelog(); 

		printf("[%s]\n", command); 
		
		strcat(buff, command); 
		strcat(buff,"\n"); 

		if(size == 0 || (command[size-1] == '\n' && command[size-2] == '\r')){
			break; 
		}	
	}
	
	int i=0, j=0, k=1; 
	while(!isspace(buff[j] && (i < sizeof(method) -1)){
		method[i] = buff[j]; 
		i++; 
		j++;
	}
	method[i] = '\0'; 
	i =0; 
	
	while (isspace(buff[j]) && (j < sizeof(url))){
		j++;
	}

	while(!isspace(buff[j]) && (i < sizeof(url) -1) && (buff[j] != '\n')){
		url[i] = buff[j];
		i++; 
		j++; 
	}

	url[i] = '\0'; 
	if(strcmp(url,"\")==0){
		strcat(uri, " "); 
	}else{
		strcat(uri, url); 
	}

	if(strstr(uri, "?") > 0){
		if(strcmp (method, "GET") == 0){
			t_m = 1;
		}
	}else{
		if(strcmp(method, "POST") == 0){
			t_m = 2; 
		}else{
			t_m = 3; 
		}
	}

	if(t_m == 3){
		
	}
	
}
