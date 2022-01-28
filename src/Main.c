#include "GenericList.h"
#include "Lexer.h"
#include "Disassembler.h"
#include "Util.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

char* ReadFileAsString(char* path, size_t* size)
{
    FILE* f = fopen(path, "rb");
    if (f == NULL)
    {
        size = 0;
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* code = xmalloc(fsize + 1);
    fread(code, 1, fsize, f);
    fclose(f);
    code[fsize] = 0; // Null terminate

    *size = (uint32_t)fsize;

    return code;
}

void ExitWithError(char* error)
{
    printf("error: %s\n", error);
    exit(1);
}

void SkipIndent(char* code, size_t codeLen, size_t* i)
{
    while ((code[*i] == ' ' || code[*i] == '\t') && *i < codeLen)
        (*i)++;
}

void SkipWhitespace(char* code, size_t codeLen, size_t* i)
{
    while (isspace(code[*i]) && *i < codeLen)
        (*i)++;
}

void SkipComment(char* code, size_t codeLen, size_t* i)
{
    if (code[*i] == ';')
        while (code[*i] != '\n' && *i < codeLen)
            (*i)++;
}

typedef struct
{
    char* id;
    uint16_t addr;
} Label;
GenericList labels;

char* ReadNextIdentifier(char* iCode, size_t* ioI, size_t codeLen)
{
    size_t curBufLen = 32;
    char* buf = xmalloc(curBufLen);
    size_t j = 0;
    while (*ioI < codeLen)
    {
        if (!isalpha(iCode[*ioI]) && !isdigit(iCode[*ioI]) && iCode[*ioI] != '_')
            break;

        buf[j++] = iCode[*ioI];
        (*ioI)++;
        if (j == curBufLen)
            buf = realloc(buf, curBufLen *= 2);
    }

    buf[j++] = 0;
    if (j != curBufLen)
        buf = realloc(buf, j);
    return buf;
}

void FindLabels(char* code, size_t codeLen)
{
    size_t i = 0;
    uint16_t instrNum = 0;

    while (i < codeLen)
    {
        SkipWhitespace(code, codeLen, &i);
        if (ParseOPCode(code, &i) == OPCode_Invalid)
        {
            if (code[i] != '\n')
            {
                SkipIndent(code, codeLen, &i);
                if (code[i] == 0)
                    return;

                size_t oldI = i;

                char* id = ReadNextIdentifier(code, &i, codeLen);
                if (code[i] == ':')
                {
                    GenericList_Append(&labels, &(Label){id, instrNum});

                    // When a label has been found, it is simply replaced with whitespace.
                    // This might not be the most elegant solution, but it saves us from having
                    // to parse the label again later or having to copy strings.
                    oldI = i;
                    while (code[i] != '\n' && i != SIZE_MAX)
                        code[i--] = ' ';
                    i = oldI;
                }
                else
                {
                    i = oldI;
                    free(id);
                }
            }
        }
        else
            instrNum++;

        while (i < codeLen && code[i] != '\n')
            i++;
        i++;
    }
}

bool TryParseNumber(char* iCode, size_t* ioI, uint16_t* oLiteral)
{
    long n = 0;
    char* end = NULL;
    if (strncmp(&iCode[*ioI], "0b", 2) == 0)
        n = strtol(&iCode[*ioI], &end, 2);
    else if (strncmp(&iCode[*ioI], "0x", 2) == 0)
        n = strtol(&iCode[*ioI], &end, 16);
    else
        n = strtol(&iCode[*ioI], &end, 10);

    if (errno == EINVAL)
        ExitWithError("Literal too large!");

    if (end == &iCode[*ioI])
        return false;

    *oLiteral = (uint16_t)n;
    (*ioI) += (end - &iCode[*ioI]);
    return true;
}

bool CompareLabelToString(void* label, void* string)
{
    return strcmp(((Label*)label)->id, string) == 0;
}

bool TryParseLabel(char* iCode, size_t* ioI, size_t codeLen, uint16_t* oLiteral)
{
    // Copy is not really required here...
    size_t oldI = *ioI;
    char* id = ReadNextIdentifier(iCode, ioI, codeLen);
    Label* l = GenericList_Find(&labels, CompareLabelToString, id);
    free(id);

    if (l == NULL)
    {
        *ioI = oldI;
        return false;
    }

    *oLiteral = l->addr;
    return true;
}

bool TryParseLiteral(char* iCode, size_t* ioI, size_t codeLen, uint16_t* ioLiteral, bool* ioLiteralSet)
{
    uint16_t literal;
    bool found = TryParseNumber(iCode, ioI, &literal) || TryParseLabel(iCode, ioI, codeLen, &literal);
    if (!found)
        return false;

    if (*ioLiteralSet && *ioLiteral != literal)
        ExitWithError("Instruction uses too many literals!");

    *ioLiteralSet = true;
    *ioLiteral = literal;
    return true;
}

void IncrementAllLabelsLargerThan(void* elem, void* param)
{
    Label* label = elem;
    uint16_t* largerThan = param;

    if (label->addr > *largerThan)
        label->addr++;
}

// This is the second pass. After getting the address of all labels
// in the first pass, here we check if a conditional jump is larger than 255.
// If so, it can't be represented in one instruction, two are required.
// Therefore all labels after such a jump need to be incremented,
// as the jmp turns out to be two instructions, rather than one.
void CheckCondJumpLengths(char* code, size_t codeLen)
{
    size_t i = 0;
    uint16_t instrNum = 0;

    OPCode opcode;
    while (i < codeLen)
    {
        if ((opcode = ParseOPCode(code, &i)) != OPCode_Invalid)
        {
            if (opcode == OPCode_JMP)
            {
                if (code[i] == '_')
                {
                    (i)++;
                    Cond cond = ParseCond(code, &i);
                    if (cond == Cond_Invalid)
                        ExitWithError("Invalid condition");

                    SkipWhitespace(code, codeLen, &i);
                    uint16_t literal;
                    bool literalSet = false;

                    if (!TryParseLiteral(code, &i, codeLen, &literal, &literalSet))
                        ExitWithError("Invalid jump");

                    int16_t delta = literal - (instrNum + 1);
                    if (delta > 0xFF || delta < -0xFF)
                        GenericList_ForAll(&labels, IncrementAllLabelsLargerThan, &instrNum);
                }
            }
            instrNum++;
        }

        while (i < codeLen && code[i] != '\n')
            i++;
        i++;
    }
}

bool TryParseMemoryAccess(char* iCode, size_t* ioI, size_t codeLen, uint16_t* ioLiteral, bool* ioLiteralSet,
                          Addr* ioAddr, bool* ioAddrSet)
{
    if (iCode[*ioI] != '[')
        return false;
    (*ioI)++;
    SkipIndent(iCode, codeLen, ioI);
    Addr addr;

    if (!TryParseLiteral(iCode, ioI, codeLen, ioLiteral, ioLiteralSet))
    {
        addr = ParseAddr(iCode, ioI);
        if (addr == Addr_Invalid)
            ExitWithError("Invalid memory address");

        if (addr == Addr_SP_MINUS || addr == Addr_SP_PLUS)
        {
            if (!TryParseLiteral(iCode, ioI, codeLen, ioLiteral, ioLiteralSet) || *ioLiteral > 0xFF)
                ExitWithError("Invalid memory address");
        }
    }
    else
        addr = Addr_LIT16;

    if (*ioAddrSet && *ioAddr != addr)
        ExitWithError("Cannot access memory at different addresses in one instruction");
    *ioAddrSet = true;
    *ioAddr = addr;

    if (iCode[(*ioI)++] != ']')
        ExitWithError("Syntax error");

    return true;
}

Source ParseSourceToken(char* iCode, size_t* ioI, size_t codeLen, uint16_t* ioLiteral, bool* ioLiteralSet, Addr* ioAddr,
                        bool* ioAddrSet)
{
    Source s = -1;
    if (TryParseLiteral(iCode, ioI, codeLen, ioLiteral, ioLiteralSet))
    {
        if (*ioLiteral <= 0xFF)
            return Source_LIT8;
        return Source_LIT16;
    }
    else if ((s = ParseSource(iCode, ioI)) != Source_Invalid)
    {
        return s;
    }
    else if (TryParseMemoryAccess(iCode, ioI, codeLen, ioLiteral, ioLiteralSet, ioAddr, ioAddrSet))
    {
        return Source_RAM;
    }

    return -1;
}

Dst ParseDestinationToken(char* iCode, size_t* ioI, size_t codeLen, uint16_t* ioLiteral, bool* ioLiteralSet,
                          Addr* ioAddr, bool* ioAddrSet)
{
    Dst d = -1;

    if ((d = ParseDst(iCode, ioI)) != Dst_Invalid)
    {
        if (d == Dst_PER)
            // For accessing a peripheral at an address the notation per[123] is used.
            TryParseMemoryAccess(iCode, ioI, codeLen, ioLiteral, ioLiteralSet, ioAddr, ioAddrSet);

        return d;
    }
    else if (TryParseMemoryAccess(iCode, ioI, codeLen, ioLiteral, ioLiteralSet, ioAddr, ioAddrSet))
    {
        return Dst_RAM;
    }

    return -1;
}

uint32_t AssembleInstruction(uint16_t literal, Dst dst, Cond cond, OPCode opcode, Addr addr, Source srcB, Source srcA)
{
    uint32_t instr = 0;
    instr |= srcA & 0b1111;
    instr |= srcB << 4;
    instr |= addr << 8;
    instr |= opcode << 12;

    if (literal > 0xFF || addr == Addr_LIT16 || srcA == Source_LIT16 || srcB == Source_LIT16)
    {
        if (cond != 0)
            ExitWithError("Cannot use 16-bit literal and condition");
        // Once again, casting here causes issues with out
        if (dst != (Dst)srcA)
            ExitWithError("Cannot use 16-bit literal and three operands");
        instr |= (uint32_t)(literal >> 8) << (uint32_t)16;
    }
    else
    {
        instr |= dst << 16;
        instr |= cond << 20;
    }
    instr |= (uint32_t)(literal & 0xFF) << (uint32_t)24;

    return instr;
}

static Cond InvertCond(Cond c)
{
    if (((c - 1) % 4) == 0)
        return c + 2;
    return c - 2;
}

uint32_t followingInstruction = 0;
bool followingInstructionSet = false;

uint32_t ParseInstruction(char* code, size_t* i, size_t codeLen, uint16_t* instrAddr)
{
    // Tiny hack to allow pseudo-instructions that consist of two real instructions
    if (followingInstructionSet)
    {
        followingInstructionSet = false;
        return followingInstruction;
    }

    Source srcA = 0, srcB = 0;
    Addr addr = Addr_SP;
    OPCode opcode = 0;
    Dst dst = 0;
    Cond cond = 0;
    uint16_t literal = 0;

    bool literalSet = false;
    bool addrSet = false;

    opcode = ParseOPCode(code, i);
    if (opcode == OPCode_Invalid)
        ExitWithError("Invalid opcode");

    // Condition
    if (code[*i] == '_')
    {
        (*i)++;
        cond = ParseCond(code, i);
        if (cond == Cond_Invalid)
            ExitWithError("Invalid condition");
    }

    if (opcode == OPCode_JMP)
    {
        SkipIndent(code, codeLen, i);

        if (!TryParseLiteral(code, i, codeLen, &literal, &literalSet))
            ExitWithError("Invalid jmp destination");
        int16_t delta = literal - (*instrAddr + 1);

        dst = Dst_IP;
        srcA = Source_IP;
        if (cond == 0)
            srcB = Source_LIT16;
        else
            srcB = Source_LIT8;

        literal = (delta < 0) ? -delta : delta;

        if (cond != 0 && literal > 0xFF)
        {
            followingInstructionSet = true;
            followingInstruction = AssembleInstruction(literal, Dst_IP, 0, delta < 0 ? OPCode_SUB : OPCode_ADD, 0,
                                                       Source_LIT16, Source_IP);

            cond = InvertCond(cond);
            literal = 1;
        }

        if (delta > 0)
            opcode = OPCode_ADD;
        else
            opcode = OPCode_SUB;

        goto ParseInstruction_return;
    }
    else if (opcode == OPCode_NOP)
    {
        SkipIndent(code, codeLen, i);
        opcode = OPCode_MOV;
        goto ParseInstruction_return;
    }
    else if (opcode == OPCode_PER)
    {
        TryParseMemoryAccess(code, i, codeLen, &literal, &literalSet, &addr, &addrSet);
        SkipIndent(code, codeLen, i);
    }

    SkipIndent(code, codeLen, i);

    // Destination
    dst = ParseDestinationToken(code, i, codeLen, &literal, &literalSet, &addr, &addrSet);
    if (dst == Dst_Invalid)
    {
        printf("%s\n", &code[*i]);
        ExitWithError("Invalid destination!");
    }
    SkipIndent(code, codeLen, i);
    if (code[(*i)] != ',')
    {
        // Might be better to re-parse as dst instead of casting, this breaks eg "not out"
        srcA = (Source)dst;
        goto ParseInstruction_return;
    }
    (*i)++;
    SkipIndent(code, codeLen, i);

    // Source B (if given)
    srcB = ParseSourceToken(code, i, codeLen, &literal, &literalSet, &addr, &addrSet);
    if (srcB == Source_Invalid)
        ExitWithError("Invalid source!");
    SkipIndent(code, codeLen, i);
    if (code[(*i)] != ',')
    {
        srcA = (Source)dst;
        goto ParseInstruction_return;
    }
    (*i)++;
    SkipIndent(code, codeLen, i);

    // Source A (if given)
    srcA = ParseSourceToken(code, i, codeLen, &literal, &literalSet, &addr, &addrSet);
    if (srcA == Source_Invalid)
        ExitWithError("Invalid source!");
    SkipIndent(code, codeLen, i);

    Source t = srcA;
    srcA = srcB;
    srcB = t;

ParseInstruction_return:
    if (code[*i] != '\n' && code[*i] != 0)
        ExitWithError("Syntax error!");

    // MOV doesn't need srcA (only srcB is moved into dst), so disable it if possible
    if (opcode == OPCode_MOV && literal <= 0xFF && addr != Addr_LIT16 && srcA != Source_LIT16 && srcB != Source_LIT16 && dst != Dst_PER)
        srcA = 0;

    (*instrAddr)++;
    return AssembleInstruction(literal, dst, cond, opcode, addr, srcB, srcA);
}



int main(int argc, char** args)
{
    if (argc <= 1)
    {
        printf("Usage %s [ARGUMENTS]... [INPUT-FILE]\n", args[0]);
        return 0;
    }

    // Disassemble
    if (strncmp(args[1], "-d", 2) == 0) 
    {
        if (argc == 2)
            ExitWithError("Invalid input file!");
        uint32_t buf[256];
        FILE* f = fopen(args[2], "r");
        
        if(!f)
            ExitWithError("Invalid input file!");

        size_t addr = 0;
        size_t count = 0;
        while ((count = fread(buf, sizeof(uint32_t), sizeof(buf) / sizeof(uint32_t), f)))
        {
            for (size_t i = 0; i < count; i++)
            {
                
                printf("%.6zu: %.8x: ", addr, buf[i]);
                PrintDisassembled(buf[i]);
                putc('\n', stdout);
                addr += 1;
            }
        }
    }
    else
    {
        size_t size = 0;
        char* text = ReadFileAsString(args[argc - 1], &size);

        if (size == 0)
            ExitWithError("Invalid input file!");

        labels = GenericList_Create(sizeof(Label));
        FindLabels(text, size);
        CheckCondJumpLengths(text, size);

        size_t i = 0;
        uint16_t addr = 0;

        const size_t bufSize = 16;
        uint32_t buf[bufSize];
        size_t curBufLen = 0;

        FILE* outFile = fopen("program.bin", "w");
        if (!outFile)
            ExitWithError("Could not open output file!");

        while (i < size || followingInstructionSet)
        {
            while (isspace(text[i]) || text[i] == ';')
            {
                SkipWhitespace(text, size, &i);
                SkipComment(text, size, &i);
            }
            if (i < size || followingInstructionSet)
            {
                uint32_t instr = ParseInstruction(text, &i, size, &addr);
                //printf("%.6u: %.8x: ", addr - 1, instr);
                //PrintDisassembled(instr);
                //putc('\n', stdout);
                buf[curBufLen++] = instr;

                if (curBufLen == bufSize)
                {
                    fwrite(buf, sizeof(uint32_t), bufSize, outFile);
                    curBufLen = 0;
                }
            }
        }

        if (curBufLen)
        {
            fwrite(buf, sizeof(uint32_t), curBufLen, outFile);
            curBufLen = 0;
        }

        fclose(outFile);
    }
}
