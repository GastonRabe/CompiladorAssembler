#include <stdio.h>
#include <stdlib.h>
#include "declaraciones.h"
#include <string.h>
#define cantceldas 4096;

int RAM[4096];
int reg[16];
int Flags[3]={0,0,0};
int p=0;
char mnemonicos[4082][5];
char registros[16][3];
void (*Funciones[4081])(int tipo1, int tipo2,int op1,int op2, int RAM[], int reg[]);
void MOV(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void ADD(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void SUB(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void SWAP(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void MUL(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void DIV(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void CMP(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void SHL(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void SHR(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void AND(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void OR(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void XOR(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void SYS(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void JMP(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void JZ(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void JP(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void JN(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void JNZ(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void JNP(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void JNN(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void LDL(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void LDH(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void RND(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void NOT(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);
void STOP(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]);


void setCC(int reg[], int resultado);
void CargoMemoria(int argc , char *argv[]);
void AnalizaFlags(int argc,char *argv[]);    // fijarse el nombre variables
void iniciaVectorFunciones();
void Ejecuta();
void convierteINT(char aux[],int *dosop,int *x1,int *x2);
void diccionario();
void dissasembler();

//--------------- PROGRAMA ------------------------------------

   int main(int argc , char *argv[]){
    iniciaVectorFunciones();
    AnalizaFlags(argc,argv);
    diccionario();
    if (c)
        system("cls");
    CargoMemoria(argc,argv);  //cls al pcipio de ejecucion si c activado
    Ejecuta();
    return 0;
}

void CargoMemoria(int argc, char *argv[]){
    int inst,i=0;
    FILE *arch;
    if((arch=fopen(argv[1],"rb"))==NULL)
        printf("El archivo no existe");
    else{
        while(fread(&inst,sizeof(int),1,arch)==1){
            RAM[i]=inst;
            i++;
        }
        fclose(arch);
        DS=i;
        if (d)              //Hace dissasembler al pcipio de ejecucion, si es que el flag d esta activado
            dissasembler();
    }
}



void Ejecuta()
{
    int tipo1,tipo2,op1,op2,codop,i,x1,x2,dosop=0;
    char aux[9]={""},auxaux[9];
    IP=0;
    while(0<=IP && IP<DS)
    {
        tipo1=0; op1=0; op2=0; tipo2=0;
        if(((RAM[IP]>>28) & 0x0000000f)==15)                     //Si entra este if codop=SYS si corro el byte de max izq y me da F
            if(((RAM[IP]>>24) & 0x000000ff)==255)
                    codop=((RAM[IP]>>20 )& 0x00000fff);           // si entra este es porque es FF, 0 OPERANDOS

            else{
                    codop=((RAM[IP]>>24)&0x000000ff);            //1 OPERANDO
                    tipo2=(RAM[IP]>>22) & 0x00000003;
                    if(tipo2==0){                                  //OPERANDO INMEDIATO
                         op2=RAM[IP]&0xffff;
                         op2=op2 |((op2&0x800)?0xffff0000:0x0);     //CONTROLA INMEDIATO NEGATIVO
                    }
                    else                                        //OPERANDO REGISTRO
                        if(tipo2==1)
                            op2=(RAM[IP])&0x0000000f;
                        else                                    //OPERANDO DIRECTO
                            op2=(RAM[IP])&0x0000ffff;
            }
        else{
            codop=((RAM[IP]>>28)& 0x0000000f);                 // 2 OPERANDOS
            tipo1=((RAM[IP]>>26)& 0x00000003);
            if(tipo1==1)                                        //OPERANDO 1 DE REGISTRO. si el tipo1 es registro 4 bits nos interesan.
                op1=((RAM[IP]>>12)& 0x0000000f);
            else
                op1=((RAM[IP]>>12)& 0x00000fff);                //OPERANDO 1 DIRECTO
            tipo2=(RAM[IP]>>24)& 0x00000003;
            if(tipo2==0){                                       //TIPO 2 INMEDIATO
                op2=RAM[IP]&0xfff;
                op2= op2 | ((op2&0x800)? 0xfffff000 : 0x0);
            }
            else
                if(tipo2==1)                                      //TIPO 2 DE REGISTRO
                  op2=RAM[IP]&0xf;
                else
                 op2=RAM[IP]&0xfff;                             //TIPO 2 DIRECTO
        }
        IP++;
        Funciones[codop](tipo1,tipo2,op1,op2,RAM,reg);          //EJECUTA LA FUNCION CORRESPONDIENTE
        while(0<=IP && IP<DS && p==1){                           // YA CORROBORA QUE EL IP < DS ENTONCES NO HACE FALTA IF, (POR CASO DE STOP)
            tipo1=0; op1=0; op2=0; tipo2=0;                      //EJECUCION DEL PASO POR PASO
            if(c)
                system("cls");
            if(((RAM[IP]>>28) & 0x0000000f)==15)                     //MISMA DECODIFICACION DE LA INSTRUCCION QUE ANTES
                if(((RAM[IP]>>24) & 0x000000ff)==255)
                    codop=((RAM[IP]>>20 )& 0x00000fff);

                else{
                    codop=((RAM[IP]>>24)&0x000000ff);
                    tipo2=(RAM[IP]>>22)& 0x00000003;
                    if(tipo2==0){
                        op2=RAM[IP]&0xffff;
                        op2= op2 | ((op2&0x8000) ? 0xffff0000 : 0x0);
                    }
                    else if(tipo2==1)
                        op2=(RAM[IP])&0x0000000f;
                    else
                        op2=(RAM[IP])&0x0000ffff;
                    }
            else{
                codop=((RAM[IP]>>28)& 0x0000000f);
                tipo1=((RAM[IP]>>26)& 0x00000003);
                if(tipo1==1)
                    op1=((RAM[IP]>>12)& 0x0000000f);
                else
                    op1=((RAM[IP]>>12)& 0x00000fff);
                tipo2=(RAM[IP]>>24)& 0x00000003;
                if(tipo2==0){
                    op2=RAM[IP]&0xfff;
                    op2= op2 | ((op2&0x800) ? 0xfffff000 : 0x0);
                }
                else
                    if(tipo2==1)
                        op2=(RAM[IP])&0x0000000f;
                    else
                        op2=(RAM[IP])&0x00000fff;
                }
                IP++;
                Funciones[codop](tipo1,tipo2,op1,op2,RAM,reg);
                if (IP<DS){                                     //INGRESA SI LA INSTRUCCION ANTERIOR NO FUE STOP
                    if(d && IP<DS)
                        dissasembler();                         //EJECUTA EL DISSASEMBLER
                    strcpy(auxaux,"");
                    printf("[%04d] cmd:",IP);
                    fflush(stdin);
                    scanf("%[^\n]",auxaux);                     //LEE VALOR SIGUIENTE, PARA SABER SI SEGUIR EL PASO POR PASO O MOSTRAR ALGUNA CELDA, O SIMPLEMENTE SEGUIR
                    fflush(stdin);
                    strcpy(aux,auxaux);
                    strcpy(auxaux,"");
                    while(aux[0]!='\0' && aux[0]!='p'){           //SI EL VALOR ES UNA CELDA DE MEMORIA O VARIAS, LAS MUESTRA SIN SEGUIR EJECUTANDO, HASTA QUE SE PIDA SEGUIR EJECUTANDO
                      if(aux[0]>=48 && aux[0]<=57)                  //48 y 57 valores tabla ascii
                      {
                        convierteINT(aux,&dosop,&x1,&x2);           //CONVIERTE EL VALOR INGRESADO A 1 VARIABLE O 2 EN INT
                        if(dosop)   //dos operandos, el else va a 1 operando
                            for(i=x1 ; i<=x2 ; i++){
                                printf("[%04d]\t %04X %04X\t%d ",i,(RAM[i]>>16)&0xffff,RAM[i]&0xffff,RAM[i]);
                                printf("\n");
                            }
                            else
                            {
                                printf("[%04d]\t %04X %04X\t%d ",i,(RAM[i]>>16)&0xffff,RAM[x1]&0xffff,RAM[x1]);
                                printf("\n");
                            }
                    }
                    strcpy(auxaux,"");
                    printf("[%04d] cmd:",IP);
                    fflush(stdin);
                    scanf("%[^\n]",auxaux);         //VUELVE A PEDIR VALOR
                    fflush(stdin);
                    strcpy(aux,auxaux);
                    strcpy(auxaux,"");
                  }
                  if(aux[0]!='p')
                        p=0;            //PONE LE P EN 0 EN EL CASO DE QUE EL VALOR INGRESADO NO SEA P, Y DEBA SALIR DEL PASO POR PASO
                }
        }
    }

}



//---------------------FUNCIONES---------------------

void dissasembler()
{
    int i=0,auxcodop,auxtipo2,auxtipo1,auxop1,auxop2,contador=0;
    if (IP+5>DS && DS>10)   //Significa que tengo que mostrar lineas hacia atras para completar las 10 lineas
        i=DS-10;
    else
        if((IP-5)>0 && DS>10)           //CALCULA LAS LINEAS QUE DEBE MOSTRAR, SIEMPRE Y CUANDO SEA POSIBLE, SE MUESTRAN DIEZ
            i=IP-5;
        else
            i=0;
    printf("Codigo: \n");
    while(i<DS && contador<10)
    {
        contador++;
        if(i==IP)
            printf(">");
        if(((RAM[i]>>28) & 0x0000000f)==15)                     //MISMA DECODIFICACION ANTES MOSTRADA
            if(((RAM[i]>>24) & 0x000000ff)==255){
                auxcodop=((RAM[i]>>20 )& 0x00000fff);
                printf("\t[%04d]:   ",i);
                printf("%02X %02X %02X %02X",(RAM[i]>>24)&0xFF,(RAM[i]>>16)&0xFF,(RAM[i]>>8)&0xFF,RAM[i]&0xFF);
                printf("   %2d:   %s\n",i+1,mnemonicos[auxcodop]);
                }
                else{
                    auxcodop=((RAM[i]>>24)&0x000000ff);
                    auxtipo2=(RAM[i]>>22)& 0x00000003;
                    if(auxtipo2==0){
                        auxop2=RAM[i]&0xffff;
                        auxop2= auxop2 | ((auxop2&0x8000) ? 0xffff0000 : 0x0);
                        printf("\t[%04d]:   ",i);
                        printf("%02X %02X %02X %02X",(RAM[i]>>24)&0xFF,(RAM[i]>>16)&0xFF,(RAM[i]>>8)&0xFF,RAM[i]&0xFF);
                        printf("   %2d:   %s   \t%d\n",i+1,mnemonicos[auxcodop],auxop2);
                    }
                    else if(auxtipo2==1){
                        auxop2=(RAM[i])&0x0000000f;
                        printf("\t[%04d]:   ",i);
                        printf("%02X %02X %02X %02X",(RAM[i]>>24)&0xFF,(RAM[i]>>16)&0xFF,(RAM[i]>>8)&0xFF,RAM[i]&0xFF);
                        printf("   %2d:   %s   \t%s\n",i+1,mnemonicos[auxcodop],registros[auxop2]);
                    }
                    else{
                        auxop2=(RAM[i])&0x0000ffff;
                        printf("\t[%04d]:   ",i);
                        printf("%02X %02X %02X %02X",(RAM[i]>>24)&0xFF,(RAM[i]>>16)&0xFF,(RAM[i]>>8)&0xFF,RAM[i]&0xFF);
                        printf("   %2d:   %s   \t[%d]\n",i+1,mnemonicos[auxcodop],auxop2);
                        }
                    }
                        else{
                            auxcodop=((RAM[i]>>28)& 0x0000000f);
                            auxtipo1=((RAM[i]>>26)& 0x00000003);
                            if(auxtipo1==1){
                                auxop1=((RAM[i]>>12)& 0x0000000f);
                                printf("\t[%04d]:   ",i);
                                printf("%02X %02X %02X %02X",(RAM[i]>>24)&0xFF,(RAM[i]>>16)&0xFF,(RAM[i]>>8)&0xFF,RAM[i]&0xFF);
                                printf("   %2d:   %s   \t%s",i+1,mnemonicos[auxcodop],registros[auxop1]);
                            }
                            else{
                                auxop1=((RAM[i]>>12)& 0x00000fff);
                                printf("\t[%04d]:   " ,i);
                                printf("%02X %02X %02X %02X",(RAM[i]>>24)&0xFF,(RAM[i]>>16)&0xFF,(RAM[i]>>8)&0xFF,RAM[i]&0xFF);
                                printf("   %2d:   %s   \t[%d]" ,i+1,mnemonicos[auxcodop],auxop1);
                                }
                            auxtipo2=(RAM[i]>>24)& 0x00000003;
                            if(auxtipo2==0){
                                auxop2=RAM[i]&0xfff;
                                auxop2= auxop2 | ((auxop2&0x800) ? 0xfffff000 : 0x0);
                                printf(", %d \n",auxop2);
                            }
                            else
                                if(auxtipo2==1){
                                    auxop2=(RAM[i])&0x0000000f;
                                    printf(", %s \n",registros[auxop2]);
                                }
                                else{
                                    auxop2=(RAM[i])&0x00000fff;
                                    printf(", [%d] \n",auxop2);
                                }

                            }
        i++;
    }
    printf("Registros: \n");
    printf("DS=\t%11d |     \t            |     \t            |    \t            |\n",DS);
    printf("   \t            | IP= \t%11d |     \t            |    \t            |\n",IP);
    printf("CC=\t%11d | AC= \t%11d | AX= \t%11d | BX= \t%11d |\n",CC,AC,AX,BX);
    printf("CX=\t%11d | DX= \t%11d | EX= \t%11d | FX= \t%11d |\n",CX,DX,EX,FX);


}

void iniciaVectorFunciones()    //LE PASA LAS DIRECCIONES DE LAS FUNCIONES
{
    Funciones[0]=&MOV;
    Funciones[1]=&ADD;
    Funciones[2]=&SUB;
    Funciones[3]=&SWAP;
    Funciones[4]=&MUL;
    Funciones[5]=&DIV;
    Funciones[6]=&CMP;
    Funciones[7]=&SHL;
    Funciones[8]=&SHR;
    Funciones[9]=&AND;
    Funciones[0xA]=&OR;
    Funciones[0xB]=&XOR;
    Funciones[0xF0]=&SYS;
    Funciones[0xF1]=&JMP;
    Funciones[0xF2]=&JZ;
    Funciones[0xF3]=&JP;
    Funciones[0xF4]=&JN;
    Funciones[0xF5]=&JNZ;
    Funciones[0xF6]=&JNP;
    Funciones[0xF7]=&JNN;
    Funciones[0xF8]=&LDL;
    Funciones[0xF9]=&LDH;
    Funciones[0xFA]=&RND;
    Funciones[0xFB]=&NOT;
    Funciones[0xFF1]=&STOP;

}
void setCC(int REG[], int resultado) {
    CC = 0;
    CC = ((resultado == 0) | ((resultado < 0) << 31));
}

void MOV(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]){
    if(tipo1==1){                       // s el tipo1 es del tipo registro
        if(tipo2==0)                    // si el tipo 2 es inmediato
            reg[op1]=op2;
        else if(tipo2==1)
            reg[op1]=reg[op2];
         else
             reg[op1]=RAM[op2+DS];           // si es de tipo registro
    }
    else{
         if(tipo2==0)                       // si el tipo 2 es inmediato
            RAM[op1+DS]=op2;
        else if(tipo2==1)
            RAM[op1+DS]=reg[op2];
         else
             RAM[op1+DS]=RAM[op2+DS];
    }

}

void ADD(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[])  //tipo 1 no puede ser directo, hay que considerar error o eso solo en el traductor?
{
    if (tipo1==1){                                  //el tipo 1 es registro
        if (tipo2==0)
            reg[op1]=reg[op1]+op2;
        else
            if (tipo2==1)
                reg[op1]=reg[op1]+reg[op2];
            else
                reg[op1]=reg[op1]+RAM[op2+DS];
        setCC( reg,reg[op1]);
    }

    else{
        if (tipo2==0)                                   //el tipo 0 es inmediato
            RAM[op1+DS]=RAM[op1+DS]+op2;
        else
            if (tipo2==1)
                RAM[op1+DS]=RAM[op1+DS]+reg[op2];
            else
                RAM[op1+DS]=RAM[op1+DS]+RAM[op2+DS];

        setCC( reg, RAM[op1+DS]);
    }

}


void SUB(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[])
{     if (tipo1==1){                                  //el tipo 1 es registro
        if (tipo2==0)
            reg[op1]=reg[op1]-op2;
        else
            if (tipo2==1)
                reg[op1]=reg[op1]-reg[op2];
            else
                reg[op1]=reg[op1]-RAM[op2+DS];
        setCC( reg,reg[op1]);
    }

    else{
        if (tipo2==0)                                   //el tipo 1 es inmediato
            RAM[op1+DS]=RAM[op1+DS]-op2;
        else
            if (tipo2==1)
                RAM[op1+DS]=RAM[op1+DS]-reg[op2];
            else
                RAM[op1+DS]=RAM[op1+DS]-RAM[op2+DS];

        setCC( reg, RAM[op1+DS]);
    }

}


void SWAP(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[])
{
    int aux;
    if (tipo1==1)                                   //el tipo 1 es registro
        if (tipo2==1)                                //el tipo 2 es registro
        {
            aux=reg[op1];
            reg[op1]=reg[op2];
            reg[op2]=aux;
        }
        else                                        //el tipo 2 es inmediato
        {
            aux=reg[op1];
            reg[op1]=RAM[op2+DS];
            RAM[op2+DS]=aux;
        }
    else                                            //el tipo 1 es inmediato
        if (tipo2==1)
        {
            aux=RAM[op1+DS];
            RAM[op1+DS]=reg[op2];
            reg[op2]=aux;
        }
        else                                        //el tipo 2 es inmediato
        {
            aux=RAM[op1+DS];
            RAM[op1+DS]=RAM[op2+DS];
            RAM[op2+DS]=aux;
        }

}


void MUL(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[])
{
    if (tipo1==1){                                   //el tipo 1 es registro
        if (tipo2==0)
            reg[op1]=reg[op1]*op2;
        else
            if (tipo2==1)
                reg[op1]=reg[op1]*reg[op2];
            else
                reg[op1]=reg[op1]*RAM[op2+DS];
        setCC(reg,reg[op1]);
     }                                                       //puedo usar el operador "*" o debo sumar n veces un valor?
    else{
        if (tipo2==0)                                   //el tipo 1 es inmediato
            RAM[op1+DS]=RAM[op1+DS]*op2;
        else
            if (tipo2==1)
                RAM[op1+DS]=RAM[op1+DS]*reg[op2];
            else
                RAM[op1+DS]=RAM[op1+DS]*RAM[op2+DS];
        setCC(reg,RAM[op1+DS]);
    }
}

void DIV(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[])
{
      if (tipo1==1){                                   //el tipo 1 es registro
           if (tipo2==0 && op2!=0){
             AC=reg[op1]%op2;
             reg[op1]=reg[op1]/op2;
           }
       else if (tipo2==1 && reg[op2]!=0){
                AC=reg[op1]%reg[op2];
                reg[op1]=reg[op1]/reg[op2];
            }
            else
                if(RAM[op2+DS]!=0){
                  AC=reg[op1]%reg[op2];
                  reg[op1]=reg[op1]/RAM[op2+DS];
                }
        setCC(reg,reg[op1]);
       }
    else{
        if (tipo2==0 && op2!=0){
            AC=RAM[op1+DS]%op2;                                    //el tipo 1 es inmediato
            RAM[op1+DS]=RAM[op1+DS]/op2;
        }
        else
            if (tipo2==1 && reg[op2]!=0){
                AC=RAM[op1+DS]%reg[op2];
                RAM[op1+DS]=RAM[op1+DS]/reg[op2];
            }
            else
                if(RAM[op2+DS]!=0)
                 RAM[op1+DS]=RAM[op1+DS]/RAM[op2+DS];
        setCC(reg,RAM[op1+DS]);
    }
}



void CMP(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[])
{
    if (tipo1==1)                                   //el tipo 1 es registro
        if (tipo2==0)
        {
            setCC(reg,reg[op1]-op2);
        }
        else
            if (tipo2==1)
            {
                setCC(reg,reg[op1]-reg[op2]);
            }
            else
            {
                setCC(reg,reg[op1]-RAM[op2]);
            }
    else if (tipo1==0)
        {
            if(tipo2==0)           //el tipo 1 es inmediato
             setCC(reg,op1-op2);
            else if(tipo2==1)
               setCC(reg,op1-reg[op2]);
            else
                setCC(reg,op1-RAM[op2+DS]);
        }
        else{
            if (tipo2==1)
            {
                setCC(reg,RAM[op1+DS]-reg[op2]);
            }
            else if(tipo2==0)
            {

                setCC(reg,RAM[op1+DS]-op2);
            }
            else
                setCC(reg,RAM[op1+DS]-RAM[op2+DS]);
        }

}
void SHL(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]){
       if (tipo1==1){                                   //el tipo 1 es registro
        if (tipo2==0)
            reg[op1]=reg[op1]<<op2;
        else
            if (tipo2==1)
                reg[op1]=reg[op1]<<reg[op2];
            else
                reg[op1]=reg[op1]<<RAM[op2+DS];
        setCC(reg,reg[op1]);
     }
    else{
        if (tipo2==0)                                   //el tipo 0 es inmediato
            RAM[op1+DS]=RAM[op1+DS]<<op2;
        else
            if (tipo2==1)
                RAM[op1+DS]=RAM[op1+DS]<<reg[op2];
            else
                RAM[op1+DS]=RAM[op1+DS]<<RAM[op2+DS];
        setCC(reg,RAM[op1+DS]);
    }
}
void SHR(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]){
    if (tipo1==1){                                   //el tipo 1 es registro
        if (tipo2==0){
                reg[op1]=(reg[op1]>>1)&0x7fffffff;
                reg[op1]=(reg[op1]>>(op2-1));
        }
        else
            if (tipo2==1){
                reg[op1]=(reg[op1]>>1)&0x7fffffff;
                reg[op1]=(reg[op1]>>(reg[op2]-1));
            }
            else{
                reg[op1]=(reg[op1]>>1)&0x7fffffff;
                reg[op1]=(reg[op1]>>(RAM[op2+DS]-1));
            }
        setCC(reg,reg[op1]);
     }
    else{
        if (tipo2==0){
            RAM[op1+DS]=(RAM[op1+DS]>>1)&0x7fffffff;                                 //el tipo 0 es inmediato
            RAM[op1+DS]=(RAM[op1+DS]>>(op2-1));
        }
        else
            if (tipo2==1){
                RAM[op1+DS]=(RAM[op1+DS]>>1)&0x7fffffff;
                RAM[op1+DS]=(RAM[op1+DS]>>(reg[op2]-1));
            }
            else{
                RAM[op1+DS]=(RAM[op1+DS]>>1)&0x7fffffff;
                RAM[op1+DS]=(RAM[op1+DS]>>(RAM[op2+DS]-1));
            }
        setCC(reg,RAM[op1+DS]);
    }
}
void AND (int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]){
       if (tipo1==1){                                   //el tipo 1 es registro
        if (tipo2==0)
            reg[op1]=reg[op1]&op2;
        else
            if (tipo2==1)
                reg[op1]=reg[op1]&reg[op2];
            else
                reg[op1]=reg[op1]&RAM[op2+DS];
        setCC(reg,reg[op1]);
     }
    else{
        if (tipo2==0)                                   //el tipo 0 es inmediato
            RAM[op1+DS]=RAM[op1+DS]&op2;
        else
            if (tipo2==1)
                RAM[op1+DS]=RAM[op1+DS]&reg[op2];
            else
                RAM[op1+DS]=RAM[op1+DS]&RAM[op2+DS];
        setCC(reg,RAM[op1+DS]);
    }
}
void OR(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]){
       if (tipo1==1){                                   //el tipo 1 es registro
        if (tipo2==0)
            reg[op1]=reg[op1]|op2;
        else
            if (tipo2==1)
                reg[op1]=reg[op1]|reg[op2];
            else
                reg[op1]=reg[op1]|RAM[op2+DS];
        setCC(reg,reg[op1]);
     }
    else{
        if (tipo2==0)                                   //el tipo 0 es inmediato
            RAM[op1+DS]=RAM[op1+DS]|op2;
        else
            if (tipo2==1)
                RAM[op1+DS]=RAM[op1+DS]|reg[op2];
            else
                RAM[op1+DS]=RAM[op1+DS]|RAM[op2+DS];
        setCC(reg,RAM[op1+DS]);
    }
}
void XOR(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]){
       if (tipo1==1){                                   //el tipo 1 es registro
        if (tipo2==0)
            reg[op1]=reg[op1]^op2;
        else
            if (tipo2==1)
                reg[op1]=reg[op1]^reg[op2];
            else
                reg[op1]=reg[op1]^RAM[op2+DS];
        setCC(reg,reg[op1]);
     }
    else{
        if (tipo2==0)                                   //el tipo 0 es inmediato
            RAM[op1+DS]=RAM[op1+DS]^op2;
        else
            if (tipo2==1)
                RAM[op1+DS]=RAM[op1+DS]^reg[op2];
            else
                RAM[op1+DS]=RAM[op1+DS]^RAM[op2+DS];
        setCC(reg,RAM[op1+DS]);
    }
}
void SYS(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]){
    int prompt=0,octal=0,hexa=0,decimal=0,carxcar=0, i=0, aux1,dosop=0,x1,x2,endline=0;
    char aux[7];
    char auxaux[7];
    if(op2==1){
            //read
        if(((AX>>11)&0x00000001)==1)        //MUSTRA O NO EL PROMPT
            prompt=0;
        else
            prompt=1;
        if(((AX>>8)&0x00000001)==1)         //LEE CAR A CAR SI ES 1
            carxcar=1;
        else{
            carxcar=0;
            if(((AX>>3)&0x00000001)==1)
               hexa=1;                          //interpreta hexa
            else if(((AX>>2)&0x00000001)==1)    //interpreta octal
                octal=1;
             else
               decimal=1;
        }
        if(carxcar==0)          //LEE CON EL FORMATO CORRESPONDIENTE
         for(i=0 ; i<CX ; i++){  //cantidad numeros a leer
            if(prompt)
                printf("[%04d]",DX+i);
             if(hexa)
                scanf("%08X",&aux1);
             else if(octal)
                  scanf("%o",&aux1);
             else
                 scanf("%d",&aux1);
            RAM[DX+i+DS]=aux1;
         }
        else{
             if(prompt)
               printf("[%04d]",DX);
             scanf("%s",aux);
             for(i=0 ; i<CX ; i++){
                RAM[DX+i+DS]=aux[i];  //ALMACENA CADA CAR DE LA CADENA LEIDA EN POS DISTINTAS
             }

         }

    }
    else if(op2==2){                                //write
            if(((AX>>11)&0x00000001)==1)  //ESCRIBE O NO EL PROMPT
                prompt=0;
            else
                prompt=1;
            if(((AX>>8)&0x00000001)==1)
                endline=1;
            if(((AX>>4)&0x00000001)==1)
                carxcar=1;
            else
                carxcar=0;
            if(((AX>>3)&0x00000001)==1)
                hexa=1;                          //interpreta hexa
            if(((AX>>2)&0x00000001)==1)         //octal
                octal=1;
            if((AX&0x00000001)==1)
                 decimal=10;
            for(i=0 ; i<CX ; i++){          //IMPRIME CON LOS FORMATOS CORRESPONDIENTES
                if(prompt)
                    printf("[%04d]\t",DX+i);
                if(carxcar)
                    printf(" %c ",(RAM[DX+DS+i]&0x000000ff));
                if(hexa)
                {
                    char signo='%';
                    printf("    %c %X \t",signo,RAM[DX+DS+i]);
                }
                if(octal)
                    printf("    @ %o \t",RAM[DX+DS+i]);
                if(decimal)
                    printf("    %d \t",RAM[DX+DS+i]);
                if(!endline)
                    printf("\n");
                }

    }                       //Breakpoints
    else
    {
        if(c)
            system("cls");   //aca se ejecuta solo cuando lee el SYS F
        if(d && p!=1)
            dissasembler();
        if(b && p!=1){              //SI P ES IGUAL A 1, ENTONCES VOLVERA AL EJECUTA A SEGUIR EJECUTANDO EL PASOXPASO
            strcpy(auxaux,"");
            printf("[%04d] cmd:",IP);
            fflush(stdin);
            scanf("%[^\n]",auxaux);
            fflush(stdin);
            strcpy(aux,auxaux);
            strcpy(auxaux,"");
            while(aux[0]!='\0' && aux[0]!='p'){
                if(aux[0]>=48 && aux[0]<=57)
                {
                    convierteINT(aux,&dosop,&x1,&x2);
                    if(dosop)                                   //dos operandos, el else va a 1 operando
                         for(i=x1 ; i<=x2 ; i++){
                            printf("[%04d]\t %04X %04X\t%d ",i,(RAM[i]>>16)&0xffff,RAM[i]&0xffff,RAM[i]);
                            printf("\n");
                    }
                    else
                    {
                        printf("[%04d]\t %04X %04X\t%d ",x1,(RAM[x1]>>16)&0xffff,RAM[x1]&0xffff,RAM[x1]);
                        printf("\n");
                    }
                        /*strcpy(aux," ");
                        system("pause");*/
                }
                strcpy(auxaux,"");
                printf("[%04d] cmd:",IP);
                fflush(stdin);
                scanf("%[^\n]",auxaux);
                fflush(stdin);
                strcpy(aux,auxaux);
                strcpy(auxaux,"");
            }
            if(aux[0]!='p')
                p=0;
            else
                p=1;

        }
    }
}


void RND(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]){
    if(tipo2==1)
            reg[op2]=rand() % (op2+1);
    else
         RAM[op2+DS]=rand() % (op2 + 1);

}
void NOT(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]){
    if(tipo2==1){
        reg[op2]=~(reg[op2]);
        setCC(reg,reg[op2]);
    }
    else{
        RAM[op2+DS]=~(RAM[op2+DS]);
        setCC(reg,RAM[op2+DS]);
    }
}
void JMP(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]){
    if(tipo2==0)
     IP=op2;
    else if(tipo2==1)
        IP=reg[op2];
    else
        IP=RAM[op2];
}
void JZ(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]){
    if (CC & 0b1)
        JMP(tipo1, tipo2,op1,op2,RAM, reg);
}
void JP(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]) {
    if (CC == 0)
        JMP(tipo1, tipo2,op1,op2,RAM, reg);
}

void JN(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]) {
    if (CC & (0b1 << 31))
        JMP(tipo1, tipo2,op1,op2,RAM, reg);


}

void JNZ(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]) {
    if (!(CC & 0b1))
         JMP(tipo1, tipo2,op1,op2,RAM, reg);


}

void JNP(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]) {
    if (CC != 0)
         JMP(tipo1, tipo2,op1,op2,RAM, reg);

}

void JNN(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]) {
    if (!(CC & (0b1 << 31)))
         JMP(tipo1, tipo2,op1,op2,RAM, reg);

}
void LDL(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]){
    if(tipo2==0)
        AC+=op2&0x0000ffff;
    else if(tipo1==1)
        AC+=reg[op2]&0x0000ffff;
        else
            AC+=RAM[op2+DS]&0x0000ffff;
}
void LDH(int tipo1, int tipo2, int op1, int op2, int RAM[], int reg[]){
    if(tipo2==0){
        AC+=(op2<<16)&0xffff0000;
    }
    else if(tipo2==1){
        AC+=(reg[op2]<<16)&0xffff0000;
    }
        else{
            AC+=(RAM[op2+DS]<<16)&0xffff0000;
        }
}
void STOP(int tipo1 ,int tipo2,int op1, int op2, int REG[], int RAM[]) {
    IP = DS * 2;
}

//-----------------------FLAGS---------------------------

void AnalizaFlags(int argc,char *argv[])
{
    int i;
    for(i=2;i<argc;i++)
    {
        if(argv[i][1] == 'b')
            b=1;
        else if(argv[i][1] == 'c')
            c=1;
        else if(argv[i][1] == 'd')
            d=1;
    }
}


//------------------FUNCIONES AUXILIARES------------------

void convierteINT(char x[8],int *dosop,int *x1,int *x2)
{
    int j=0;
    char v1[4];
    char v2[4];
    int i=0;
    *dosop=0;
    while (x[i]!=' ' && i<strlen(x))
    {
        v1[i]=x[i];
        i++;
    }
    if (i<strlen(x))
    {
        i++;
        while(i<strlen(x))
        {
            v2[j]=x[i];
            i++;
            j++;
        }
        *x2=atoi(v2);
        *dosop=1;
    }
    *x1=atoi(v1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void diccionario()
{
    strcpy(mnemonicos[0x0], "MOV");         //ASIGNA MNEMONICOS
    strcpy(mnemonicos[0x1], "ADD");
    strcpy(mnemonicos[0x2], "SUB");
    strcpy(mnemonicos[0x3], "SWAP");
    strcpy(mnemonicos[0x4], "MUL");
    strcpy(mnemonicos[0x5], "DIV");
    strcpy(mnemonicos[0x6], "CMP");
    strcpy(mnemonicos[0x7], "SHL");
    strcpy(mnemonicos[0x8], "SHR");
    strcpy(mnemonicos[0x9], "AND");
    strcpy(mnemonicos[0xA], "OR");
    strcpy(mnemonicos[0xB], "XOR");
    strcpy(mnemonicos[0xF0], "SYS");
    strcpy(mnemonicos[0xF1], "JMP");
    strcpy(mnemonicos[0xF2], "JZ");
    strcpy(mnemonicos[0xF3], "JP");
    strcpy(mnemonicos[0xF4], "JN");
    strcpy(mnemonicos[0xF5], "JNZ");
    strcpy(mnemonicos[0xF6], "JNP");
    strcpy(mnemonicos[0xF7], "JNN");
    strcpy(mnemonicos[0xF8], "LDL");
    strcpy(mnemonicos[0xF9], "LDH");
    strcpy(mnemonicos[0xFA], "RND");
    strcpy(mnemonicos[0xFB], "NOT");
    strcpy(mnemonicos[0xFF1], "STOP");


    strcpy(registros[0],"DS");      //ASIGNA REGISTROS
    strcpy(registros[1],"");
    strcpy(registros[2],"");
    strcpy(registros[3],"");
    strcpy(registros[4],"");
    strcpy(registros[5],"IP");
    strcpy(registros[6],"");
    strcpy(registros[7],"");
    strcpy(registros[8],"CC");
    strcpy(registros[9],"AC");
    strcpy(registros[10],"AX");
    strcpy(registros[11],"BX");
    strcpy(registros[12],"CX");
    strcpy(registros[13],"DX");
    strcpy(registros[14],"EX");
    strcpy(registros[15],"FX");
}
