#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOP 0

#define LDA 1
#define WRA 2
#define STA 3

#define LDB 4
#define WRB 5
#define STB 6

#define ADD 7 // SIN ARGUMENTOS; guarda en el A_REG
#define SUB 8 // SIN ARGUMENTOS; guarda en el A_REG

#define JMP 9

#define JMZ 10 // jump if zero
#define JMC 11 // jump if carry

#define HALT 255


uint8_t pc;
uint8_t mar;
uint8_t mem[256]={0};
uint16_t ir;

uint8_t a_reg;
uint8_t b_reg;
uint8_t alu;

uint32_t clock;

uint8_t flags;

void reset();
void fetch_cycle();
void load_reg(uint8_t*reg);
void write_reg(uint8_t*reg);
void store_reg(uint8_t*reg);
void add();
void sub();
void jump();
void jump_zero();
void jump_carry();
void decimal_to_hex(int x);
void peak_memory();

int main(){
    reset();
    int i=0;
    mem[i]=LDA; i++;
    mem[i]=253; i++;
    mem[i]=LDB; i++;
    mem[i]=255; i++;
    mem[i]=ADD; i++;
    mem[i]=STA; i++;
    mem[i]=255; i++;
    mem[i]=LDA; i++;
    mem[i]=254; i++;
    mem[i]=WRB; i++;
    mem[i]=1; i++;
    mem[i]=SUB; i++;
    mem[i]=STA; i++;
    mem[i]=254; i++;
    mem[i]=JMZ; i++;
    mem[i]=250; i++;
    mem[i]=JMP; i++;
    mem[i]=0; i++;
    mem[250]=HALT;
    mem[253]=5;
    mem[254]=3;
    mem[255]=0;

    ////////////////
    char run='\n';
    while(run=='\n'){
        peak_memory();
        fetch_cycle();
        switch((ir>>8)&255){
            case NOP:
                clock+=5;
            break;
            /////////
            case LDA:
                load_reg(&a_reg);
            break;
            case WRA:
                write_reg(&a_reg);
            break;
            case STA:
                store_reg(&a_reg);
            break;
            /////////
            case LDB:
                load_reg(&b_reg);
            break;
            case WRB:
                write_reg(&b_reg);
            break;
            case STB:
                store_reg(&b_reg);
            break;
            /////////
            case ADD:
                add();
            break;
            case SUB:
                sub();
            break;
            /////////
            case JMP:
                jump();
            break;
            case JMZ:
                jump_zero();
            break;
            case JMC:
                jump_carry();
            break;
            case HALT:
                run=0;
                printf("\n - HALTED - ");                
            break;
            default:
                clock+=1;//si no reconoce pasa a la siguiente inmediatamente
            break;
        }
        //
        if(run=='\n'){
            run=getchar();
        }
    }
    return 0;
}

void decimal_to_hex(int x){
    char*index="0123456789ABCDEF";
    char hex[3]={0};
    int i=1;
    hex[0]='0';
    hex[1]='0';
    while(x>0){
        hex[i]=index[x%16];
        x/=16;
        i--;
    }
    printf("%2s",hex);
}

void peak_memory(){
    printf("\n - OUTPUT:%d - I_REG(x): %d - I_REG(y): %d - A_REG: %d - B_REG: %d",mem[255],(ir>>8)&255,ir&255,a_reg,b_reg);
    printf(" - ALU: %d - FLAGS: %d - CLOCK: %d\n",alu,flags,clock);
    for(int i=0; i< 16;i++){
        if(i+16*(pc/16) == pc){
            printf(">  ");
            decimal_to_hex(i+16*(pc/16));
            printf(" : ");
            decimal_to_hex(mem[i+16*(pc/16)]);
            printf(" = %3d\n",mem[i+16*(pc/16)]);
        }
        else{
            printf("   ");
            decimal_to_hex(i+16*(pc/16));
            printf(" : ");
            decimal_to_hex(mem[i+16*(pc/16)]);
            printf(" = %3d\n",mem[i+16*(pc/16)]);
        }
    }
}

void jump_carry(){
    /*IF CONDITION MET
        * PCout; MARin
        * RAMout; JUMP
    */
    if((flags&0b00000010)!=0){
        mar=pc;
        pc=mem[mar];
        clock+=2;
        flags=flags&253;//reset flag
    }else{
        ///skip * PC enable
        pc++;
    }
}

void jump_zero(){
    /*IF CONDITION MET
        * PCout; MARin
        * RAMout; JUMP
    */
    if((flags&0b00000001)!=0){
        mar=pc;
        pc=mem[mar];
        clock+=2;
        flags=flags&254;//reset flag
    }else{
        //skip *PC enable
        pc++;
    }
}

void jump(){
    /*
        * PCout; MARin
        * RAMout; JUMP
    */
    mar=pc;
    pc=mem[mar];
    clock+=2;
}

void add(){
    /*
        ALUadd;
        ALUout; A_REGin
    */
    alu=a_reg+b_reg;
    a_reg=alu;
    clock+=2;
    if((alu)==0){
        flags|=0b00000001;
    }
    else if((a_reg+b_reg)>255){
        flags|=0b00000010;
    }
}

void sub(){
    /*
        ALUsub;
        ALUout; A_REGin
    */
    alu=a_reg-b_reg;
    a_reg=alu;
    clock+=2;
    if((alu)==0){
        flags|=0b00000001;
    }
    else if((a_reg-b_reg)<0){
        flags|=0b00000010;
    }
}

void store_reg(uint8_t*reg){
    /*
        * PCout; MARin
        * RAMout; IRin (byte MENOS significante)
        * MARin; IRout (byte MENOS significante)
        * X_REGout; RAMin;
        * PC enable
    */
   //printf(" -D{MAR:%d , REG:%d}D- ",(ir | mem[pc])&255,*reg);
    mar=pc;
    ir=ir | mem[mar];
    mar=ir&255;
    mem[mar]=*reg;
    pc++;
    clock+=5;
    
}

void write_reg(uint8_t*reg){
     /*
        * PCout; MARin
        * X_REGin; RAMout;
        * PC enable
    */
    mar=pc;
    *reg=mem[mar];
    pc++;
    clock+=3;
}

void load_reg(uint8_t*reg){
    /*
        * PCout; MARin
        * RAMout; IRYin (byte MENOS significante)
        * MARin; IRYout (byte MENOS significante)
        * X_REGin; RAMout;
        * PC enable
    */
    mar=pc;
    ir=ir | mem[mar];
    mar=ir&255;
    *reg=mem[mar];
    pc++;
    clock+=5;
}

void fetch_cycle(){
    /*
        * PCout; MARin
        * RAMout; IRXin (byte MÁS significante)
        * PC enable
    */
    mar=pc;
    ir=mem[mar]<<8;
    pc++;
    clock+=3;
}

void reset(){
    pc=0;
    mar=0;
    ir=0;
    a_reg=0;
    b_reg=0;
    alu=0;
    clock=0;
    flags=0;
}