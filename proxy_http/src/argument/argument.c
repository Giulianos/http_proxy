#include <argument/argument.h>
#include <memory.h>
#include <config/config.h>

void printhelp();

void argument_get(int argc,const char **argv) {
    int c;

    while ((c = getopt (argc, argv, "e:hl:l:L:m:M:o:p:t:v")) != -1){
        switch (c) {
            case 'e': //archivo de error
                config_create("error_file", optarg);
                break;
            case 'h': //imprime ayuda
                printhelp();
                exit(0);
            case 'l': //direccion IP del proxy HTTP
                //TODO hardcoded
                break;
            case 'L': //direccion IP del management
                //TODO hardcoded
                break;
            case 'M': //media-types
                config_create("media_types", optarg);
                break;
            case 'o': //puerto de management
                config_create("mgmt_port", optarg);
                break;
            case 'p': //puerto del proxy http
                config_create("proxy_port", optarg);
                break;
            case 't': //programa de transformacion
                config_create("cmd", optarg);
                break;
            case 'v': //imprime version
                printf("%s\n",VER_ARG);
                exit(0);
            default:
                break;
        }
    }
}
void printhelp(){
    printf("httpd - proxy HTTP que perimte transformar el cuerpo de las respuestas\n\n");
    printf("\t%s\n", "-e archivo de error");
    printf("\t%s\n", "-h ayuda");
    printf("\t%s\n", "-l direccion-http");
    printf("\t%s\n", "-L direccion-de-management");
    printf("\t%s\n", "-M media-types-transformables");
    printf("\t%s\n", "-o puerto de management");
    printf("\t%s\n", "-p puerto del proxy");
    printf("\t%s\n", "-t comando de transformacion");
    printf("\t%s\n", "-v version");
};


