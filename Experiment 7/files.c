#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>


int main (int argc, char **argv){
    int option;
    int fd, opened, isread, tam;
    char *option_value = NULL;
    char buffer[100];
    opened = isread = 0;

    while((option = getopt(argc, argv, "o:s:r:w:Wc")) != -1){
        switch(option){
            case 'o':
                if((fd = open(optarg, O_RDWR | O_CREAT, 0666)) == -1){
                    fprintf(stderr,"Erro na abertura do arquivo\n");
                    abort();
                }
                fprintf(stdout, "Arquivo %s aberto\n", optarg);
                opened = 1;
                break;

            case 's':
                if(!opened){
                    fprintf(stderr, "Arquivo não foi aberto\n");
                    abort();
                }else{
                    if (lseek(fd, atoi(optarg), SEEK_SET) == -1){
                        fprintf(stderr, "Erro na procura do arquivo\n");
                        abort();
                    }
                }
                break;

            case 'r':
                if(!opened){
                    fprintf(stderr, "Arquivo não foi aberto\n");
                    abort();
                }else{
                    if( (tam = read(fd, buffer, atoi(optarg)) ) == -1){
                        fprintf(stderr,"Erro de leitura\n");
                        abort();
                    }
                    fprintf(stdout, "Arquivo Lido: %s\n", buffer);
                    isread = 1;
                }
                break;

            case 'w':
                if(!opened){
                    fprintf(stderr, "Arquivo não foi aberto\n");
                    abort();
                }else{
                    if(write(fd, optarg, strlen(optarg)) == -1){
                        fprintf(stderr, "Erro na escrita do arquivo\n");
                        abort();
                    }
                    fprintf(stdout, "Mensagem escrita: %s\n", optarg);
                }
                break;

            case 'W':
                if(opened && isread){
                    if(write(fd, buffer, strlen(buffer)) == -1){
                        fprintf(stderr, "Erro na escrita do arquivo\n");
                        abort();
                    }
                    fprintf(stdout, "Mensagem escrita: %s\n", buffer);
                }else{
                    fprintf(stderr, "Arquivo não foi aberto ou lido\n");
                    abort();
                }
                break;

            case 'c':
                if(!opened){
                    fprintf(stderr, "Arquivo não foi aberto\n");
                    abort();
                }else{
                    if(close (fd) == -1){
                        fprintf(stderr, "Erro ao fechar o arquivo\n");
                        abort();
                    }
                    fprintf(stdout, "Arquivo fechado\n");
                }
                break;

            default:
                abort();
        }
    }
    return 0;
}