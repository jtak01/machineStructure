/*
 * umlab.c
 *
 * Functions to generate UM unit tests. Once complete, this module
 * should be augmented and then linked against umlabwrite.c to produce
 * a unit test writing program.
 *  
 * A unit test is a stream of UM instructions, represented as a Hanson
 * Seq_T of 32-bit words adhering to the UM's instruction format.  
 * 
 * Any additional functions and unit tests written for the lab go
 * here. 
 *  
 */


#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <seq.h>
#include <bitpack.h>


typedef uint32_t Um_instruction;
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;


/* Functions that return the two instruction types */

Um_instruction three_register(Um_opcode op, int ra, int rb, int rc);
Um_instruction loadval(unsigned ra, unsigned val);


/* Wrapper functions for each of the instructions */

static inline Um_instruction halt(void) 
{
        return three_register(HALT, 0, 0, 0);
}

typedef enum Um_register { r0 = 0, r1, r2, r3, r4, r5, r6, r7 } Um_register;

static inline Um_instruction add(Um_register a, Um_register b, Um_register c) 
{
        return three_register(ADD, a, b, c);
}
static inline Um_instruction div(Um_register a, Um_register b, Um_register c) 
{
    return three_register(DIV, a, b, c);

}
static inline Um_instruction nand(Um_register a, Um_register b, Um_register c) 
{
    return three_register(NAND, a, b, c);
}
static inline Um_instruction mul(Um_register a, Um_register b, Um_register c)
{
    return three_register(MUL, a, b, c);
}

static inline Um_instruction output(Um_register c)
{
        return three_register(OUT, 0, 0, c);
}

/* Functions for working with streams */

static inline void append(Seq_T stream, Um_instruction inst)
{
        assert(sizeof(inst) <= sizeof(uintptr_t));
        Seq_addhi(stream, (void *)(uintptr_t)inst);
}

const uint32_t Um_word_width = 32;

void Um_write_sequence(FILE *output, Seq_T stream)
{
        assert(output != NULL && stream != NULL);
        int stream_length = Seq_length(stream);
        for (int i = 0; i < stream_length; i++) {
                Um_instruction inst = (uintptr_t)Seq_remlo(stream);
                for (int lsb = Um_word_width - 8; lsb >= 0; lsb -= 8) {
                        fputc(Bitpack_getu(inst, 8, lsb), output);
                }
        }
      
}

Um_instruction three_register(Um_opcode op, int ra, int rb, int rc)
{
        uint32_t instruction = 0;
        instruction = Bitpack_newu(instruction, 4, 28, op);
        instruction = Bitpack_newu(instruction, 3, 6, ra);
        instruction = Bitpack_newu(instruction, 3, 3, rb);
        instruction = Bitpack_newu(instruction, 3, 0, rc);
        return instruction;
}

Um_instruction loadval(unsigned ra, unsigned val)
{
        uint32_t inst = 0;
        inst = Bitpack_newu(inst, 4, 28, 13);
        inst = Bitpack_newu(inst, 3, 25, ra);
        inst = Bitpack_newu(inst, 25, 0, val);
        return inst;
}


/* Unit tests for the UM */

void build_halt_test(Seq_T stream)
{
        append(stream, halt());
}

void build_verbose_halt_test(Seq_T stream)
{
        append(stream, halt());
        append(stream, loadval(r1, 'B'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'a'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'd'));
        append(stream, output(r1));
        append(stream, loadval(r1, '!'));
        append(stream, output(r1));
        append(stream, loadval(r1, '\n'));
        append(stream, output(r1));
}

void build_add_test(Seq_T stream)
{
        append(stream, loadval(r1, 1));
        append(stream, loadval(r2, 60));
        append(stream, loadval(r3, 9));
        
        append(stream, add(r1, r2, r3));
        append(stream, output(r1));
        append(stream, halt());
}

void build_print_six_test(Seq_T stream)
{
        append(stream, loadval(r1, 48));
        append(stream, loadval(r2, 6));
        append(stream, add(r3, r1, r2));
        append(stream, output(r3));
        append(stream, halt());
}

void build_mul_test(Seq_T stream) 
{
    append(stream, loadval(r1, 12));
    append(stream, loadval(r2, 3));
    append(stream, mul(r3, r1, r2));
    append(stream, output(r3));
    append(stream, halt());
}

void build_div_test(Seq_T stream) 
{
    append(stream, loadval(r1, 72));
    append(stream, loadval(r2, 2));
    append(stream, div(r3, r1, r2));
    append(stream, output(r3));
    append(stream, halt());
}

//This test was used to check the bitwise nand operation. 
//NOTE: Does not print character, output must be changed "%u"
void build_nand_test(Seq_T stream)
{
  append(stream, loadval(r1, 33554431));
  append(stream, loadval(r2, 33554431)); 
  append(stream, nand(r3, r1, r2));
  append(stream, halt());
}
