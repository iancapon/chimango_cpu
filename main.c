#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/*
    FORMATO DE OPERACIONES
    x x x x  |  y y y y
       oper  |  arg

    en el programa.txt:
    A: X Y\n
    donde A-> linea en hex menor a 16.
    donde X-> operacion (puede ser o no con NOMBRE EN MAYUS)
    donde Y-> argumento de la operacion (en decimal o $hex)
*/

/////////////////////////// OPERACIONES
const uint8_t NOP= 0x00<<4;
const uint8_t LDA= 0x01<<4;
const uint8_t ADD= 0x02<<4;
const uint8_t SUB= 0x03<<4;
const uint8_t STA= 0x04<<4;
const uint8_t JMP= 0x05<<4;
const uint8_t JMC= 0x06<<4;
const uint8_t JMZ= 0x07<<4;
const uint8_t OUT= 0x08<<4;
const uint8_t HLT= 0x09<<4;
const uint8_t LDI= 0x0a<<4;

const char OPS[][4]={
    "NOP",///no hace nada
    "LDA",///load A register
    "ADD",///load B regitrer y suma, guarda en A register
    "SUB",///load B regitrer y resta, guarda en A register
    "STA",///store in memory from A register
    "JMP",///jump sin condicion
    "JMC",///jump con carry bit 1
    "JMZ",///jump con zero bit 1
    "OUT",///store en output register from memory
    "HLT",///halt
    "LDI"////store numero de 4 bits en register
};

uint8_t PC= 0x00;//PROGRAM COUNTER de 4 bits MAXIMO
uint8_t AR= 0x00;// A_ REGISTER
uint8_t BR= 0x00;// B_ REGISTER
uint8_t OR= 0x00;// OUT_ REGISTER
uint8_t FL= 0x00;// FLAGS_ REGISTER

uint8_t * memory;//arreglo de la memoria

void printMemory(){
    printf("REG_A: %3d | REG_B: %3d | FLAGS: %3d | OUTPUT: %3d | P_COUNTER: %2x \n\n",AR,BR,FL,OR,PC);
    for(int i=0; i< 16; i++){
        if(i==PC){
            printf(">  %2x: ",i);
        }else{
            printf("   %2x: ",i);
        }
        if((memory[i]>>4) < 11){
            printf("%s $%x ",OPS[(memory[i]>>4)],memory[i]&0b00001111);
        }else{
            printf("$%5x ",memory[i]);
        }
        printf("| %4d\n",memory[i]);
    }
    printf("\n\n");
}

void appendChar(char*text,int*pointer,char c){
    text=realloc(text,sizeof(char)*(*pointer+2));
    text[*pointer+1]=0;
    text[*pointer]=c;
    *pointer+=1;
}

char* cargarDesdeArchivo(){
    FILE*file;
    file = fopen("programa.txt", "r");
    /////
    char*input=malloc(sizeof(char));
    int pointer=0;
    input[pointer]=0;
    /////
    if (file == NULL) {
        printf("No se puede abrir el archivo");
    }
    else{
        char c=fgetc(file);
        while(c!=EOF){
            appendChar(input,&pointer,c);
            c=fgetc(file);
        }
    }
    fclose(file);
    return input;
}

uint8_t matchOp(char*word){
    uint8_t i=0;
    while (i<11){
        if(strcmp(OPS[i],word)==0){
            return i;
        }
        i++;
    }
    return 255;
}

int hexToDecChar(char c){
    int ret =(int)c;
    if(ret>=48 && ret<=57){
        ret-=48;
    }else if(ret>=97 && ret<=102){
        ret-=87;
    }
    //printf(" DEBUG:%d\n",ret);
    return ret;
}

int hexToDecStr(char*word){
    int i=strlen(word)-1;
    int mult=1;
    int ret=0;
    while (i>0){
        ret+=hexToDecChar(word[i])*mult;
        mult*=16;
        i--;
    }
    return ret;
}

void parsearTexto(char*input){
    int i=0;
    char*word=malloc(sizeof(char));
    int w=0;
    int pc=0;
    int xx=0;
    int yy=0;
    while (i<strlen(input)){
        if(input[i]!=' ' && input[i]!='\n'){
            appendChar(word,&w,input[i]);
        }
        else{
            if(word[1]==':'){
                pc=hexToDecChar(word[0]);
                //printf("pc->%d",(uint8_t)pc);
            }
            else if(word[0]=='$'){
                yy=hexToDecStr(word);
                //printf("yy->%d",(uint8_t)yy);
            }
            else if(matchOp(word)!=255){
                xx=matchOp(word);
                //printf("xx->%d",(uint8_t)xx);
            }
            else{
                yy=atoi(word);
                //printf("yy->%d",(uint8_t)yy);
            }
            //printf(":%s\n",word);
            w=0;
            if(input[i]=='\n'){
                //printf(" RES: %d\n",((uint8_t)xx<<4 ) + (uint8_t)yy);
                memory[(uint8_t)pc]=((uint8_t)xx<<4 ) + (uint8_t)yy;
            }
        }
        i++;
    }
    free(word);
}


void run(uint16_t*clk){
    uint8_t x=memory[PC]&0b11110000;
    uint8_t y=memory[PC]&0b00001111;

    switch(x){
        case NOP:
            PC++;
        break;
        case LDA:
            AR=memory[y];
            PC++;
        break;
        case LDI:
            AR=y;
            PC++;
        break;
        case ADD:
            BR=memory[y];
            AR+=BR;
            if(AR+BR>255){
                FL+=0b10000000;
            }
            PC++;
        break;
        case SUB:
            BR=memory[y];
            AR-=BR;
            if(AR-BR<0){
                FL+=0b01000000;
            }
            PC++;
        break;
        case STA:
            memory[y]=AR;
            PC++;
        break;
        case JMP:
            PC=y;
        break;
        case JMC:
            PC++;
            if((FL& 0b10000000)==0b10000000){
                PC=y;
                FL-=0b10000000;
            }
        break;
        case JMZ:
            PC++;
            if((FL& 0b01000000)==0b01000000){
                PC=y;
                FL-=0b01000000;
            }
        break;
        case OUT:
            OR=memory[y];
            PC++;
        break;
        case HLT:
        break;
        default:
        PC++;
        break;
    }
    *clk+=3;
}

int main(){
    uint16_t clock_pulse=0;
    memory=(uint8_t*)malloc(sizeof(uint8_t)*16);
    char*text=cargarDesdeArchivo();    
    parsearTexto(text);
    //printMemory();
    free(text);
    char c='\n';
    while(c=='\n'){
        printf("\n CLK: %d\n",clock_pulse);
        printMemory();
        run(&clock_pulse);
        c=getchar();
        clock_pulse++;
    }
    return 0;
}

/*void parsearTexto(char*input){
    int i=0;
    uint8_t pc=0;
    uint8_t op=0;
    ///
    char*word=malloc(sizeof(char));
    int p=0;
    word[p]=0;
    ///
    do{
        if(input[i]!=' ' && input[i]!='\n'){
            appendChar(word,&p,input[i]);
        }
        else{
            //printf("%s - ",word);
            if(word[1]==':'){/// linea n°
                if(word[0]<=57 && word[0]>=48){
                    pc=word[0]-48;
                }else if(word[0]<=102 && word[0]>=97){
                    pc=word[0]-97;
                }
            op=0;
            printf("%d: ",pc);
            }
            else if(matchOp(word)!=255){
                op+=((0+matchOp(word))<<4)&0b11110000;
                printf("%s ",OPS[matchOp(word)]);
                printf("%d :: ",matchOp(word)<<4);
            }
            else if(word[0]=='$'){
                char aux[4]={0};
                int j=1;
                while(word[j]!=0){
                    aux[j-1]=word[j];
                    j++;
                }
                //printf("%d\n",atoi(aux));
                op+=atoi(aux);
                
            }else{
                //printf("%d\n",atoi(word));
                op+=atoi(word);
                printf("%d ",op);
                p=0;
                memory[pc]=op;
                op=0;
            }
            //printf("%d\n",op);
            p=0;
            
        }
        i++;
    } while(input[i]!=0);

    free(word);
}*/