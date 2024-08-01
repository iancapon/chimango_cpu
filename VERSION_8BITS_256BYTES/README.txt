SOLO IMPLEMENTA LA LÓGICA por medio de MICROCODIGO, NO LA ELECTRÓNICA NI EL BUS..

--------------------------------------------------

             (8b)     |PC|<---->||<---->|A|   (8b)
                                ||       v
             (8b)    |MAR|<-----||<-----|ALU| (8b)
                       v        ||       ^
        (256Bytes)   |MEM|<---->||<---->|B|   (8b)
                       ^        ||
             (16b)  |IRxy|<---->||
                       v        ||<---->|IO|  (2bytes mapeados a memoria)
                   |CONTROL|    ||
                                ||

INSTRUCTION_REGISTER :  xxxxxxxx|yyyyyyyy (16bits)
                        instr       argum


MICROOPERACIONES (CONTROL):

0   *PC_out
1   *PC_in
2   *PC_enable
3   *MAR_in
4   *MEM_out
5   *MEM_in
6   *IRY_out 
7   *IRX_in 
8   *IRY_in
9   *A_out
10  *A_in
11  *B_out
12  *B_in
13  *ALU_add
14  *ALU_sub
15  *ALU_out


EJEMPLOS DE CODIGO :

----MULTIPLICADOR----

*x == &FD
*y == &FE
*z == &FF //output mapped

loop:
    LDA x
    LDB z
    ADD
    STA z
    LDA y
    WRB 1
    SUB
    STA y
    JMZ stop
    JMP loop
stop:
    HALT

--------------------