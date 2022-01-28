#include "Lexer.h"
#include <stddef.h>
#include <stdint.h>

Addr ParseAddr(char* code, size_t* i)
{

    switch (code[*i + 0])
    {
        case '-':
            if (code[*i + 1] == '-')
            {
                if (code[*i + 2] == 's')
                {
                    if (code[*i + 3] == 'p')
                    {
                        *i += 4;
                        return Addr_SP_DEC;
                    }
                }
            }
            return -1;
        case 'r':
            switch (code[*i + 1])
            {
                case '0':
                    *i += 2;
                    return Addr_R0;
                case '1':
                    *i += 2;
                    return Addr_R1;
                case '2':
                    *i += 2;
                    return Addr_R2;
                case '3':
                    *i += 2;
                    return Addr_R3;
                case '4':
                    *i += 2;
                    return Addr_R4;
                case '5':
                    *i += 2;
                    return Addr_R5;
                case '6':
                    *i += 2;
                    return Addr_R6;
                case '7':
                    *i += 2;
                    return Addr_R7;
                case 'z':
                    *i += 2;
                    return Addr_RZ;
            }
            return -1;
        case 's':
            if (code[*i + 1] == 'p')
            {
                switch (code[*i + 2])
                {
                    case '+':
                        if (code[*i + 3] == '+')
                        {
                            *i += 4;
                            return Addr_SP_INC;
                        }
                        *i += 3;
                        return Addr_SP_PLUS;
                    case '-':
                        *i += 3;
                        return Addr_SP_MINUS;
                }
                *i += 2;
                return Addr_SP;
            }
            return -1;
    }
    return -1;
}

Cond ParseCond(char* code, size_t* i)
{
    switch (code[*i + 0])
    {
        case 'c':
            *i += 1;
            return Cond_C;
        case 'h':
            if (code[*i + 1] == 'l')
            {
                if (code[*i + 2] == 't')
                {
                    *i += 3;
                    return Cond_HLT;
                }
            }
            return -1;
        case 'n':
            switch (code[*i + 1])
            {
                case 'c':
                    *i += 2;
                    return Cond_NC;
                case 'p':
                    *i += 2;
                    return Cond_NP;
                case 's':
                    *i += 2;
                    return Cond_NS;
                case 'z':
                    *i += 2;
                    return Cond_NZ;
            }
            return -1;
        case 'p':
            *i += 1;
            return Cond_P;
        case 's':
            *i += 1;
            return Cond_S;
        case 'z':
            *i += 1;
            return Cond_Z;
    }
    return -1;
}

Dst ParseDst(char* code, size_t* i)
{
    switch (code[*i + 0])
    {
        case 'i':
            if (code[*i + 1] == 'p')
            {
                *i += 2;
                return Dst_IP;
            }
            return -1;
        case 'o':
            if (code[*i + 1] == 'u')
            {
                if (code[*i + 2] == 't')
                {
                    *i += 3;
                    return Dst_OUT;
                }
            }
            return -1;
        case 'p':
            switch (code[*i + 1])
            {
                case 'e':
                    if (code[*i + 2] == 'r')
                    {
                        *i += 3;
                        return Dst_PER;
                    }
                    return -1;
                case 's':
                    if (code[*i + 2] == 'e')
                    {
                        if (code[*i + 3] == 'l')
                        {
                            *i += 4;
                            return Dst_PSEL;
                        }
                    }
                    return -1;
            }
            return -1;
        case 'r':
            switch (code[*i + 1])
            {
                case '0':
                    *i += 2;
                    return Dst_R0;
                case '1':
                    *i += 2;
                    return Dst_R1;
                case '2':
                    *i += 2;
                    return Dst_R2;
                case '3':
                    *i += 2;
                    return Dst_R3;
                case '4':
                    *i += 2;
                    return Dst_R4;
                case '5':
                    *i += 2;
                    return Dst_R5;
                case '6':
                    *i += 2;
                    return Dst_R6;
                case '7':
                    *i += 2;
                    return Dst_R7;
                case 'z':
                    *i += 2;
                    return Dst_RZ;
            }
            return -1;
        case 's':
            if (code[*i + 1] == 'p')
            {
                *i += 2;
                return Dst_SP;
            }
            return -1;
    }
    return -1;
}

Source ParseSource(char* code, size_t* i)
{

    switch (code[*i + 0])
    {
        case 'i':
            switch (code[*i + 1])
            {
                case 'n':
                    *i += 2;
                    return Source_IN;
                case 'p':
                    *i += 2;
                    return Source_IP;
            }
            return -1;
        case 'o':
            if (code[*i + 1] == 'u')
            {
                if (code[*i + 2] == 't')
                {
                    *i += 3;
                    return Source_OUT;
                }
            }
            return -1;
        case 'r':
            switch (code[*i + 1])
            {
                case '0':
                    *i += 2;
                    return Source_R0;
                case '1':
                    *i += 2;
                    return Source_R1;
                case '2':
                    *i += 2;
                    return Source_R2;
                case '3':
                    *i += 2;
                    return Source_R3;
                case '4':
                    *i += 2;
                    return Source_R4;
                case '5':
                    *i += 2;
                    return Source_R5;
                case '6':
                    *i += 2;
                    return Source_R6;
                case '7':
                    *i += 2;
                    return Source_R7;
                case 'z':
                    *i += 2;
                    return Source_RZ;
            }
            return -1;
        case 's':
            if (code[*i + 1] == 'p')
            {
                *i += 2;
                return Source_SP;
            }
            return -1;
    }
    return -1;
}

OPCode ParseOPCode(char* code, size_t* i)
{

    switch (code[*i + 0])
    {
        case 'a':
            switch (code[*i + 1])
            {
                case 'd':
                    if (code[*i + 2] == 'd')
                    {
                        *i += 3;
                        return OPCode_ADD;
                    }
                    return -1;
                case 'n':
                    if (code[*i + 2] == 'd')
                    {
                        *i += 3;
                        return OPCode_AND;
                    }
                    return -1;
            }
            return -1;
        case 'd':
            if (code[*i + 1] == 'i')
            {
                if (code[*i + 2] == 'v')
                {
                    if (code[*i + 3] == 's')
                    {
                        *i += 4;
                        return OPCode_DIVS;
                    }
                    *i += 3;
                    return OPCode_DIV;
                }
            }
            return -1;
        case 'i':
            if (code[*i + 1] == 'n')
            {
                if (code[*i + 2] == 'v')
                {
                    if (code[*i + 3] == 'q')
                    {
                        *i += 4;
                        return OPCode_INVQ;
                    }
                }
            }
            return -1;
        case 'j':
            if (code[*i + 1] == 'm')
            {
                if (code[*i + 2] == 'p')
                {
                    *i += 3;
                    return OPCode_JMP;
                }
            }
            return -1;
        case 'm':
            switch (code[*i + 1])
            {
                case 'o':
                    if (code[*i + 2] == 'v')
                    {
                        *i += 3;
                        return OPCode_MOV;
                    }
                    return -1;
                case 'u':
                    if (code[*i + 2] == 'l')
                    {
                        switch (code[*i + 3])
                        {
                            case 'h':
                                *i += 4;
                                return OPCode_MULH;
                            case 'q':
                                *i += 4;
                                return OPCode_MULQ;
                        }
                        *i += 3;
                        return OPCode_MUL;
                    }
                    return -1;
            }
            return -1;
        case 'n':
            if (code[*i + 1] == 'o')
            {
                switch (code[*i + 2])
                {
                    case 'p':
                        *i += 3;
                        return OPCode_NOP;
                    case 't':
                        *i += 3;
                        return OPCode_NOT;
                }
            }
            return -1;
        case 'o':
            if (code[*i + 1] == 'r')
            {
                *i += 2;
                return OPCode_OR;
            }
            return -1;
        case 'p':
            if (code[*i + 1] == 'e')
            {
                if (code[*i + 2] == 'r')
                {
                    *i += 3;
                    return OPCode_PER;
                }
            }
            return -1;
        case 's':
            switch (code[*i + 1])
            {
                case 'h':
                    switch (code[*i + 2])
                    {
                        case 'l':
                            *i += 3;
                            return OPCode_SHL;
                        case 'r':
                            *i += 3;
                            return OPCode_SHR;
                    }
                    return -1;
                case 'u':
                    if (code[*i + 2] == 'b')
                    {
                        *i += 3;
                        return OPCode_SUB;
                    }
                    return -1;
            }
            return -1;
        case 'x':
            if (code[*i + 1] == 'o')
            {
                if (code[*i + 2] == 'r')
                {
                    *i += 3;
                    return OPCode_XOR;
                }
            }
            return -1;
    }
    return -1;
}