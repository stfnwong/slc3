/* OPCODE
 * Opcodes for the LC3 
 * Stefan Wong 2018
 */

#ifndef __OPCODE_H
#define __OPCODE_H

// TODO : note here that for immediate mode we have
// to check bit 5 in ADD and AND 

#define LC3_ADD  0x01
#define LC3_AND  0x05
#define LC3_BR   0x00
//#define LC3_JMP  0x0C
#define LC3_JSR  0x04
#define LC3_LD   0x02
#define LC3_LDI  0x0A
#define LC3_LDR  0x06
#define LC3_LEA  0x0E
#define LC3_NOT  0x09
//#define LC3_RET  0x0C   
#define LC3_RTI  0x08
#define LC3_ST   0x03
#define LC3_STI  0x0B
#define LC3_STR  0x07
#define LC3_TRAP 0x0F
#define LC3_RES  0x0D

#define LC3_JMP_RET 0x0C

#endif /*__OPCODE_H*/
