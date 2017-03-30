#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"

InterCode ICL[10000];//四元式InterCodeList
int ICL_index = -1;//四元式列表指针
extern Reg RegisterList[10000];//记录识别出来的单词
extern SymTable Table[10000];//符号表
extern int line;

char Temp_Var = 'a';//记录临时变量


void Init()
{
    ICL_index++;
    ICL[ICL_index].OP = UNKNOWN;
    ICL[ICL_index].Arg1.arg_type = 0;
    ICL[ICL_index].Arg1.char_type_value = '\0';
    strcpy(ICL[ICL_index].Arg1.ident_type_value,"");
    ICL[ICL_index].Arg2.arg_type = 0;
    ICL[ICL_index].Arg2.char_type_value = '\0';
    strcpy(ICL[ICL_index].Arg2.ident_type_value,"");
    ICL[ICL_index].Result.arg_type = 0;
    ICL[ICL_index].Result.char_type_value = '\0';
    strcpy(ICL[ICL_index].Result.ident_type_value,"");
    ICL[ICL_index].JumpTo = -1;
}

ArgNode InitArgNode()
{
    ArgNode OP;
    OP.arg_type = 0;
    OP.int_type_value = 0;
    OP.char_type_value = '\0';
    strcpy(OP.string_type_value,"");
    strcpy(OP.ident_type_value,"");
    return OP;
}


void Fill(int instruct,int address)
{
    ICL[instruct].Result.int_type_value = address;
    return;
}

ArgNode Emit(OpType OP,ArgNode A1,ArgNode A2,ArgNode R)//R只在数组赋值时用到
{
    Init();

    if(OP==UNKNOWN)
    {
        ICL_index--;
        return A1;
    }
    else if(OP==ADD)
    {
        ICL[ICL_index].OP = ADD;
        ICL[ICL_index].Arg1 = A1;
        ICL[ICL_index].Arg2 = A2;
        ICL[ICL_index].Result.arg_type = 7;//7是临时变量
        ICL[ICL_index].Result.ident_type_value[0] = '%';
        ICL[ICL_index].Result.ident_type_value[1] = Temp_Var;
        Temp_Var = Temp_Var+1;
        if(Temp_Var > 122)  Temp_Var = Temp_Var - 26;
        ICL[ICL_index].Result.ident_type_value[2] = '\0';
        return ICL[ICL_index].Result;
    }
    else if(OP==SUB)
    {
        ICL[ICL_index].OP = SUB;
        ICL[ICL_index].Arg1 = A1;
        ICL[ICL_index].Arg2 = A2;
        ICL[ICL_index].Result.arg_type = 7;//7是临时变量
        ICL[ICL_index].Result.ident_type_value[0] = '%';
        ICL[ICL_index].Result.ident_type_value[1] = Temp_Var;
        Temp_Var = Temp_Var+1;
        if(Temp_Var > 122)  Temp_Var = Temp_Var - 26;
        ICL[ICL_index].Result.ident_type_value[2] = '\0';
        return ICL[ICL_index].Result;
    }
    else if(OP==ASSI)
    {

        ICL[ICL_index].OP = ASSI;
        ICL[ICL_index].Arg1 = A2;
        ICL[ICL_index].Result = A1;
        return ICL[ICL_index].Result;
    }
    else if(OP==CALL)
    {

        ICL[ICL_index].OP = CALL;
        ICL[ICL_index].Result = A1;
        return ICL[ICL_index].Result;
    }

    else if(OP==PRINTSTR)
    {
        ICL[ICL_index].OP = PRINTSTR;
        ICL[ICL_index].Result = A1;
        return ICL[ICL_index].Result;
    }
    else if(OP==PRINTINTIDENT)
    {
        ICL[ICL_index].OP = PRINTINTIDENT;
        ICL[ICL_index].Result = A1;
        return ICL[ICL_index].Result;
    }
    else if(OP==PRINTCHARIDENT)
    {
        ICL[ICL_index].OP = PRINTCHARIDENT;
        ICL[ICL_index].Result = A1;
        return ICL[ICL_index].Result;
    }
    else if(OP==PRINTINT)
    {
        ICL[ICL_index].OP = PRINTINT;
        ICL[ICL_index].Result = A1;
        return ICL[ICL_index].Result;
    }
    else if(OP==PRINTCHAR)
    {
        ICL[ICL_index].OP = PRINTCHAR;
        ICL[ICL_index].Result = A1;
        return ICL[ICL_index].Result;
    }
    else if(OP==PARAM)
    {
        ICL[ICL_index].OP = PARAM;
        ICL[ICL_index].Arg1 = A2;//记录是第几个参数
        ICL[ICL_index].Result = A1;
        return ICL[ICL_index].Result;
    }
    else if(OP==SCANINT)
    {
        ICL[ICL_index].OP = SCANINT;
        ICL[ICL_index].Result = A1;
        return ICL[ICL_index].Result;
    }
    else if(OP==SCANCHAR)
    {
        ICL[ICL_index].OP = SCANCHAR;
        ICL[ICL_index].Result = A1;
        return ICL[ICL_index].Result;
    }
    else if(OP==CMP)
    {
        ICL[ICL_index].OP = CMP;
        ICL[ICL_index].Arg1 = A1;
        ICL[ICL_index].Arg2 = A2;
        return ICL[ICL_index].Result;
    }
    else if(OP==JE || OP==JNE || OP==JL || OP==JLE || OP==JG || OP==JGE || OP==JMP)
    {
        ICL[ICL_index].OP = OP;
        ICL[ICL_index].Result.arg_type = 1;
        ICL[ICL_index].Result.int_type_value = 0;//待回填
        return ICL[ICL_index].Result;
    }

    else if(OP==GETINTRVALUE)
    {
        ICL[ICL_index].OP = GETINTRVALUE;
        ICL[ICL_index].Result.arg_type = 5;
        ICL[ICL_index].Result.ident_type_value[0] = '%';
        ICL[ICL_index].Result.ident_type_value[1] = Temp_Var;
        Temp_Var = Temp_Var+1;
        if(Temp_Var > 122)  Temp_Var = Temp_Var - 26;
        ICL[ICL_index].Result.ident_type_value[2] = '\0';
        return ICL[ICL_index].Result;
    }
    else if(OP==GETCHARRVALUE)
    {
        ICL[ICL_index].OP = GETCHARRVALUE;
        ICL[ICL_index].Result.arg_type = 6;
        ICL[ICL_index].Result.ident_type_value[0] = '%';
        ICL[ICL_index].Result.ident_type_value[1] = Temp_Var;
        Temp_Var = Temp_Var+1;
        if(Temp_Var > 122)  Temp_Var = Temp_Var - 26;
        ICL[ICL_index].Result.ident_type_value[2] = '\0';
        return ICL[ICL_index].Result;
    }
    else if(OP==LABEL)
    {
        ICL[ICL_index].OP = LABEL;
        ICL[ICL_index].Result = A1;
        return ICL[ICL_index].Result;
    }
    else if(OP==GENCONST)
    {
        ICL[ICL_index].OP = GENCONST;

        ICL[ICL_index].Result = A1;
        return ICL[ICL_index].Result;
    }
    else if(OP==GENVAR)
    {
        ICL[ICL_index].OP = GENVAR;
        ICL[ICL_index].Result = A1;
    }
    else if(OP==PARAMX)
    {
        ICL[ICL_index].OP = PARAMX;
        ICL[ICL_index].Result = A1;
    }
    else if(OP==RETURNVOID)
    {
        ICL[ICL_index].OP = RETURNVOID;
        return ICL[ICL_index].Result;
    }
    else if(OP==RETURNNVOID)
    {
        ICL[ICL_index].OP = RETURNNVOID;
        ICL[ICL_index].Result = A1;
        return ICL[ICL_index].Result;
    }
    else if(OP==RETURNMAIN)
    {
        ICL[ICL_index].OP = RETURNMAIN;
        return ICL[ICL_index].Result;
    }
    else if(OP==GENARRAY)
    {
        ICL[ICL_index].OP = GENARRAY;
        ICL[ICL_index].Arg2 = A2;
        ICL[ICL_index].Result = A1;
        return ICL[ICL_index].Result;
    }
    else if(OP==ARRAY)
    {
        ICL[ICL_index].OP = ARRAY;
        ICL[ICL_index].Arg1 = A1;
        ICL[ICL_index].Arg2 = A2;
        ICL[ICL_index].Result.arg_type = 7;
        ICL[ICL_index].Result.ident_type_value[0] = '%';
        ICL[ICL_index].Result.ident_type_value[1] = Temp_Var;
        Temp_Var = Temp_Var+1;
        if(Temp_Var > 122)  Temp_Var = Temp_Var - 26;
        ICL[ICL_index].Result.ident_type_value[2] = '\0';
        return ICL[ICL_index].Result;
    }
    else if(OP==ASSIARRAY)
    {
        ICL[ICL_index].OP = ASSIARRAY;
        ICL[ICL_index].Arg1 = R;
        ICL[ICL_index].Arg2 = A2;
        ICL[ICL_index].Result = A1;
        return ICL[ICL_index].Result;
    }
    else if(OP==MUL)
    {
        ICL[ICL_index].OP = MUL;
        ICL[ICL_index].Arg1 = A1;
        ICL[ICL_index].Arg2 = A2;
        ICL[ICL_index].Result.arg_type = 7;
        ICL[ICL_index].Result.ident_type_value[0] = '%';
        ICL[ICL_index].Result.ident_type_value[1] = Temp_Var;
        Temp_Var = Temp_Var+1;
        if(Temp_Var > 122)  Temp_Var = Temp_Var - 26;
        ICL[ICL_index].Result.ident_type_value[2] = '\0';
        return ICL[ICL_index].Result;
    }
    else if(OP==DIV)
    {
        ICL[ICL_index].OP = DIV;
        ICL[ICL_index].Arg1 = A1;
        ICL[ICL_index].Arg2 = A2;
        ICL[ICL_index].Result.arg_type = 7;
        ICL[ICL_index].Result.ident_type_value[0] = '%';
        ICL[ICL_index].Result.ident_type_value[1] = Temp_Var;
        Temp_Var = Temp_Var+1;
        if(Temp_Var > 122)  Temp_Var = Temp_Var - 26;
        ICL[ICL_index].Result.ident_type_value[2] = '\0';
        return ICL[ICL_index].Result;
    }
    return ICL[ICL_index].Result;
}

void print_ICL()
{
    char Name[][30] = {"ADD","SUB","MUL","DIV","ARRAYVALUE","ARRAYADDR","OPPO","ASSI","ASSIGNRVALUE","ARRAYASSIGN",
                       "CMP","JE","JNE","JL","JLE","JG","JGE","JMP","PARAM","CALL","GETINTRVALUE","GETCHARRVALUE","RETURNNVOID","VRETURN",
                       "PRINTSTR","PRINTINTIDENT","PRINTCHARIDENT","PRINTINT","PRINTCHAR","SCANINT","SCANCHAR","LABEL","GENCONST","GENVAR","PARAMX","RETURNVOID","RETURNMAIN","GENARRAY","ARRAY",
                       "ASSIARRAY","ONEORZERO","UNKNOWN"};
    int i= 0;
    InterCode IC;
    for(i = 0;i<=ICL_index;i++)
    {
        IC = ICL[i];
        printf("%5d%12s",i,Name[IC.OP]);
        if(IC.Arg1.arg_type==1) printf("%15d",IC.Arg1.int_type_value);
        else if(IC.Arg1.arg_type==2) printf("%15c",IC.Arg1.char_type_value);
        else    printf("%15s",IC.Arg1.ident_type_value);
        if(IC.Arg2.arg_type==1) printf("%15d",IC.Arg2.int_type_value);
        else if(IC.Arg2.arg_type==2) printf("%15c",IC.Arg2.char_type_value);
        else    printf("%15s",IC.Arg2.ident_type_value);


        if(IC.Result.arg_type==1) printf("%8d",IC.Result.int_type_value);
        else if(IC.Result.arg_type==2) printf("%15c",IC.Result.char_type_value);
        else if(IC.Result.arg_type==8) printf("%20s",IC.Result.string_type_value);
        else if(IC.Result.arg_type==9) printf("%20s",IC.Result.ident_type_value);
        else    printf("%15s",IC.Result.ident_type_value);
        printf("\n");
    }
}
