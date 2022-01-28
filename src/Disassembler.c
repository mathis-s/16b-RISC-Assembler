#include "Disassembler.h" 

const char* opcodes[] = {
    "mov", "add", "sub", "and", "or", "xor", "shl", "shr", "not", "mul", "div", "mulh", "mulq", "invq", "divs", "per",
};

const char* sources[] = {
    "rz", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "ram", "ip", "sp", "in", "out", "lit16", "lit8",
};

const char* destinations[] = {"rz", "r0",  "r1", "r2", "r3",   "r4",  "r5", "r6",
                              "r7", "ram", "ip", "sp", "psel", "out", "per", "none"};

const char* addresses[] = {"none", "r0", "r1",   "r2",   "r3",   "r4",   "r5",   "r6",
                           "r7", "sp", "sp-#", "sp++", "sp+#", "--sp", "none", "lit16"};

const char* conditions[] = {
    "", "z", "", "nz", "", "p", "", "np", "", "s", "", "ns", "", "c", "", "nc", "", "hlt",
};

void PrintRAMAccessDisassembled(Addr addr, uint16_t literal)
{
    switch (addr)
    {
        case Addr_LIT16:
            printf("[%u]", literal);
            break;
        case Addr_SP_MINUS:
            printf("[sp-%u]", literal & 0xFF);
            break;
        case Addr_SP_PLUS:
            printf("[sp+%u]", literal & 0xFF);
            break;
        default:
            printf("[%s]", addresses[addr]);
            break;
    }
}

void PrintSourceDisassembled(Source src, Addr addr, uint16_t literal)
{
    switch (src)
    {
        case Source_LIT8:
        case Source_LIT16:
            printf("%u ", literal);
            break;
        case Source_RAM:
            PrintRAMAccessDisassembled(addr, literal);
            break;
        default:
            printf("%s", sources[src]);
            break;
    }
}

void PrintDstDisassembled(Dst dst, Addr addr, uint16_t literal)
{
    switch (dst)
    {
        case Dst_RAM:
            PrintRAMAccessDisassembled(addr, literal);
            break;
        default:
            printf("%s", destinations[dst]);
            break;
    }
}

void PrintDisassembled(uint32_t instr)
{

    Source srcA = instr & 0xF;
    Source srcB = (instr & 0xF0) >> 4;
    Addr addr = (instr & 0xF00) >> 8;
    OPCode opcode = (instr & 0xF000) >> 12;
    Dst dst;
    Cond cond;

    if (srcA == Source_LIT16 || srcB == Source_LIT16 || addr == Addr_LIT16)
    {
        uint literal = instr >> 24;
        literal |= (instr & 0xFF0000) >> 8;
        // printf("%s %s, %s", opcodes[opcode], sources[srcA], sources[srcB]);
        printf("%s ", opcodes[opcode]);
        // This cast is kind of questionable. But the exact same thing is done in hardware,
        // so this shows the actual destination.
        PrintDstDisassembled((Dst)srcA, addr, literal);
        printf(", ");
        PrintSourceDisassembled(srcB, addr, literal);
    }
    else
    {
        dst = (instr & 0xF0000) >> 16;
        cond = (instr & 0xF00000) >> 20;
        uint literal = instr >> 24;

        if ((cond & 1) == 1)
            printf("%s_%s ", opcodes[opcode], conditions[cond]);
        else
            printf("%s ", opcodes[opcode]);

        PrintDstDisassembled(dst, addr, literal);
        printf(", ");
        PrintSourceDisassembled(srcA, addr, literal);
        printf(", ");
        PrintSourceDisassembled(srcB, addr, literal);
    }
}