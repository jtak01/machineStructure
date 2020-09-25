/*
 * Name: Jun Sung Tak (jtak) and Nilay Maity (nmaity01)
 * hw6
 */

#include <math.h>
#include <assert.h>
#include "um_executor.h"
#include "bitpack.h"
typedef struct Instruction Instruction;
struct Instruction {
        unsigned op;
        unsigned regA;
        unsigned regB;
        unsigned regC;
};

/*
 * UM operation declarations
 */
void conditional_move(unsigned *reg, Instruction inst);
void segmented_load(unsigned *reg, Instruction inst, Seg_T mem);
void segmented_store(unsigned *reg, Instruction inst, Seg_T mem);
void halt(Seg_T prgm);
void output(unsigned *reg, Instruction inst);
void load_val(unsigned *reg, unsigned index, uint32_t val);
void add(unsigned *reg, Instruction inst);
void multiplication(unsigned *reg, Instruction inst);
void division(unsigned *reg, Instruction inst);
void bitwise_nand(unsigned *reg, Instruction inst);
void map_segment(unsigned *reg, Instruction inst, Seg_T mem);
void unmap_segment(unsigned *reg, Instruction inst, Seg_T mem);
void input(unsigned *reg, Instruction inst);
void load_prgm(unsigned *reg, Instruction inst, Seg_T mem, 
               unsigned *prgm_ctr);

/*               
 * Parameters: uint32_t word Instruction inst
 *    Returns: Instruction
 *       Does: Gets the neccessary data from word and make a instruction to
 *             execute.
 */
Instruction build_instruction(uint32_t word, Instruction inst)
{
    inst.op = Bitpack_getu(word, 4, 28);
    if(inst.op == 13){
        inst.regA = Bitpack_getu(word, 3, 25);
        inst.regB = Bitpack_getu(word, 25, 0);
        return inst;
    }
    inst.regA = Bitpack_getu(word, 3, 6);
    inst.regB = Bitpack_getu(word, 3, 3);
    inst.regC = Bitpack_getu(word, 3, 0);
    return inst;
}

/*               
 * Parameters: Seg_T prgm, Instruction inst, unsigned *reg, unsigned *prgm_ctr
 *    Returns: None
 *    Does: Long switch statement for the different operations
 */
void run(Seg_T prgm, Instruction inst, unsigned *reg, unsigned *prgm_ctr)
{
    assert(inst.op < 14);
    int operation = inst.op;
    switch(operation)
    {
        case 0:
            conditional_move(reg, inst);
            break;
        case 1:
            segmented_load(reg, inst, prgm);
            break;
        case 2: 
            segmented_store(reg, inst, prgm);
            break;
        case 3:
            add(reg, inst);
            break;
        case 4:
            multiplication(reg, inst);
            break;
        case 5:
            division(reg, inst);
            break;
        case 6:
            bitwise_nand(reg, inst);
            break;
        case 7: 
            halt(prgm);
            break;
        case 8:
            map_segment(reg, inst, prgm);
            break;
        case 9: 
            unmap_segment(reg, inst, prgm);
            break;
        case 10:
            output(reg, inst);
            break;
        case 11:
            input(reg, inst);
            break;
        case 12:
            load_prgm(reg, inst, prgm, prgm_ctr);
            break;
        case 13: 
            load_val(reg, inst.regA, inst.regB); 
            break;
    }
}

/*               
 * Parameters: Seg_T program, unsigned *regs
 *    Returns: None
 *       Does: Loop that will execute all instructions in um files
 */
extern void um_exec(Seg_T program, unsigned *regs)
{
        uint32_t word;
        Instruction instruction;
        unsigned prgm_ctr = 0;        
        while(1){
            word = Seg_load(program, prgm_ctr, 0);
            instruction = build_instruction(word, instruction);
            run(program, instruction, regs, &prgm_ctr);
            prgm_ctr++;
        }
}

/*
 * UM operations
 */
void conditional_move(unsigned *reg, Instruction inst)
{
    if(reg[inst.regC] != 0)
        reg[inst.regA] = reg[inst.regB];
}

void segmented_load(unsigned *reg, Instruction inst, Seg_T mem)
{
    reg[inst.regA] = Seg_load(mem, reg[inst.regC], reg[inst.regB]);
}

void segmented_store(unsigned *reg, Instruction inst, Seg_T mem)
{
    Seg_store(mem, reg[inst.regC], reg[inst.regA], reg[inst.regB]);
}

void add(unsigned *reg, Instruction inst)
{
    reg[inst.regA] = reg[inst.regB] + reg[inst.regC];
}

void multiplication(unsigned *reg, Instruction inst)
{
    reg[inst.regA] = reg[inst.regB] * reg[inst.regC];
}

void division(unsigned *reg, Instruction inst)
{
    reg[inst.regA] = reg[inst.regB] / reg[inst.regC];
}

void bitwise_nand(unsigned *reg, Instruction inst)
{
    reg[inst.regA] = ~(reg[inst.regB] & reg[inst.regC]);
}

void map_segment(unsigned *reg, Instruction inst, Seg_T mem)
{
    unsigned location = Seg_map(mem, reg[inst.regC]);
    reg[inst.regB] = location;
}

void unmap_segment(unsigned *reg, Instruction inst, Seg_T mem)
{
    Seg_unmap(mem, reg[inst.regC]);
}

void output(unsigned *reg, Instruction inst)
{
    unsigned to_print = reg[inst.regC]; 
    assert(to_print <= 255);
    printf("%c", to_print);
}

void input(unsigned *reg, Instruction inst)
{
    reg[inst.regC] = fgetc(stdin);
}

void halt(Seg_T prgm)
{
    Seg_free(prgm);
    exit(0);
}

void load_val(unsigned *reg, unsigned index, uint32_t val)
{
    reg[index] = val;
}

void load_prgm(unsigned *reg, Instruction inst, Seg_T mem, unsigned *counter)
{
    if(reg[inst.regB] != 0)
        Seg_load_prgm(mem, reg[inst.regB]);
    *counter = reg[inst.regC] - 1;
}



