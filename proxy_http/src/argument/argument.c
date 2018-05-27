#include <argument/argument.h>

void printhelp();
long parse_port(char * port_name, char *optarg);

int parse_mediatypes(struct mediatypes* mt,const char* op_arg);

void argument_get(int argc, char **argv) {
    int c;

    //definir un struct
    char* error_file="/dev/null";
    char* listen_address="0.0.0.0";
    char* mgmt_address="127.0.0.1";
    char* transform="";
    __uint16_t listen_port=8080;
    __uint16_t mgmt_port=9000;
    struct mediatypes mt;
    mt.size=0;
    mt.types=NULL;
    //end definir

    while ((c = getopt (argc, argv, "e:h:l:L:m:M:o:p:t:v")) != -1){
        switch (c) {
            case 'e': //archivo de error
                error_file=optarg;
                break;
            case 'h': //imprime ayuda
                printhelp();
                exit(0);
            case 'l': //direccion IP del proxy HTTP
                listen_address = optarg;
                break;
            case 'L': //direccion IP del management
                mgmt_address = optarg;
                break;
            case 'M': //media-types
                parse_mediatypes(&mt,optarg);
            case 'o': //puerto de management
                mgmt_port = (__uint16_t) parse_port("management port",optarg);
                break;
            case 'p': //puerto del proxy http
                listen_port = (__uint16_t) parse_port("proxy port",optarg);
                break;
            case 't': //programa de transformacion
                transform=optarg;
                break;
            case 'v': //imprime version
                printf("%s\n",VERSION);
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

long parse_port(char * port_name, char *optarg) {
    char *end     = 0;
    const long sl = strtol(optarg, &end, 10);

    if (end == optarg|| '\0' != *end
        || ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno)
        || sl < 0 || sl > USHRT_MAX) {
        fprintf(stderr, "%s port should be an integer: %s\n", port_name, optarg); //TODO: espaniol?
        exit(1);
    }

    return sl;
}

int parse_mediatypes(struct mediatypes* mt,const char* op_arg){
    int i=0;
    Strin *s=newStrin();
    if(s==NULL)
        return MEMERR;
    if(mt->size==0){
        mt->types=malloc(sizeof(struct mediatypes));
        if(mt->types==NULL)
            return MEMERR;
    }
    while(op_arg[i]!=0){
        char c=op_arg[i];
        if(c==','){
            void *tmp = realloc(mt->types, (mt->size+1)*sizeof(char*));
            if (tmp == NULL){
                free(mt->types);
                return MEMERR;
            }
            mt->types[mt->size]=retChar(s);
            mt->size++;
            free(s);
            s=newStrin();
            if(s==NULL)
                return -1;
        }else{
            if(OK!=addcToStrin(s,c))
                return MEMERR;
        }
        i++;
    }
    if(s->slen>0){
        mt->types[mt->size]=retChar(s);
        mt->size++;
        free(s);
    }
    if(mt->size==0){
        free(mt->types);
    }
    return ARG_OK;

}