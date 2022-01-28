#pragma once

#include <stddef.h>


typedef enum
{
    OPCode_Invalid = -1,
    OPCode_MOV,
    OPCode_ADD,
    OPCode_SUB,
    OPCode_AND,
    OPCode_OR,
    OPCode_XOR,
    OPCode_SHL,
    OPCode_SHR,
    OPCode_NOT,
    OPCode_MUL,
    OPCode_DIV,
    OPCode_MULH,
    OPCode_MULQ,
    OPCode_INVQ,
    OPCode_DIVS,
    OPCode_PER,
 
    // Pseudo-Opcodes
    OPCode_JMP,
    OPCode_NOP
} OPCode;

typedef enum
{
    Source_Invalid = -1,
    Source_RZ,
    Source_R0,
    Source_R1,
    Source_R2,
    Source_R3,
    Source_R4,
    Source_R5,
    Source_R6,
    Source_R7,
    Source_RAM,
    Source_IP,
    Source_SP,
    Source_IN,
    Source_OUT,
    Source_LIT16,
    Source_LIT8
} Source;

typedef enum
{
    Dst_Invalid = -1,
    Dst_RZ,
    Dst_R0,
    Dst_R1,
    Dst_R2,
    Dst_R3,
    Dst_R4,
    Dst_R5,
    Dst_R6,
    Dst_R7,
    Dst_RAM,
    Dst_IP,
    Dst_SP,
    Dst_PSEL,
    Dst_OUT,
    Dst_PER,
} Dst;

typedef enum
{
    Addr_Invalid = -1,
    Addr_RZ,
    Addr_R0,
    Addr_R1,
    Addr_R2,
    Addr_R3,
    Addr_R4,
    Addr_R5,
    Addr_R6,
    Addr_R7,
    Addr_SP,
    Addr_SP_MINUS,
    Addr_SP_INC,
    Addr_SP_PLUS,
    Addr_SP_DEC,
    Addr_LIT16 = 15
} Addr;

typedef enum
{
    Cond_Invalid = -1,
    Cond_HLT = 14,
    Cond_Z = 1,
    Cond_NZ = 3,
    Cond_P = 5,
    Cond_NP = 7,
    Cond_S = 9,
    Cond_NS = 11,
    Cond_C = 13,
    Cond_NC = 15,
} Cond;


OPCode ParseOPCode(char* code, size_t* i);
Source ParseSource(char* code, size_t* i);
Dst ParseDst(char* code, size_t* i);
Addr ParseAddr(char* code, size_t* i);
Cond ParseCond(char* code, size_t* i);