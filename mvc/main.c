#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define cantCeldas 4096
#define largoLinea 300
#define cantMnem 32
#define longRotulo 10

typedef struct
{
    int linea;
    char nombre[longRotulo];
} regRotulo;

typedef struct nodo
{
    char nombre[longRotulo];
    char cad[longRotulo];
    int nro;
    int esInt;
    int lineaString;
    struct nodo *sig;
} nodo;

typedef struct nodo *TLista;

void preProceso(regRotulo reg[],int *nroRotulo, FILE *arch, int *errores);
void leoEspacios(char linea[], int *i);
int interpretaMnemonico(char mnem[]);
void toUpper(char mnem[], char retorno[]);
void procesoImpresion(FILE *origen,int nroLineas,regRotulo rotulos[],int nroRotulos, int instrucciones[], int *errores);
void procesoCarga(FILE *destino, int instrucciones[]);
int rotuloErroneo(regRotulo rotulos[],char auxRotulo[],int nroRotulos);
void identificaOperando(char auxOperando[], regRotulo rotulos[],int nroRotulos, int *tipoOperando,int *valorOperando);
int anyToInt(char auxOperando[]);
void imprimirLinea(char *auxRotulo,int instruccion,char *auxMnem,char *auxPrimerOperando,char *auxSegundoOperando,char *comentario);
void operandoEsRegistro(char *auxOperando,int *tipoOperando,int *valorOperando);
void operandoEsSimbolo(char *auxOperando, regRotulo *rotulos, int nroRotulos, int *tipoOperando,int *valorOperando);
void truncaImpresion(char auxOperando[], int bits);
int o=1;

//segunda parte
void leeDirectiva(char linea[],int *i,int *errores);
int repiteSimbolo(regRotulo rotulos[], char auxRotulo[], int nroRotulos);
int nroLineas=0, errorSimbolo=0, nroLineasEqu;
int CS, DS=1024, ES=1024, SS=1024;
TLista simbolos=NULL;
void agregoConstante(char simboloUpper[], char constante[]);
void asignoLineasStrings();


int main(int argC,char *argV[])
{
    int nroRotulos, errores, MV21;
    FILE *origen, *destino=NULL;
    regRotulo rotulos[10];
    if (argC<3)
        printf("No se han ingresado la cantidad de archivos pedidos: Origen.asm Destino.bin\n");
    else
        if (argC==3 || argC==4)
        {
            if (argC==4)
            {
                if (strcmp(argV[3],"-o")==0)
                  o=0;
            }
            origen=fopen(argV[1],"r");
            preProceso(rotulos,&nroRotulos,origen,&errores);
            asignoLineasStrings();
            int instrucciones[nroLineas];
            procesoImpresion(origen,nroLineas,rotulos,nroRotulos,instrucciones,&errores);
            if (errores==0)
            {
                destino=fopen(argV[2],"wb");
                MV21=((0x4D<<24)&0xFF000000) | ((0x56<<16)&0x00FF0000) | ((0x32<<8)&0x0000FF00) | (0x31 & 0x000000FF);
                CS=nroLineasEqu;
                fwrite(&MV21,1,sizeof(int),destino);
                fwrite(&DS,1,sizeof(int),destino);
                fwrite(&SS,1,sizeof(int),destino);
                fwrite(&ES,1,sizeof(int),destino);
                fwrite(&CS,1,sizeof(int),destino);
                procesoCarga(destino,instrucciones);
                fclose(destino);
            }
            fclose(origen);
        }
        else
            printf("Se ingresaron mas parametros que los permitidos\n");
    system("Pause");
    return 0;
}

//Reconoce previamente los rotulos y cuenta la cantidad de lineas->instrucciones a grabar en el archivo
//(si es que no hay error)
void preProceso(regRotulo registroRotulos[],int *nroRotulo, FILE *origen, int *errores)
{
    char linea[largoLinea], posibleRotulo[longRotulo], simboloUpper[longRotulo], constante[longRotulo];
    int i,j, simboloDuplicado;
    (*nroRotulo)=nroLineas=0;
    while (fgets(linea,largoLinea,origen) != NULL)
    {
        i=0;
        leoEspacios(linea,&i);
        if (linea[i]!=92)
        {
            if (linea[i]!='\r' && linea[i]!='\n' && linea[i]!='\000') //Si la linea no es de espacios puros sigo
            {
                if (i<largoLinea-20)
                {
                    j=0;
                    while (linea[i]!=' ' && linea[i]!='\0' && linea[i]!=':' && linea[i]!=',' && linea[i]!=';' && linea[i]!='\t')
                    {
                        posibleRotulo[j]=linea[i];
                        i++;
                        j++;
                    }
                    posibleRotulo[j]='\0';
                    toUpper(posibleRotulo, simboloUpper);
                    if (linea[i]==':') //Significa que hay un rotulo
                    {
                        simboloDuplicado=repiteSimbolo(registroRotulos,simboloUpper,*nroRotulo);
                        if (simboloDuplicado==1)
                        {
                            (*errores)++;
                            printf("Error simbolo duplicado: %s\n",posibleRotulo);
                        }
                        else
                        {
                            strcpy(registroRotulos[(*nroRotulo)].nombre,simboloUpper);
                            registroRotulos[(*nroRotulo)].linea=nroLineas;
                            (*nroRotulo)++;
                        }
                    }
                    else
                    {
                        leoEspacios(linea,&i); //Me fijo si es EQU
                        if ((linea[i]=='E' || linea[i]=='e') && (linea[i+1]=='q' || linea[i+1]=='Q') && (linea[i+2]=='U' || linea[i+2]=='u') && (linea[i+3]=='\t' || linea[i+3]==' '))
                        {
                            i+=3; j=0;
                            leoEspacios(linea,&i);
                            while (linea[i]!=' ' && linea[i]!='\0' && linea[i]!='\n' && linea[i]!='\r' && linea[i]!=';' && linea[i]!='\t')
                            {
                                constante[j]=linea[i];
                                i++;
                                j++;
                            }
                            constante[j]='\0';
                            simboloDuplicado=repiteSimbolo(registroRotulos,simboloUpper,*nroRotulo);
                            if (simboloDuplicado==1)
                            {
                                (*errores)++;
                                printf("Error simbolo duplicado: %s\n",posibleRotulo);
                            }
                            else
                            {
                                agregoConstante(simboloUpper,constante);
                            }
                            nroLineas--; //Resta para simplificar la ominion de la linea EQU
                        }
                    }
                }
                if (linea[i]!=';') //Si la linea no contiene solo comentario, se suma la cantidad de lienas
                    nroLineas++;
            }
        }
    }
    rewind(origen); //Se apunta nuevamente al comienzo del archivo
}

void asignoLineasStrings()
{
    nroLineasEqu=nroLineas;
    TLista act=simbolos;
    while (act)
    {
        if (act->esInt==0)
        {
            act->lineaString=nroLineasEqu;
            nroLineasEqu+=strlen(act->cad)+1;
        }
        act=act->sig;
    }
}

void agregoConstante(char simboloUpper[], char constante[])
{
    char auxConstante[longRotulo];
    int i=0, j=0;
    TLista aux, act;
    aux=(TLista)malloc(sizeof(nodo));
    strcpy(aux->nombre,simboloUpper);
    if (constante[0]=='"')
    {
        constante++;
        while (constante[i]!='"')
        {
            auxConstante[j]=constante[i];
            i++; j++;
        }
        auxConstante[j]='\0';
        strcpy(aux->cad,auxConstante);
        aux->esInt=0;
    }
    else
    {
        aux->nro=anyToInt(constante);
        aux->esInt=1;
    }
    if (simbolos==NULL)
    {
        aux->sig=simbolos;
        simbolos=aux;
    }
    else
    {
        act=simbolos;
        while (act->sig!=NULL)
            act=act->sig;
        act->sig=aux;
    }
    aux->sig=NULL;
}

int repiteSimbolo(regRotulo rotulos[], char auxSimbolo[], int nroRotulos)
{
    int i=0;
    TLista act;
    while (i<nroRotulos && strcmp(auxSimbolo, rotulos[i].nombre)!=0)
        i++;
    if (strcmp(auxSimbolo, rotulos[i].nombre)==0)
        return 1;
    else
    {
        act=simbolos;
        while (act && strcmp(act->nombre,auxSimbolo)!=0)
            act=act->sig;
        if (!act)
            return 0;
        else
            return 1;
    }
}

void leoEspacios(char linea[], int *i)
{
    while ((*i)<largoLinea && (linea[(*i)]==' ' || linea[(*i)]=='\t'))
        (*i)++;
}

//Devuelve el codigo de instruccion del mnemonico
int interpretaMnemonico(char mnem[])
{
    char mnemUpper[5];
    toUpper(mnem, mnemUpper);
    char acciones[][5]= {"MOV", "ADD", "SUB", "SWAP", "MUL", "DIV", "CMP", "SHL", "SHR", "AND", "OR", "XOR","SLEN","SMOV","SCMP",// DOS OP
    "SYS","JMP","JZ","JP","JN","JNZ","JNP","JNN","LDL","LDH","RND","NOT","PUSH","POP","CALL",//UN OP
    "RET","STOP"}; //CERO OP
    int mnemHexa[cantMnem] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9,0xA,0xB,0xC,0xD,0xE,
        0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB,0xFC,0xFD,0xFE,
        0xFF0,0xFF1 };
    int resultado, i=0;
    while (strcmp(mnemUpper,acciones[i])!=0 && i<cantMnem)
        i++;
    if (i>=cantMnem)
    {
        resultado = 0xFFFF; //El mnemonico pasado por parametro no existe, deriva en error de sintaxis
    } else {
        resultado=mnemHexa[i];
    }
    return resultado;
}

//Aplica una mascara en caso de ser necesario, el retorno esta en mayuscula para su analisis
void toUpper(char mnem[], char retorno[])
{
    int i, longMnem=strlen(mnem);
    for (i=0; i<longMnem; i++)
        if (mnem[i]>96 && mnem[i]<123)
            retorno[i]=mnem[i] & 0b11011111;
        else
            retorno[i]=mnem[i];
    retorno[i]='\0';
}

//Analiza las lineas del .asm y copia las instrucciones
//al array para luego cargar el .bin en caso de no haber errores
void procesoImpresion(FILE *origen,int nroLineas,regRotulo rotulos[],int nroRotulos, int instrucciones[], int *errores)
{
    int i,j,k=0,l,codigoMnem,tipoPrimerOperando,tipoSegundoOperando,valorOperandoPrimero,valorOperandoSegundo, instruccionAct, errorEnLinea;
    char auxMnem[40], auxRotulo[longRotulo], linea[largoLinea];
    char auxPrimerOperando[longRotulo], auxSegundoOperando[longRotulo], comentario[40];
    (*errores)=0;
    while (fgets(linea,largoLinea,origen) != NULL)
    {
        instruccionAct=0;
        valorOperandoPrimero=valorOperandoSegundo=0;
        i=j=0;
        leoEspacios(linea,&i);
        if (linea[i]!='\r' && linea[i]!='\n' && linea[i]!='\000') //Si la linea no es vacia, entra
        {
            if (linea[i]==92)
            {
                i+=5;
                leeDirectiva(linea, &i, errores);
            }
            else
            {
                strcpy(comentario,"-"); //"-" bandera para identificar que componentes de las instrucciones no existen en la linea
                errorEnLinea=0;
                while (i<largoLinea && linea[i]!=':' && linea[i]!=' ' && linea[i]!='\t' && linea[i]!='\r' && linea[i]!='\n')
                { //Avanza sobre la linea y guarda el posible mnemonico en auxMnem
                    auxMnem[j]=linea[i];
                    i++;
                    j++;
                }
                auxMnem[j]='\0';
                if (linea[i]==':') //El posible mnemonico resulta ser un rotulo
                {
                    strcpy(auxRotulo,auxMnem);
                    i++;
                    leoEspacios(linea,&i);
                    j=0;
                    while (i<largoLinea && linea[i]!=' ' && linea[i]!='\t' && linea[i]!='\r' && linea[i]!='\n')
                    { //Ahora es seguro que se lee un mnemonico (aunque puede ser erroneo)
                        auxMnem[j]=linea[i];
                        i++;
                        j++;
                    }
                    auxMnem[j]='\0';
                }
                else
                    strcpy(auxRotulo,"-"); //Si la linea no tiene rotulo utilizamos la bandera
                if (auxMnem[0]!=';')
                {
                    l=i;
                    leoEspacios(linea,&l); //Me fijo si hay EQU antes de decir que el mnemonico es erroneo
                    if ((linea[l]=='E' || linea[l]=='e') && (linea[l+1]=='q' || linea[l+1]=='Q') && (linea[l+2]=='U' || linea[l+2]=='u') && (linea[l+3]=='\t' || linea[l+3]==' '))
                    {
                        codigoMnem=0xFFFF;
                        comentario[0]='E'; //Damos por sentado que hay un EQU (referencia para impresion)
                    }
                    else
                        codigoMnem=interpretaMnemonico(auxMnem);
                }
                else
                { //Si el posible mnemonico comienza con ; significa que la linea es solo de comentario
                    while (i<largoLinea && linea[i]!='\r' && linea[i]!='\n')
                    {
                        auxMnem[j]=linea[i]; //Sigo cargando el comentario en auxMnem
                        i++;
                        j++;
                    }
                    auxMnem[j]='\0';
                    strcpy(comentario,auxMnem);
                    codigoMnem=0xFFFF;
                }
                if (codigoMnem==0xFFFF) //Entra si el mnemonico es erroneo o si la linea solo es de comentario
                {
                    instruccionAct=0xFFFFFFFF;
                    if (o==1 && comentario[0]==';')
                    {
                        printf("Linea de comentario:\t\t\t\t\t\t%s\n",&comentario);
                    }
                    else
                    {
                        if (comentario[0]!=';' && comentario[0]!='E') //Imprime el error si la linea no es de comentario ni de EQU
                        {
                            imprimirLinea("-",0xFFFFFFFF,auxMnem,"-","-",comentario);
                            if (comentario[0]!=';')
                            {
                                errorEnLinea=1;
                                (*errores)++;
                            }
                        }
                    }
                }
                else
                {
                    strcpy(auxPrimerOperando,"-"); //Inicializo el primer operando como inexistente
                    if (0x0<=codigoMnem && codigoMnem<=0xE) //Leo el primer operando
                    {
                        leoEspacios(linea,&i);
                        j=0;
                        while (i<largoLinea && linea[i]!=',' && linea[i]!=' ' && linea[i]!='\t')
                        {
                            auxPrimerOperando[j]=linea[i];
                            j++;
                            i++;
                        }
                        auxPrimerOperando[j]='\0';
                        identificaOperando(auxPrimerOperando,rotulos,nroRotulos,&tipoPrimerOperando,&valorOperandoPrimero);
                        leoEspacios(linea,&i);
                        if (linea[i]==',')
                            i++;
                    }
                    leoEspacios(linea,&i);
                    if (0x0<=codigoMnem && codigoMnem<=0xFE) //Leo el segundo operando
                    {
                        j=0;
                        while (i<largoLinea && linea[i]!=';' && linea[i]!=' ' && linea[i]!='\t' && linea[i]!='\r' && linea[i]!='\n')
                        {
                            auxSegundoOperando[j]=linea[i];
                            j++;
                            i++;
                        }
                        auxSegundoOperando[j]='\0';
                        identificaOperando(auxSegundoOperando,rotulos,nroRotulos,&tipoSegundoOperando,&valorOperandoSegundo);
                    }
                    else
                        if (codigoMnem==0xFF0 || codigoMnem==0xFF1)
                            strcpy(auxSegundoOperando,"-"); //Si no hay operandos, marco tambien el segundo como inexistente
                        if (valorOperandoPrimero<-35000 || valorOperandoSegundo<-35000) //Identifica error de simbolo
                        {
                            errorSimbolo=1;
                            valorOperandoPrimero=0x0FFF;
                            tipoPrimerOperando=0b00;
                            valorOperandoSegundo=0x0FFF;
                            tipoSegundoOperando=0b00;
                            errorEnLinea=1;
                            (*errores)++;
                        }
                        leoEspacios(linea,&i);
                        if (linea[i]==';') //Leo el comentario luego de leer satisfactoriamente los operandos
                        {
                            j=0;
                            while (i<largoLinea && linea[i]!='\r' && linea[i]!='\n')
                            {
                                comentario[j]=linea[i];
                                i++;
                                j++;
                            }
                            comentario[j]='\0';
                        }
                        if (strcmp(auxPrimerOperando,"-")==0 && strcmp(auxSegundoOperando,"-")==0) //Armo instruccion de ningun operando
                            instruccionAct=codigoMnem<<20;
                        else
                            if (strcmp(auxPrimerOperando,"-")==0 && (strcmp(auxSegundoOperando,"-")!=0))
                            {
                                if (valorOperandoSegundo>0xFFFF && o==1)
                                {
                                    printf("ADVERTENCIA: en la siguiente linea se trunco el operando: %s a 16 bits\n",&auxSegundoOperando);
                                    if (auxSegundoOperando[0]<65)
                                        truncaImpresion(auxSegundoOperando,16);
                                } //Armo instruccion de un operando
                                instruccionAct= (codigoMnem<<24) | ((tipoSegundoOperando<<22) & 0x00C00000) | (valorOperandoSegundo & 0x0000FFFF);
                            }
                            else
                            {
                                if (valorOperandoPrimero>0xFFF && valorOperandoSegundo>0xFFF && o==1)
                                {
                                    printf("ADVERTENCIA: en la siguiente linea se truncaron ambos operando: %s y %s a 12 bits\n",&auxPrimerOperando,&auxSegundoOperando);
                                    if (auxPrimerOperando[0]<65)
                                        truncaImpresion(auxPrimerOperando,12);
                                    if (auxSegundoOperando[0]<65)
                                        truncaImpresion(auxSegundoOperando,12);
                                }
                                else
                                    if (valorOperandoPrimero>0xFFF && o==1)
                                    {
                                        printf("ADVERTENCIA: en la siguiente linea se trunco el 1er operando: %s a 12 bits\n",&auxPrimerOperando);
                                        if (auxPrimerOperando[0]<65)
                                            truncaImpresion(auxPrimerOperando,12);
                                    }
                                    else
                                        if (valorOperandoSegundo>0xFFF && o==1)
                                        {
                                            printf("ADVERTENCIA: en la siguiente linea se trunco el 2do operando: %s a 12 bits\n",&auxSegundoOperando);
                                            if (auxSegundoOperando[0]<65)
                                                truncaImpresion(auxSegundoOperando,12);
                                        } //Armo la instruccion de dos operandos
                                instruccionAct= (codigoMnem<<28) | ((tipoPrimerOperando<<26) & 0x0C000000) |
                                ((tipoSegundoOperando<<24) & 0x03000000) | ((valorOperandoPrimero<<12) & 0x00FFF000) |
                                (valorOperandoSegundo & 0x00000FFF);
                            }
                        instrucciones[k]=instruccionAct; //Guardo en el array la instruccion
                        k++;
                        if (errorEnLinea==1 || o==1) //Si esta permitido muestro por pantalla (si es error se muestra igual)
                            imprimirLinea(auxRotulo,instruccionAct,auxMnem,auxPrimerOperando,auxSegundoOperando,comentario);
                        else
                            imprimirLinea("XXXX","-","-","-","-","-"); //No se muestra la linea por pantalla
                   // }


                }
            }
        }
    }
}

void leeDirectiva(char linea[],int *i, int *errores)
{
    char auxSegmento[6], segmento[6], tamanio[10];
    int k, tamanioInt;
    while (linea[(*i)]!='\n' && linea[(*i)]!='\r' && linea[(*i)]!='\000')
    {
        leoEspacios(linea,i);
        k=0;
        while(linea[(*i)]!='\n' && linea[(*i)]!='\r' && linea[(*i)]!='\000' && linea[(*i)]!='=')
        {
            auxSegmento[k]=linea[(*i)];
            (*i)++;
            k++;
        }
        auxSegmento[k]=='\0';
        toUpper(auxSegmento,segmento);
        k=0;
        if (linea[(*i)]=='=')
        {
            (*i)++;
            while (linea[(*i)]!='\n' && linea[(*i)]!='\r' && linea[(*i)]!='\000' && linea[(*i)]!='\t' && linea[(*i)]!=' ')
            {
                tamanio[k]=linea[(*i)];
                (*i)++;
                k++;
            }
            tamanio[k]='\0';
            tamanioInt=anyToInt(tamanio);
            if (strcmp(segmento,"STACK")==0)
                SS=tamanioInt;
            else
                if (strcmp(segmento,"EXTRA")==0)
                    ES=tamanioInt;
                else
                    if (strcmp(segmento,"DATA")==0)
                        DS=tamanioInt;
        }
        leoEspacios(linea, i);
    }
    if (DS>65535 || DS<0 || ES>65535 || ES<0 || SS>65535 || SS<0)
    {
        (*errores)=1;
        printf("Error valores inapropiados en directivas: Segmento<0 0xFFFF<Segmento\n");
    }
}

void truncaImpresion(char auxOperando[], int bits)
{
    int auxInt,rellenar=0;
    char auxChar[12],simbolo='-';
    if (auxOperando[0]=='[')
    {
        auxOperando++;
        rellenar=1;
    }
    if (auxOperando[0]=='%' || auxOperando[0]=='#' || auxOperando[0]=='@')
        simbolo=auxOperando[0];
    if (bits==12)
    {
        auxInt=anyToInt(auxOperando);
        auxInt&=0xFFF;
    }
    else
    {
        auxInt=anyToInt(auxOperando);
        auxInt&=0xFFFF;
    }
    if (simbolo=='%')
        sprintf(auxChar,"%X",auxInt);
    else
        if (simbolo=='@')
            sprintf(auxChar,"%o",auxInt);
        else
            sprintf(auxChar,"%d",auxInt);
    if (simbolo!='-')
        auxOperando++;
    strcpy(auxOperando,auxChar);
    if (rellenar==1)
        strcat(auxOperando,"]");
}

void procesoCarga(FILE *destino, int instrucciones[])
{
    int i, longitud, aux;
    TLista act=simbolos;
    for (i=0; i<nroLineas; i++)
    {
        fwrite(&instrucciones[i],1,sizeof(int),destino);
    }
    while (act)
    {
        if (act->esInt==0)
        {
            i=0;
            longitud=strlen(act->cad);
            while (i<longitud)
            {
                aux=(act->cad[i]) & 0x000000FF;
                i++;
            }
            aux=0;
            fwrite(&aux,1,sizeof(int),destino);
        }
        act=act->sig;
    }
}

//Devuelve el tipo de operando y el valor del mismo correspondientes
void identificaOperando(char auxOperando[], regRotulo rotulos[], int nroRotulos, int *tipoOperando,int *valorOperando)
{
    char aux[10], signo;
    int offset=0, i=3, j=0;
    if (auxOperando[0]=='[') //Es operando directo
    {
        if (auxOperando[1]<65)
        {
            (*tipoOperando)=0b10;
            (*valorOperando)=anyToInt(++auxOperando);
        }
        else
        {
            aux[0]=auxOperando[1]; aux[1]=auxOperando[2]; aux[2]='\0';
            operandoEsRegistro(aux,tipoOperando,valorOperando);
            if (auxOperando[i]!=']')
            {
                signo=auxOperando[i];
                i++;
                while (auxOperando[i]!=']')
                {
                    aux[j]=auxOperando[i];
                    j++;
                    i++;
                }
                aux[j]='\0';
                offset=anyToInt(aux);
                if (offset<-35000)
                {
                    operandoEsSimbolo(aux,rotulos,nroRotulos,tipoOperando,&offset);
                }
                if (signo=='-')
                    offset*=-1;
            }
            if ((offset>0xFF || offset<-128) && o==1)
                printf("ADVERTENCIA: en la siguiente linea se trunco el offset %c%s%c (puede ocasionarse un cambio de signo)\n",34,&aux,34);
            (*valorOperando)=((offset<<4) | (*valorOperando)) & 0x0FFF;
            (*tipoOperando)=0b11;
        }
    }
    else
    {
        (*valorOperando)=anyToInt(auxOperando);
        if ((*valorOperando)>-10000) //Es operando inmediato
        {
            (*tipoOperando)=0b00;
        }
        else
        {
            operandoEsRegistro(auxOperando,tipoOperando,valorOperando); //Es operando de registro
            if ((*valorOperando)>0)
                (*tipoOperando)=0b01;
            else
                operandoEsSimbolo(auxOperando,rotulos,nroRotulos,tipoOperando,valorOperando); //Es rotulo, es decir inmediato
        }
    }
}

int anyToInt(char auxOperando[])
{
    char bases[]= {"'*$*****@*#*****%"}, *out=NULL;
    int base=10;
    char *bp=strchr(bases,auxOperando[0]),simbolo=NULL;
    if (strcmp("'",auxOperando)==0)
        return 32;
    if (bp!=NULL)
    {
        base= bp-bases;
        simbolo=auxOperando[0];
        ++auxOperando;
    }//Si empieza por ' entonces se da el valor ASCII
    if (base==0)
        return (int)auxOperando[0];
    if (auxOperando[0]>57 && simbolo==NULL) //Si no cumple nada el operando se devuelve -1000 parea luego analizar si es rotulo o error
        return -40000;
    return strtol(auxOperando,out,base);
}

void operandoEsRegistro(char auxOperando[],int *tipoOperando,int *valorOperando)
{
    char upperOperando[longRotulo];
    toUpper(auxOperando,upperOperando);
    if (strcmp(upperOperando,"SP")==0)
        (*valorOperando)=6;
    else
        if (strcmp(upperOperando,"BP")==0)
            (*valorOperando)=7;
        else
            if (strcmp(upperOperando,"AC")==0)
                (*valorOperando)=9;
            else
                if (strcmp(upperOperando,"AX")==0)
                    (*valorOperando)=10;
                else
                    if (strcmp(upperOperando,"BX")==0)
                        (*valorOperando)=11;
                    else
                        if (strcmp(upperOperando,"CX")==0)
                            (*valorOperando)=12;
                        else
                            if (strcmp(upperOperando,"DX")==0)
                                (*valorOperando)=13;
                            else
                                if (strcmp(upperOperando,"EX")==0)
                                    (*valorOperando)=14;
                                else
                                    if (strcmp(upperOperando,"FX")==0)
                                        (*valorOperando)=15;
}

void operandoEsSimbolo(char *auxOperando, regRotulo *rotulos, int nroRotulos, int *tipoOperando,int *valorOperando)
{
    int i=-1;
    TLista act;
    char simboloUpper[longRotulo];
    toUpper(auxOperando,simboloUpper);
    do
    {
        i++;
    } while (i<nroRotulos && strcmp(rotulos[i].nombre,simboloUpper)!=0);
    if (i<nroRotulos)
    {
        (*valorOperando)=rotulos[i].linea;
        (*tipoOperando)=0b00;
    }
    else
    {
        act=simbolos;
        while (act)
        {
            if (strcmp(simboloUpper,act->nombre)==0)
            {
                if (act->esInt==1)
                    (*valorOperando)=act->nro;
                else
                    (*valorOperando)=act->lineaString;
                (*tipoOperando)=0b00;
            }
            act=act->sig;
        }
    }
}

void imprimirLinea(char *auxRotulo,int instruccion, char *auxMnem, char *auxPrimerOperando, char *auxSegundoOperando, char *comentario)
{
    static int memoria=0, linea=1;
    char charLinea[4];
    if (strcmp(auxRotulo,"XXXX")!=0) //No imprime la linea por pantalla al encontrar esa marca
    {
        sprintf(charLinea,"%d",linea);
        printf("[%04d]:",memoria);
        printf("  %02X %02X %02X %02X",(instruccion>>24)&0xFF,(instruccion>>16)&0xFF,(instruccion>>8)&0xFF,instruccion&0xFF);
        if (instruccion==0xFFFFFFFF && auxMnem[0]!=' ')
        { //Mensaje de error, se imprime siempre que exista
            printf("%12s: Error de sintaxis, mnemonico erroneo/inexistente\n",charLinea);
        }
        else
        {
            if (errorSimbolo==1) //Otro mensaje de error
            {
                printf("%12s: Error, no se encuentra el simbolo\n",charLinea);
                errorSimbolo=0;
            }
            else
            {
                if (strcmp(auxRotulo,"-")!=0)
                    printf("%12s:",auxRotulo);
                else
                    printf("%12s:",charLinea);
                printf(" %s \t",auxMnem);
                if (strcmp(auxPrimerOperando,"-")!=0)
                {
                    printf("%10s , ",auxPrimerOperando);
                    printf("%1s",auxSegundoOperando);
                }
                else
                    if (strcmp(auxSegundoOperando,"-")!=0)
                        printf("%10s\t",auxSegundoOperando);
                    else
                        printf("\t\t");
                if (strcmp(comentario,"-")!=0)
                    printf("  \t%s",comentario);
                printf("\n");
            }
        }
    }
    memoria++;
    linea++;
}
