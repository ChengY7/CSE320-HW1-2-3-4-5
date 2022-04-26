/*
 * "PBX" server module.
 * Manages interaction with a client telephone unit (TU).
 */
#include <stdlib.h>

#include "debug.h"
#include "pbx.h"
#include "server.h"
#include "csapp.h"

/*
 * Thread function for the thread that handles interaction with a client TU.
 * This is called after a network connection has been made via the main server
 * thread and a new thread has been created to handle the connection.
 */
#if 1
void *pbx_client_service(void *arg) {
    int connfd = *((int*)arg);
    free(arg);
    pthread_detach(pthread_self());
    TU *tu = tu_init(connfd);
    if(pbx_register(pbx, tu, connfd)==-1) {
        return NULL;
        //error to fix ^
    }
    int fd = tu_fileno(tu);
    if(fd==-1)
        return NULL;
        //error to fix ^
    char buffer;
    int readResult;
    char *command;
    int commandlen=0;
    int firstChar=1;
    while (1) {
        command=malloc(sizeof(char));
        while((readResult=read(fd, &buffer, 1))) {
            if(firstChar) {
                if(buffer=='\r') {
                    readResult=read(fd, &buffer, 1);
                    if(buffer=='\n')
                        break;
                    else {
                        command[commandlen]='\r';
                        commandlen++;
                        firstChar=0;
                        command = realloc(command, commandlen+1);
                        command[commandlen]=buffer; 
                        commandlen++;
                    }
                }
                command[commandlen]=buffer;
                commandlen++;
                firstChar=0;
            } 
            else if(buffer=='\r') {
                readResult=read(fd, &buffer, 1);
                if (buffer=='\n')
                    break;
                else {
                    command = realloc(command, commandlen+1);
                    command[commandlen]='\r'; 
                    commandlen++;
                    command = realloc(command, commandlen+1);
                    command[commandlen]=buffer; 
                    commandlen++;
                }
            }
            else {
               command = realloc(command, commandlen+1);
               command[commandlen]=buffer; 
               commandlen++;
            }
        }
        if(strcmp(command, tu_command_names[TU_PICKUP_CMD])==0) {
            tu_pickup(tu);
        }
        else if(strcmp(command, tu_command_names[TU_HANGUP_CMD])==0) {
            tu_hangup(tu);
        }
        else if(strncmp(command, tu_command_names[TU_DIAL_CMD], 4)==0) {
            int num = atoi(command+4);
            if(num>-1 && num<1024) {
                pbx_dial(pbx, tu, num);
            }
        }
        else if (strncmp(command, tu_command_names[TU_CHAT_CMD], 4)==0) {
            tu_chat(tu, command+4);
        }
        free(command);
        commandlen=0;
        firstChar=1;
    }
    int unreg = pbx_unregister(pbx, tu);
    if(unreg==-1)
        return NULL;
        //fix ^^
    close(fd);    
}
#endif

