#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"

extern Reg RegisterList[1000];//记录识别出来的单词
extern int ListIndex;//单词数量
extern void error(int errornum);
extern int line;
extern void print_ICL();
extern ArgNode InitArgNode();
extern int ICL_index;
extern void genmips();


extern InterCode ICL[1000];//四元式InterCodeList
extern int ICL_index;//四元式列表指针
extern ArgNode Emit(int OP,ArgNode Arg1,ArgNode Arg2,ArgNode R);//生成四元式

char currentsym[100];//currentsym是当前的单词，这里刚开始因为数组定义太小，没法存储较长的字符串导致错误
int currenttype;//当前单词的类型
char lastsym[100];//lastsym上一个单词
char lasttype;//上一个单词的类型
int Index = 0;//指针的位置指向下一个单词
int Level = 0;//记录当前程序段的层次
int bound = 0;//记录当前函数在符号表中的开始位置
int global_bound = 10000;//全局变量到这里

/*符号表中要用到的变量*/
SymTable Table[200];//符号表
char Name[30];
int Kind;
int DigitValue;
char CharValue;
char StringValue[1000];
int Size;
int Level;
/*分程序表*/
ProTable PTable[100];//分程序表
int P_parameter_num;//程序参数个数
int P_const_number;//程序中的常量个数
int P_var_number;//程序中的变量个数
int Pindex = 0;//分程序表指针

int Tindex = 0;
/**/
int Recall = 0;//用于记录程序判读失误，需要返回的位置

int const_num;//记录最后一个常量的位置
int var_num;//记录最后一个变量的位置
int function_num;//记录最后一个函数的位置
//////////////////////////////////////////////////////
int returnflag = 0;//0是void 1是int 2是char 3是main
/////////////////////////////////////////////////////
int Fillindex[100];//需要回填的指令位置。用于if和while。先进先出
int FI = -1;
extern void Fill(int instruct,int address);//用于回填跳转地址
int casenum = 0;//记录switch语句下的case数量
int Fillswitch[20];
int Fillswitchend = 0;
/////////////////////////////////////////////////////
extern void genmips();
/////////////////////////////////////////////////////////
void Parse();
void getnextsym();//读取下一个字符
void sendbacksym();//退回一个字符
void procedure();//核心程序
void const_declare_sentence();//常量说明语句
void const_define_sentence();//常量定义语句
void variable_declare_sentence();//变量说明语句
void variable_define_sentence();//变量定义语句
void parameter_table();//参数处理语句
void insert_paranumber_to_function(int i);//向函数中插入参数
void head_define();//头部定义
int StringToInt();//把字符串形式的数字表示为整型
void InsertTable();//插入符号表

void compound_sentence();
void sentence_set();
void sentence();
void if_sentence();
void condition_sentence();
void while_sentence();
void scanf_sentence();
void printf_sentence();
void return_sentence();
ArgNode expression();
ArgNode item();
ArgNode factor();
void return_function();//有返回值的函数调用语句处理
void para_table();//
void parameter_sentence();//记录函数调用过程中的参数个数
void switch_sentence();
void situation_table(ArgNode A1);
void situation_sentence(ArgNode A1);
void default_sentence();
void InsertTable();//插入符号表
int look_up_type();//返回当前单词的类型
int Find_const();//查看是否是常量
int Rep_int_const();//替换int常量
char  Rep_char_const();//替换char常量
int Check_Dulplicate();//查找是否有重复定义
char Name_[][30] = {"ADD","SUB","MUL","DIV","ARRAYVALUE","ARRAYADDR","OPPO","ASSI","ASSIGNRVALUE","ARRAYASSIGN",
                       "CMP","JE","JNE","JL","JLE","JG","JGE","JMP","PARAM","CALL","GETRVALUE","RRETURN","VRETURN",
                       "PRINT","SCAN","LABEL","GENCONST","PARAMX","RETURNVOID","RETURNMAIN","GENARRAY","UNKNOWN"};


int Find_const()//查找当前识别的标识符是不是常量
{
    int i = 0;
    for(i = Tindex;i>=0;i--)
    {
        if(strcmp(currentsym,Table[i].Name)==0)
        {
            if(Table[i].Kind==CONSTINTKIND) return 1;
            else if(Table[i].Kind==CONSTCHARKIND)    return 2;
            else    return 0;
        }
    }
    return 0;
}

int Rep_int_const()//将常量替换为值
{
    int i = 0;
    for(i = Tindex;i>=0;i--)
    {
        if(strcmp(currentsym,Table[i].Name)==0)
        {
            return Table[i].DigitValue;
        }
    }
    return 0;
}

char  Rep_char_const()//将常量替换为值
{
    int i = 0;
    for(i = Tindex;i>=0;i--)
    {
        if(strcmp(currentsym,Table[i].Name)==0)
        {
            return Table[i].CharValue;
        }
    }
    return '\0';
}

void InsertTable()//插入符号表
{
    if(Kind!=INTFUNCTIONKIND && Kind!=CHARFUNCTIONKIND && Kind!=VOIDFUNCTIONKIND)   Check_Dulplicate();
    Table[Tindex].Addr = Tindex;
    strcpy(Table[Tindex].Name,Name);//name 是这个符号看起来的样子
    Table[Tindex].Kind = Kind;
    Table[Tindex].line = line;
    if(Kind==INTKIND || Kind==CONSTINTKIND) Table[Tindex].DigitValue = DigitValue;
    else if(Kind==CHARKIND || Kind==CONSTCHARKIND)   Table[Tindex].CharValue = CharValue;
    else if(Kind==INTFUNCTIONKIND || Kind==CHARFUNCTIONKIND || Kind==VOIDFUNCTIONKIND)
    {
        Table[Tindex].ProIndex = Pindex;
        strcpy(PTable[Pindex].PName,Table[Tindex].Name);
        PTable[Pindex].PAddr = Pindex;
        PTable[Pindex].TAddr = Tindex;
    }
    else if(Kind==STRINGKIND)   strcpy(Table[Tindex].StringValue,StringValue);
    else if(Kind==INTARRAYKIND || Kind==CHARARRAYKIND)    Table[Tindex].Size = Size;
    else
    {
        printf("%s 插入符号表出错，没有当前类型\n",currentsym);
        Tindex--;
    }
    //清零
    int i = 0;
    for(i = 0;i<strlen(Name);i++)   Name[i] = '\0';
    DigitValue = 0;
    CharValue = '\0';
    for(i = 0;i<strlen(StringValue);i++)   StringValue[i] = '\0';
    Size = 0;
    //指针+1
    Tindex++;
}

int look_up_type()//查看当前单词的类型，在scanf_sentence中被调用
{
    int i = 0;
    for(i = Tindex;i>=0;i--)
    {
        if(strcmp(currentsym,Table[i].Name)==0)
        {
            return Table[i].Kind;
        }
    }
    printf("line:%d %s变量未定义\n",line,currentsym);
    return -1;
}

int Check_Dulplicate()
{
    int i = 0;
    if(global_bound==10000)//说明此时还是全局变量定义阶段
    {
        for(i = 0;i <= Tindex && i <= global_bound;i++)
        {
            if(strcmp(Table[i].Name,Name)==0)
            {
                printf("line:%d %s 重复定义！\n",line,currentsym);
                return false;
            }
        }
        for(i = 0;i < Pindex;i++)
        {
            if(strcmp(PTable[i].PName,Name)==0)
            {
                printf("line:%d %s 重复定义！\n",line,currentsym);
                return false;
            }
        }
    }
    else
    {
        for(i = bound;i <= Tindex;i++)
        {
            if(strcmp(Table[i].Name,Name)==0)
            {
                printf("line:%d %s 重复定义！\n",line,Name);
                return false;
            }
        }
    }
    return true;
}

void print_symbol_table()//打印符号表用于调试
{
    int i;
    printf("---------------符号表----------------\n");
    printf("                   变量名         种类      行数\n");
    for(i = 0;i<Tindex;i++)     printf("序号%3d    %15s  %8d  %8d\n",i,Table[i].Name,Table[i].Kind,Table[i].line);
    printf("-------------------------------------\n");
    return;
}

void print_procedure_table()//打印分程序表，好像没什么用
{
    int i;
    printf("---------------分程序表---------------\n");
    printf("            函数名            PADDR           TADDR        参数数量        常量数       变量数\n");
    for(i = 0;i<Pindex;i++)     printf("%15s  编号%3d %5d %5d %5d %5d\n",PTable[i].PName,PTable[i].PAddr,PTable[i].TAddr,PTable[i].parameter_num,PTable[i].const_num,PTable[i].var_num);
    printf("--------------------------------------\n");
    return;
}

void SkipTo(char a)
{
    int toline = 0;
    if(a=='\n')
    {
        toline = line + 1;
        while(line < toline)
        {
            getnextsym();
        }
        sendbacksym();
    }
    else if(a=='}' && Index < ListIndex)
    {
        while(currentsym[0]!='}')
        {
            getnextsym();
        }
    }
}

void getnextsym()//获取下一个单词
{
    strcpy(lastsym,currentsym);
    lasttype = currenttype;//把当前类型传递给上一个类型
    strcpy(currentsym,RegisterList[Index].Value);//读取下一个单词
    currenttype = RegisterList[Index].Type;
    line = RegisterList[Index].Line;//记录下这个单词位于第几行，便于报错
    Index++;
}

void sendbacksym()//退回单词
{
    Index--;//指向当前单词
    strcpy(currentsym,lastsym);//Index-1
    currenttype = lasttype;//上一个类型赋值给当前类型
    strcpy(lastsym,RegisterList[Index-2].Value);
    lasttype = RegisterList[Index-2].Type;
    line = RegisterList[Index-1].Line;
}
//＜程序＞::=［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
void  procedure()//核心程序
{
    int i = 0;
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    getnextsym();
    while(currenttype==CONST)//常量说明
    {
        sendbacksym();//退回const
        const_declare_sentence();//此时的currenttype是分号
        getnextsym();//继续读，如果是const则执行while
    }
    const_num = Tindex-1;//存下const常量的数量
    if(currenttype==INT || currenttype==CHAR)//上面常量说明部分读到了一个INT或CHAR，自动进入变量说明部分
    {
        sendbacksym();
        variable_declare_sentence();
    }
    global_bound = Tindex;//以上是全局变量

    //函数定义
    Level = 1;
    while(currenttype==INT || currenttype==CHAR || currenttype==VOID)
    {
        bound = Tindex;
        ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
        P_parameter_num = 0;//程序参数个数
        P_const_number = 0;//程序中的常量个数
        P_var_number = 0;//程序中的变量个数
        if(currenttype==INT || currenttype==CHAR)
        {
            if(currenttype==INT)    returnflag = 1;
            if(currenttype==CHAR)   returnflag = 2;
            Kind = (currenttype==INT) ? INTFUNCTIONKIND : CHARFUNCTIONKIND;
            getnextsym();//应该读到了函数的名字
            if(currenttype!=IDENT)
            {
                printf("line:%d 没有找到标识符\n",line);
            }
            else    strcpy(Name,currentsym);//int和char之后一定是标识符，至于标识符后面是不是还有[,则有待讨论
            //判断函数名字是不是重复定义
            for(i = 0;i <= Tindex && i <= global_bound;i++)
            {
                if(strcmp(Table[i].Name,currentsym)==0) printf("line:%d %s 重复定义！\n",line,currentsym);
            }
            for(i = 0;i <= Pindex;i++)
            {
                if(strcmp(PTable[i].PName,currentsym)==0)   printf("line:%d %s 重复定义！\n",line,currentsym);
            }
            strcpy(Arg1.ident_type_value,currentsym);
            Arg1.arg_type = 9;
            Emit(LABEL,Arg1,Arg2,Result);
            getnextsym();//应该读到一个左括号
            if(currenttype!=L_PARENTHESES)//不是左括号
            {
                printf("line:%d 函数定义后面缺少左括号\n",line);
                while(currenttype!=INT && currenttype!=CHAR && currenttype!=VOID)    getnextsym();
                continue;
            }
            InsertTable();//登录到符号表，是一个函数声明
            parameter_table();//进行参数分析
            //函数头部分析完了
            if(currenttype!=R_PARENTHESES)
            {
                printf("line:%d 缺少右括号\n",line);
                SkipTo('\n');
            }
            printf("---%d---这是一个函数定义语句\n",line);
            getnextsym();
            if(currenttype!=L_BRACE)
            {
                printf("line:%d 缺少左大括号\n",line);
                SkipTo('\n');
            }
            else    compound_sentence();//进行函数内的复合语句分析
            getnextsym();
            if(currenttype!=R_BRACE)
            {
                SkipTo('\n');
                printf("line:%d 函数结尾缺少右大括号\n",line);
            }
            PTable[Pindex].parameter_num = P_parameter_num;
            PTable[Pindex].const_num = P_const_number;
            PTable[Pindex].var_num = P_var_number;
        }
        else if(currenttype==VOID)//如果是void函数定义
        {
            returnflag = 0;
            Kind = VOIDFUNCTIONKIND;
            getnextsym();//读到了函数名字
            for(i = 0;i <= Tindex && i <= global_bound;i++)
            {
                if(strcmp(Table[i].Name,currentsym)==0) printf("line:%d %s 重复定义！\n",line,currentsym);
            }
            for(i = 0;i <= Pindex;i++)
            {
                if(strcmp(PTable[i].PName,currentsym)==0)   printf("line:%d %s 重复定义！\n",line,currentsym);
            }
            if(currenttype==MAIN)//读到了主函数
            {
                break;
            }
            strcpy(Arg1.ident_type_value,currentsym);
            Arg1.arg_type = 9;
            Emit(LABEL,Arg1,Arg2,Result);
            if(currenttype!=IDENT)//如果void后面不是标识符
            {
                SkipTo('\n');
                printf("line:%d void后面缺少标识符\n",line);
            }
            else
            {
                strcpy(Name,currentsym);//记录函数名字
                InsertTable();
                //print_symbol_table();
                getnextsym();
                if(currenttype==L_PARENTHESES)//是左括号，正确
                {
                    parameter_table();//进行参数分析
                    //函数头部分析完了
                    if(currenttype!=R_PARENTHESES)
                    {
                        SkipTo('\n');
                        printf("line:%d 函数头部缺少右括号\n",line);
                    }
                    printf("---%d---这是一个void函数定义语句\n",line);
                    getnextsym();
                    if(currenttype!=L_BRACE)
                    {
                        SkipTo('\n');
                        printf("line:%d 函数定义缺少左大括号\n",line);
                    }
                    compound_sentence();
                    getnextsym();
                    if(currenttype!=R_BRACE)
                    {
                        SkipTo('\n');
                        printf("line:%d 函数定义缺少右大括号\n",line);
                    }
                    Emit(RETURNVOID,Arg1,Arg2,Result);
                    PTable[Pindex].parameter_num = P_parameter_num;
                    PTable[Pindex].const_num = P_const_number;
                    PTable[Pindex].var_num = P_var_number;
                }
                else
                {
                    SkipTo('\n');
                    printf("line:%d 函数名字后面缺少括号\n",line);
                }
            }
        }
        getnextsym();
        Pindex++;
    }
    //开始主函数分析
    P_parameter_num = 0;//程序参数个数
    P_const_number = 0;//程序中的常量个数
    P_var_number = 0;//程序中的变量个数
    returnflag = 3;
    getnextsym();
    strcpy(Arg1.ident_type_value,"main");
    Arg1.arg_type = 9;
    Emit(LABEL,Arg1,Arg2,Result);
    if(currenttype==L_PARENTHESES)
    {
        getnextsym();
        if(currenttype==R_PARENTHESES)
        {
            printf("---%d---这是main函数定义\n",line);
            getnextsym();
            if(currenttype==L_BRACE)
            {
                compound_sentence();
                getnextsym();
                if(currenttype==R_BRACE)
                {
                    Emit(RETURNMAIN,Arg1,Arg2,Result);
                }
                else
                {
                    printf("line:%d 主函数缺少右括号匹配\n",line);
                }
            }
            else
            {
                SkipTo('\n');
                printf("line:%d 主函数缺少左大括号\n",line);
            }
        }
        else
        {
            SkipTo('\n');
            printf("line:%d main后小括号不匹配\n",line);
        }
    }
    else
    {
        SkipTo('\n');
        printf("line:%d main后没有左小括号\n",line);
    }
}


//int a = 10;
void const_declare_sentence()
{
    getnextsym();//读到const
    getnextsym();
    if(currenttype==INT || currenttype==CHAR)
    {
        sendbacksym();
        const_define_sentence();
    }
    else
    {
        printf("line:%d 非法字符\n",line);
    }
    getnextsym();
    if(currenttype==SEMICOLON)
    {
        printf("---%d---这是一个常量说明语句\n",line);
    }
    else
    {
        SkipTo('\n');
        printf("line:%d 缺少分号\n",line);
    }
    return;
}
//a = 10,b = 12 没有分号
void const_define_sentence()//被onst_declare_sentence调用了
{
    getnextsym();//读到int或char
    Kind = (currenttype==INT) ? CONSTINTKIND : CONSTCHARKIND;//存储当前类型
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    int temp;//用于存储数字转换的结果
    getnextsym();//读取下一个字符，应该是标识符
    while(currenttype==IDENT)
    {
        strcpy(Arg1.ident_type_value,currentsym);
        strcpy(Name,currentsym);
        getnextsym();
        if(currenttype==ASSIGN)
        {
            getnextsym();
            if(currenttype==PLUS || currenttype==MINUS)//等号后有+或-
            {
                getnextsym();
                if(currenttype==INTEGER && Kind==CONSTINTKIND)
                {
                    temp = StringToInt();//把当前字符串形式的数字转换为数字形式
                    DigitValue = (lasttype==PLUS) ? (temp) : (0-temp);
                    Arg1.arg_type = 1;
                    Arg1.int_type_value = DigitValue;
                    Emit(GENCONST,Arg1,Arg2,Result);
                    InsertTable();//向符号表中插入一个数字
                    //print_symbol_table();
                    printf("---%d---这是一个int赋值语句\n",line);
                }
                else
                {
                    SkipTo('\n');
                    printf("line:%d 等号后应该有int值\n",line);
                }
            }
            else if(currenttype==INTEGER)//等号后直接是无符号数字
            {
                if(Kind==CONSTINTKIND)
                {
                    temp = StringToInt();//string类型的数字转换为int类型
                    DigitValue = temp;
                    Arg1.arg_type = 1;
                    Arg1.int_type_value = DigitValue;
                    Emit(GENCONST,Arg1,Arg2,Result);
                    InsertTable();
                    //print_symbol_table();
                    printf("---%d---这是一个int赋值语句\n",line);
                }
                else
                {
                    SkipTo('\n');
                    printf("line:%d 类型不符\n",line);
                }
            }
            else if(currenttype==CHARACTER)//是字符类型
            {
                if(Kind==CONSTCHARKIND)
                {
                    CharValue = currentsym[0] - 0;
                    Arg1.arg_type = 2;
                    Arg1.char_type_value = CharValue;
                    Emit(GENCONST,Arg1,Arg2,Result);
                    InsertTable();//插入一个字符
                    //print_symbol_table();
                    printf("---%d---这是一个char赋值语句\n",line);
                }
                else
                {
                    SkipTo('\n');
                    printf("line:%d 类型不符\n",line);
                }
            }
            else
            {
                SkipTo('\n');
                printf("line:%d 常量定义语句错误\n",line);
            }
            getnextsym();//处理完一个a = 10语句，应该读到逗号或分号
            if(currenttype==COMMA)  getnextsym();
            else if(currenttype==SEMICOLON)
            {
                sendbacksym();//把读到的分号退回
                break;
            }
            else
            {
                SkipTo('\n');
                printf("line:%d 非法字符\n",line);
            }
        }
        else
        {
            SkipTo('\n');
            printf("line:%d 常量定义语句中，标识符后面没有等号\n",line);
        }
    }
    printf("---%d---这是一个常量定义语句\n",line);
    return;
}

//int a;
//int b,c,d;
//int sum(int a,int b){ }

void variable_declare_sentence()//int 还没读
{
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    getnextsym();//读到int或char
    while(currenttype==INT || currenttype==CHAR)
    {
        if(currenttype==INT)    Kind = INTKIND;
        if(currenttype==CHAR)   Kind = CHARKIND;
        Recall = Index;//保存程序判读失误需要返回的位置
        getnextsym();
        if(currenttype==IDENT)
        {
            strcpy(Name,currentsym);
            strcpy(Arg1.ident_type_value,currentsym);
            getnextsym();
            if(currenttype==COMMA || currenttype==L_BRACKET)//出现','和'['说明可能是变量定义
            {
                Index = Recall-1;//回到Index指向INT或CHAR之前的位置
                variable_define_sentence();//返回之后分号当前字符是分号
                getnextsym();//应该是分号
                if(currenttype!=SEMICOLON)
                {
                    printf("line:%d 变量定义语句应该以分号结尾\n",line);
                }
                printf("---%d---这是一个变量定义语句\n",line);
                getnextsym();
            }
            else if(currenttype==SEMICOLON)//如果只有一个变量，在这里就直接处理了，不需要调用variable_declare_sentence()
            {

                InsertTable();
                if(Kind==INTKIND)   Arg1.arg_type = 3;
                if(Kind==CHARKIND)   Arg1.arg_type = 4;
                Emit(GENVAR,Arg1,Arg2,Result);
                printf("---%d---这是一个变量定义语句\n",line);
                getnextsym();
            }
            else if(currenttype==L_PARENTHESES)//是左小括号说明这是个函数定义
            {
                Index = Recall-1;//返回之前记录的位置
                getnextsym();//应该读到了下一个函数的类型
                return;
            }
            else
            {
                SkipTo('\n');
                printf("line:%d 缺少分号\n",line);
            }
        }
        else
        {
            SkipTo('\n');
            printf("line:%d 类型符号后面没有标识符\n",line);
        }
    }
}


//----------------------变量定义---------------------------
//＜变量定义＞::=＜类型标识符＞(＜标识符＞|＜标识符＞'['＜无符号整数＞']'){,＜标识符＞|＜标识符＞'['＜无符号整数＞']' }
void variable_define_sentence()
{
    int arraylength = 0;//用于记录数组的长度
    int Type;
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    //进来的时候int或char还没读
    getnextsym();//int  char
    Type = (currenttype==INT) ? INTKIND : CHARKIND;
    do
    {
        Kind = Type;
        getnextsym();//ident
        strcpy(Name,currentsym);//int和char之后一定是标识符，至于标识符后面是不是还有[,则有待讨论
        strcpy(Arg1.ident_type_value,Name);//把变量名存下
        getnextsym();
        if(currenttype==COMMA || currenttype==SEMICOLON)//如果是逗号，则说明这里已经声明完了一个变量
        {
            InsertTable();//存入符号表
            if(Kind==INTKIND)   Arg1.arg_type = 3;
            if(Kind==CHARKIND)   Arg1.arg_type = 4;
            Emit(GENVAR,Arg1,Arg2,Result);//生成一个变量四元式

            sendbacksym();//把逗号或分号退回
        }
        else if(currenttype==L_BRACKET)//读到了左中括号，说明是数组定义
        {
            strcpy(Arg1.ident_type_value,Name);//把变量名存下
            if(Kind==INTKIND)   Kind = INTARRAYKIND;//如果之前是整型，这里改为整型数组
            else if(Kind==CHARKIND) Kind = CHARARRAYKIND;//如果之前是字符型，这里改为字符型数组
            getnextsym();//再读一个符号，应该是无符号整数
            if(currenttype==INTEGER)
            {
                arraylength = StringToInt();//记录下数组的长度
                Arg2.arg_type = 1;//用第二个Arg来记录长度
                Arg2.int_type_value = arraylength;
                getnextsym();//再读应该是右中括号
                if(currenttype==R_BRACKET)
                {
                    InsertTable();//插入符号表
                    Emit(GENARRAY,Arg1,Arg2,Result);//生成一个定义数组的四元式
                }
                else
                {
                    SkipTo('\n');
                    printf("line:%d 数组定义缺少右括号\n",line);
                }
            }
            else
            {
                SkipTo('\n');
                printf("line:%d 数组长度应该是无符号整数\n",line);
            }
        }
        else
        {
            SkipTo('\n');
            printf("line:%d 非法字符\n",line);
        }
        getnextsym();
        if(currenttype==SEMICOLON)  break;
    }while(currenttype==COMMA);
    sendbacksym();//把分号(正确情况)或任意字符退回，交由上一层函数处理
    return;
}

//int fun(int a,int b,int c){   }
void parameter_table()
{
    int i = 0;
    int paranum = 0;
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    int Para_Temp_Type;//临时存储参数类型
    getnextsym();
    if(currenttype==INT || currenttype==CHAR)//第一个参数的类型
    {
        do
        {
            Para_Temp_Type = currenttype;//类型赋给Para_Temp_Type
            Kind = (currenttype==INT) ? INTKIND : CHARKIND;
            if(currenttype==COMMA)  getnextsym();
            if(Para_Temp_Type==COMMA && (currenttype==INT || currenttype==CHAR))
            {
                Kind = (currenttype==INT) ? INTKIND : CHARKIND;
                Para_Temp_Type = currenttype;
            }
            head_define();//返回后这里可能是逗号
            strcpy(Arg1.ident_type_value,currentsym);
            Arg1.arg_type = 3;
            InsertTable();
            Emit(PARAMX,Arg1,Arg2,Result);//生成一个形参四元式
            paranum++;
            //print_symbol_table();
            P_parameter_num++;//记录参数数量
            getnextsym();
        }while(currenttype==COMMA);//不是逗号，则应该是右括号，类型应该是右括号

        for(i = ICL_index;i>=ICL_index-paranum;i--)//标记共有几个参数
        {
            if(ICL[i].OP==PARAMX)
            {
                ICL[i].Arg2.arg_type = 1;
                ICL[i].Arg2.int_type_value = paranum;
            }
        }
    }
}

void head_define()//头部定义
{
    if(currenttype==INT || currenttype==CHAR)
    {
        if(currenttype==INT)    Kind = INTKIND;
        else if(currenttype==CHAR)   Kind = CHARKIND;
        getnextsym();//应该读到一个标识符
        if(currenttype!=IDENT)
        {
            printf("line:%d int和char后面没有找到标识符\n",line);
            while(currenttype!=COMMA && currenttype!=L_PARENTHESES && currenttype!=R_PARENTHESES)   getnextsym();
            return;
        }
        else
        {
            strcpy(Name,currentsym);
        }
    }
    else
    {
        printf("line:%d 非法字符\n",line);
        while(currenttype!=COMMA && currenttype!=L_PARENTHESES && currenttype!=R_PARENTHESES)   getnextsym();
        return;
    }
    return;
}

int StringToInt()
{
    int i = 0;
    int number = 0;
    int len = strlen(currentsym);
    for(i = 0;i < len;i++)
    {
        number = number*10+(currentsym[i]-'0');
    }
    return number;
}

//＜复合语句＞   ::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞
void compound_sentence()//当前指向的单词类型是左括号////////////////生成四元式/////////////////////
{
    getnextsym();
    if(currenttype==R_BRACE)//是右括号，说明复合语句为空
    {
        sendbacksym();
        return;
    }
    int indexbegin = Tindex;//记录当前的位置

    while(currenttype==CONST)//常量说明
    {
        sendbacksym();
        const_declare_sentence();//此时的currenttype是分号
        getnextsym();//继续读，如果是const则执行while
    }
    P_const_number = Tindex-indexbegin;//差值是函数中的常量个数
    indexbegin = Tindex;
    if(currenttype==INT || currenttype==CHAR)//上面常量说明部分读到了一个INT或CHAR，自动进入变量说明部分
    {
        sendbacksym();//把读到的int或char退回去
        variable_declare_sentence();//返回之后当前字符是分号
    }
    P_var_number = Tindex-indexbegin;
    sendbacksym();
    sentence_set();//开始语句列的分析
    return;
}

void sentence_set()//语句列的第一个单词还没有读，语句列中没有'{''}'
{
    getnextsym();
    if(currenttype==R_BRACE)//说明语句列为空，到达函数结尾
    {
        sendbacksym();
        return;
    }
    while(currenttype==SEMICOLON || currenttype==IF || currenttype==WHILE || currenttype==L_BRACE || currenttype==IDENT || currenttype==SCANF || currenttype==PRINTF || currenttype==SWITCH || currenttype==RETURN)
    {
        sendbacksym();//退回单词
        sentence();//说明还是一个语句
        getnextsym();//继续读单词判断是不是还是语句
    }
    sendbacksym();
    printf("---%d---这是一个语句列\n",line);
    return;
}

//＜语句＞::=＜条件语句＞｜＜循环语句＞| '{'＜语句列＞'}'｜＜有返回值函数调用语句＞; | ＜无返回值函数调用语句＞;｜＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜空＞;|＜情况语句＞｜＜返回语句＞;

void sentence()//分号是sentence语句里的
{
    getnextsym();
    if(currenttype==IF)
    {
        sendbacksym();//退回读到的if
        if_sentence();
    }
    else if(currenttype==WHILE)
    {
        sendbacksym();
        while_sentence();
    }
    else if(currenttype==L_BRACE)
    {
        sentence_set();
        getnextsym();
        if(currenttype!=R_BRACE)
        {
            SkipTo('\n');
            printf("line:%d 缺少右大括号匹配\n",line);
        }
    }
    else if(currenttype==SCANF)
    {
        sendbacksym();
        scanf_sentence();
        getnextsym();
        if(currenttype==SEMICOLON)  return;
    }
    else if(currenttype==PRINTF)
    {
        sendbacksym();
        printf_sentence();
        getnextsym();
        if(currenttype==SEMICOLON)  return;
    }
    else if(currenttype==RETURN)
    {
        sendbacksym();
        return_sentence();
        getnextsym();//读到分号
        if(currenttype==SEMICOLON)  return;
    }
    else if(currenttype==SWITCH)
    {
        sendbacksym();//退回switch
        switch_sentence();
    }
    else if(currenttype==IDENT)//函数调用或赋值语句
    {
        look_up_type();
        int OP;
        ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
        strcpy(Arg1.ident_type_value,currentsym);
        getnextsym();
        if(currenttype==ASSIGN)//赋值语句
        {
            OP = ASSI;
            Arg1.arg_type = 3;
            Arg2 = expression();
            Arg1 = Emit(OP,Arg1,Arg2,Result);
            getnextsym();
            if(currenttype==SEMICOLON)
            {
                printf("---%d---这是一个赋值语句\n",line);
            }
            else
            {
                SkipTo('\n');
                printf("line:%d 没有分号结尾\n",line);
            }
        }
        else if(currenttype==L_BRACKET)//左中括号，说明是数组元素
        {
            Arg2 = expression();//用Arg2记录长度
            getnextsym();
            if(currenttype==R_BRACKET)
            {
                getnextsym();
                if(currenttype==ASSIGN)
                {
                    Result = expression();
                    getnextsym();
                    if(currenttype==SEMICOLON)
                    {
                        Arg1.arg_type = 3;
                        Arg1 = Emit(ASSIARRAY,Arg1,Arg2,Result);
                        printf("---%d---这是一个给数组元素赋值的语句\n",line);
                    }
                    else
                    {
                        SkipTo('\n');
                        printf("line:%d 缺少分号\n",line);
                    }
                }
                else
                {
                    SkipTo('\n');
                    printf("line:%d 没有找到赋值符号，语法错误\n",line);
                }
            }
            else
            {
                SkipTo('\n');
                printf("line:%d 中括号不匹配\n",line);
            }
        }
        else if(currenttype==L_PARENTHESES)//出现左小括号，说明是函数调用语句
        {
            parameter_sentence();//进行参数分析
            getnextsym();
            if(currenttype==R_PARENTHESES)//右括号也匹配
            {
                getnextsym();
                if(currenttype==SEMICOLON)
                {
                    OP = CALL;
                    Arg1 = Emit(OP,Arg1,Arg2,Result);
                    printf("---%d---这是一个无返回值的函数调用语句\n",line);
                    return;
                }
                else
                {
                    SkipTo('\n');
                    printf("line:%d 缺少分号\n",line);
                }
            }
            else
            {
                SkipTo('\n');
                printf("line:%d 括号不匹配\n",line);
            }
        }
        else
        {
            SkipTo('\n');
            printf("line:%d 非法字符\n",line);
        }
    }
    else if(currenttype==SEMICOLON)//空语句
    {
        printf("---%d---这是一个空语句\n",line);
        return;
    }
    else if(currenttype==R_BRACE)//右大括号说明语句列结束
    {
        printf("---%d---语句列分析结束\n",line);
        sendbacksym();
        return;
    }
    else
    {
        SkipTo('\n');
        printf("line:%d 非法字符\n",line);
    }
}

void parameter_sentence()//记录函数调用过程中的参数个数
{
    int sum = 0;
    int i = 0;
    ArgNode arrA1[100],arrA2[100];//最多有100个参数
    int arrind = 0;
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    getnextsym();
    if(currenttype==R_PARENTHESES)
    {
        sendbacksym();
        return;
    }
    else    sendbacksym();
    do
    {
        Arg1 = expression();
        Arg2.arg_type = 1;
        Arg2.int_type_value = sum++;
        arrA1[arrind] = Arg1;
        arrA2[arrind] = Arg2;
        arrind++;
        getnextsym();
    }while(currenttype==COMMA);
    if(currenttype==R_PARENTHESES)
    {
        printf("---%d---值参数表分析完毕\n",line);
        for(i = 0;i < arrind;i++)
        {
            Emit(PARAM,arrA1[i],arrA2[i],Result);
        }
        sendbacksym();//把右括号退回了，后面的有返回值的函数调用语句需要做相应的修改
        return;
    }
    else
    {
        SkipTo('\n');
        printf("line:%d 括号不匹配\n",line);
    }
    return;
}

void if_sentence()
{
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    getnextsym();//读到if

    getnextsym();
    if(currenttype!=L_PARENTHESES)
    {
        SkipTo('\n');
        printf("line:%d if后面缺少左括号\n",line);
    }
    condition_sentence();
    getnextsym();
    if(currenttype!=R_PARENTHESES)
    {
        SkipTo('\n');
        printf("line:%d 条件语句结束没有右括号\n",line);
    }
    else
    {
        printf("---%d---这是一个if语句\n",line);//当前符号是右括号
    }
    sentence();//if后面是一个语句
    Fill(Fillindex[FI--],ICL_index+2);//如果之前if不满足则跳到这之后的两条指令位置
    Emit(JMP,Arg1,Arg2,Result);//执行完if的内容则需要跳过else的内容
    Fillindex[++FI] = ICL_index;//等待回填
    getnextsym();//语句分析结束读取下一个单词
    if(currenttype==ELSE)
    {
        sentence();//else后面也是一个语句
        Fill(Fillindex[FI--],ICL_index+1);
    }
    else//没有else不用跳转，处理成跳转到原处
    {
        //Fill(Fillindex[FI--],Fillindex[FI+1]);
        FI--;
        sendbacksym();//如果不是else则退回误读的单词
    }
    printf("---%d---条件语句分析结束\n",line);
}
/////////////////////////////////////////////////////////////////////////////////////////
//＜条件＞::=＜表达式＞＜关系运算符＞＜表达式＞｜＜表达式＞ //表达式为0条件为假，否则为真
/////////////////////////////////////////////////////////////////////////////////////////
void condition_sentence()
{
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    OpType OP;

    Arg1 = expression();
    getnextsym();
    if(currenttype==EQUAL || currenttype==LARGER || currenttype==SMALLER || currenttype==NOT_EQUAL || currenttype==NOT_LARGER || currenttype==NOT_SMALLER)
    {
        if(currenttype==EQUAL)  OP = JNE;
        else if(currenttype==LARGER)  OP = JLE;
        else if(currenttype==SMALLER)  OP = JGE;
        else if(currenttype==NOT_EQUAL)  OP = JE;
        else if(currenttype==NOT_LARGER)  OP = JG;
        else if(currenttype==NOT_SMALLER)  OP = JL;
        Arg2 = expression();
        Arg1 = Emit(CMP,Arg1,Arg2,Result);
        Emit(OP,Arg1,Arg2,Result);//不满足条件时跳转
        Fillindex[++FI] = ICL_index;//记录这一条需要回填的指令

        printf("---%d---这是一个条件语句\n",line);
    }
    else if(currenttype==R_PARENTHESES)//如果只有一个表达式
    {
        Arg2.arg_type = 1;//如果只有一个表达式，就是和0相比，若想等则判断失败，则跳转
        Arg2.int_type_value = 0;
        Arg1 = Emit(CMP,Arg1,Arg2,Result);
        Emit(JE,Arg1,Arg2,Result);
        Fillindex[++FI] = ICL_index;//记录下需要回填的指令
        sendbacksym();//把读到的右括号退回
    }
    else
    {
        SkipTo('\n');
        printf("line:%d 非法字符\n",line);
    }
    return;
}
//＜循环语句＞::=while'('＜条件＞')'＜语句＞
void while_sentence()
{
    char str[10];
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    getnextsym();//读到while
    getnextsym();//应该读到左括号

    Arg1.arg_type = 3;
    itoa(ICL_index+2,str,10);
    Arg1.ident_type_value[0] = 'A';
    Arg1.ident_type_value[1] = '\0';
    strcat(Arg1.ident_type_value,str);//产生一个label
    Emit(LABEL,Arg1,Arg2,Result);
    Fillindex[++FI] = ICL_index+1;//跳回来的位置
    if(currenttype==L_PARENTHESES)
    {
        condition_sentence();
        getnextsym();
        if(currenttype==R_PARENTHESES)//有右括号匹配
        {
            sentence();
            Fill(Fillindex[FI--],ICL_index+2);//如果判断失败，跳转到下一条指令，填condition_sentence产生的跳转指令
            Emit(JMP,Arg1,Arg2,Result);
            Fill(ICL_index,Fillindex[FI--]);//跳回while开始处
            printf("---%d---这是一个while语句\n",line);
        }
        else
        {
            SkipTo('\n');
            printf("line:%d 条件语句右侧括号不匹配\n",line);
        }
    }
    else
    {
        SkipTo('\n');
        printf("line:%d while后面应该是左括号\n",line);
    }
}

void scanf_sentence()
{
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    getnextsym();//读到了scanf
    getnextsym();//读到左括号
    if(currenttype==L_PARENTHESES)//如果是左括号，符合语法规范
    {
        getnextsym();
        if(currenttype==R_PARENTHESES)
        {
            SkipTo('\n');
            printf("line:%d 没有读入内容\n",line);
            return;
        }
        if(currenttype==INT || currenttype==CHAR)
        {
            SkipTo('\n');
            printf("line:%d 读语句中没有变量\n",line);
        }
        while(currenttype==IDENT)//这里应该填符号表
        {
            if(look_up_type()==-1)  return;//查找是否定义
            if(look_up_type()==INTKIND)    Arg1.arg_type = 3;
            if(look_up_type()==CHARKIND)   Arg1.arg_type = 4;
            strcpy(Arg1.ident_type_value,currentsym);
            if(Arg1.arg_type==3)    Emit(SCANINT,Arg1,Arg2,Result);
            else if(Arg1.arg_type==4)   Emit(SCANCHAR,Arg1,Arg2,Result);
            printf("---%d---读入一个标识符\n",line);
            getnextsym();//再读一个单词，可能是逗号，可能是右括号
            if(currenttype==COMMA)
            {
                getnextsym();
            }
            else if(currenttype==R_PARENTHESES) break;//读到了右括号读入结束
            else
            {
                SkipTo('\n');
                printf("line:%d 非法字符\n",line);
            }
        }
        getnextsym();
        if(currenttype==SEMICOLON)
        {
            printf("---%d---这是一个读语句\n",line);
            sendbacksym();
            return;
        }
        else
        {
            printf("line:%d 读语句结尾没有分号\n",line);
            sendbacksym();
            return;
        }
    }
    else
    {
        SkipTo('\n');
        printf("line:%d 括号不匹配\n",line);
    }
}

void printf_sentence()
{
    getnextsym();//读到printf
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    getnextsym();
    if(currenttype==L_PARENTHESES)
    {
        getnextsym();
        if(currenttype==STRING)//输出一个字符串
        {
            Arg1.arg_type = 8;//表示输出字符串
            strcpy(Arg1.string_type_value,currentsym);//存下字符串变量
            Emit(PRINTSTR,Arg1,Arg2,Result);//输出一个字符串
            getnextsym();
            if(currenttype==COMMA)
            {
                Arg1 = expression();//处理表达式
                getnextsym();//再读一个单词，应该是右括号
                if(currenttype==R_PARENTHESES)
                {
                    if(Arg1.arg_type==3 || Arg1.arg_type==7)//输出int型变量
                    {
                        Emit(PRINTINTIDENT,Arg1,Arg2,Result);
                    }
                    else if(Arg1.arg_type==4)
                    {
                        Emit(PRINTCHARIDENT,Arg1,Arg2,Result);//输出字符型变量
                    }
                    else if(Arg1.arg_type==1)
                    {
                        Emit(PRINTINT,Arg1,Arg2,Result);//输出整数
                    }
                    else if(Arg1.arg_type==2)
                    {
                        Emit(PRINTCHAR,Arg1,Arg2,Result);//输出字符
                    }
                    else if(Arg1.arg_type==5)
                    {
                        Emit(PRINTINTIDENT,Arg1,Arg2,Result);//输出数字变量
                    }
                    else if(Arg1.arg_type==6)
                    {
                        Emit(PRINTCHARIDENT,Arg1,Arg2,Result);//输出字符变量
                    }
                    printf("---%d---这是一个输出语句,输出了字符串和表达式\n",line);
                }
                else
                {
                    SkipTo('\n');
                    printf("line:%d 括号不匹配\n",line);
                }
            }
            else if(currenttype==R_PARENTHESES)
            {
                printf("---%d---这是一个输出语句，输出了一个字符串\n",line);
            }
            else
            {
                SkipTo('\n');
                printf("line:%d 非法字符\n",line);
            }
        }
        else
        {
            sendbacksym();
            Arg1 = expression();
            if(Arg1.arg_type==3 || Arg1.arg_type==7 || Arg1.arg_type==5)//输出int型变量
            {
                Emit(PRINTINTIDENT,Arg1,Arg2,Result);
            }
            else if(Arg1.arg_type==4 || Arg1.arg_type==6)
            {
                Emit(PRINTCHARIDENT,Arg1,Arg2,Result);//输出字符型变量
            }
            else if(Arg1.arg_type==1)
            {
                Emit(PRINTINT,Arg1,Arg2,Result);//输出整数
            }
            else if(Arg1.arg_type==2)
            {
                Emit(PRINTCHAR,Arg1,Arg2,Result);//输出字符
            }
            getnextsym();
            if(currenttype==R_PARENTHESES)
            {
                printf("---%d---这是一个输出语句，输出了表达式\n",line);
            }
            else
            {
                SkipTo('\n');
                printf("line:%d 非法字符\n",line);
            }
        }
        getnextsym();
        if(currenttype==SEMICOLON)
        {
            sendbacksym();//退回分号
            printf("---%d---输出语句分析结束\n",line);
        }
        else
        {
            sendbacksym();
            printf("line:%d 语句结尾没有分号\n",line);
        }
        return;
    }
    else
    {
        SkipTo('\n');
        printf("line:%d printf标识后没有括号\n",line);
    }
}

void return_sentence()
{
    getnextsym();//读到return
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    OpType OP = RETURNNVOID;
    if(returnflag==3)
    {
        return;
    }
    getnextsym();
    if(currenttype==SEMICOLON)
    {
        if(returnflag==1||returnflag==2)
        {
            printf("line:%d 语义错误，函数没有返回值\n",line);
            return;
        }
        Emit(RETURNVOID,Arg1,Arg2,Result);
        sendbacksym();
        printf("---%d---这是一个无返回值的返回语句\n",line);
        return;
    }
    if(currenttype==L_PARENTHESES)//否则，进行表达式分析
    {
        if(returnflag==0)
        {
            printf("line:%d void函数不能有返回值\n",line);
            return;
        }
        Arg1 = expression();
        getnextsym();
        if(currenttype==R_PARENTHESES)
        {
            getnextsym();
            if(currenttype==SEMICOLON)
            {
                if(returnflag==1 && (Arg1.arg_type==2||Arg1.arg_type==4||Arg1.arg_type==6)) printf("line:%d 返回类型不符\n",line);
                else if(returnflag==2 && (Arg1.arg_type==1||Arg1.arg_type==3||Arg1.arg_type==5)) printf("line:%d 返回类型不符\n",line);
                else if(returnflag==0)  printf("line:%d void函数不能有返回值\n",line);
                else
                {
                    Emit(OP,Arg1,Arg2,Result);
                    printf("---%d---这是一个有返回值的返回语句\n",line);
                    sendbacksym();//退回分号
                }
                return;
            }
            else
            {
                SkipTo('\n');
                printf("line:%d 非法字符\n",line);
            }
        }
        else
        {
            SkipTo('\n');
            printf("line:%d 返回语句括号不匹配\n",line);
        }
    }
    else
    {
        SkipTo('\n');
        printf("line:%d return后应该有左括号\n",line);
    }
}

void switch_sentence()
{
    getnextsym();//读到switch
    ArgNode Arg1 = InitArgNode();
    getnextsym();
    if(currenttype==L_PARENTHESES)
    {
        Arg1 = expression();//switch的判断条件是一个表达式
        getnextsym();
        if(currenttype==R_PARENTHESES)
        {
            getnextsym();
            if(currenttype==L_BRACE)
            {
                situation_table(Arg1);
            }
            else
            {
                SkipTo('\n');
                printf("line:%d 大括号不匹配\n",line);
            }
        }
        else
        {
            SkipTo('\n');
            printf("line:%d 括号不匹配\n",line);
        }
    }
    else
    {
        SkipTo('\n');
        printf("line:%d 缺少小括号\n",line);
    }
}

void situation_table(ArgNode A1)//情况表，当前符号是
{
    int i = 0;
    getnextsym();
    while(currenttype==CASE)
    {
        sendbacksym();
        situation_sentence(A1);
        casenum++;
        getnextsym();
    }
    if(currenttype==DEFAULT)
    {
        sendbacksym();
        default_sentence();
        getnextsym();
    }
    if(currenttype==R_BRACE)//如果是右大括号，则匹配
    {
        for(i = 0;i < casenum;i++)
        {
            Fill(Fillswitch[i],ICL_index+1);
        }
        casenum = 0;
        printf("---%d---这是一个情况语句\n",line);
    }
    return;
}

void situation_sentence(ArgNode A1)//此时case还没有读
{
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    getnextsym();//应该读到了case
    char PM;
    if(currenttype==CASE)
    {
        getnextsym();
        if(currenttype==INTEGER || currenttype==CHARACTER || currenttype==PLUS || currenttype==MINUS)
        {
            if(currenttype==PLUS || currenttype==MINUS)
            {
                if(currenttype==PLUS)   PM = '+';
                if(currenttype==MINUS)  PM = '-';
                getnextsym();
                if(currenttype==INTEGER)
                {
                    Arg1.arg_type = 1;
                    if(PM=='+') Arg1.int_type_value = StringToInt(currentsym);
                    if(PM=='-') Arg1.int_type_value = 0-StringToInt(currentsym);
                }
            }
            else if(currenttype==INTEGER)
            {
                Arg1.arg_type = 1;
                Arg1.int_type_value = StringToInt(currentsym);
            }
            else if(currenttype==CHARACTER)
            {
                Arg1.arg_type = 2;
                Arg1.char_type_value = currentsym[0];
            }
            Arg1 = Emit(CMP,A1,Arg1,Result);//比较switch后的值和case后的值
            Arg1 = Emit(JNE,Arg1,Arg2,Result);//如果不相等，跳转
            Fillindex[++FI] = ICL_index;
            getnextsym();
            if(currenttype==COLON)
            {
                sentence_set();
                Arg1 = Emit(JMP,Arg1,Arg2,Result);
                Fillswitch[casenum] = ICL_index;//第casenum个case得到执行，需要跳转到switch结束
                Fill(Fillindex[FI--],ICL_index+1);//这个case如果不相等，则跳转到下一个case
                printf("---%d---这是一个CASE语句\n",line);
            }
            else
            {
                SkipTo('\n');
                printf("line:%d 缺少冒号\n",line);
            }
        }
        else
        {
            SkipTo('\n');
            printf("line:%d 这里应该是常量\n",line);
        }
    }
    else
    {
        printf("line:%d 缺少“case”",line);
        SkipTo('}');
    }
    return;
}

void default_sentence()
{
    getnextsym();
    if(currenttype==DEFAULT)
    {
        getnextsym();
        if(currenttype==COLON)
        {
            sentence();
            printf("---%d---这是一个default语句\n",line);
        }
        else    printf("line:%d 缺少冒号\n",line);
    }
    else
    {
        printf("line:%d 缺少“default”",line);
        SkipTo('}');
    }
    return;
}

ArgNode expression()
{
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    OpType OP = UNKNOWN;//unknown不能省略

    getnextsym();
    if(currenttype==PLUS || currenttype==MINUS)
    {
        if(currenttype==PLUS)   OP = ADD;
        if(currenttype==MINUS)  OP = SUB;
        Arg1.arg_type = 1;
        Arg1.int_type_value = 0;
        Arg2 = item();
        Arg1 = Emit(OP,Arg1,Arg2,Result);
        getnextsym();
    }
    else
    {
        sendbacksym();//如果不是加减号，退回
        Arg1 = item();//当前符号不是乘除
        getnextsym();//读到加减号或结束符
    }
    while(currenttype==PLUS || currenttype==MINUS)//测试多个数连续加减
    {
        OP = currenttype==PLUS ? ADD : SUB;
        Arg2 = item();

        Arg1 = Emit(OP,Arg1,Arg2,Result);
        getnextsym();
    }
    sendbacksym();
    OP = UNKNOWN;
    Arg1 = Emit(OP,Arg1,Arg2,Result);
    printf("---%d---这是一个表达式\n",line);
    return Arg1;
}

ArgNode item()
{
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    Arg1 = factor();
    getnextsym();//读取因子后面的符号
    while(currenttype==MULTIPLY || currenttype==DIVIDE)
    {
        if(currenttype==MULTIPLY)
        {
            Arg2 = factor();//是乘法符号
            Arg1 = Emit(MUL,Arg1,Arg2,Result);
            getnextsym();
        }
        else
        {
            Arg2 = factor();//是除法符号
            Arg1 = Emit(DIV,Arg1,Arg2,Result);
            getnextsym();
        }
    }
    sendbacksym();
    printf("---%d---这是一个项\n",line);//当读到了一个不是乘除的符号，结束项的分析

    return Arg1;
}

//＜因子＞::=＜标识符＞｜＜标识符＞'['＜表达式＞']'｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞|'('＜表达式＞')'
ArgNode factor()
{
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    getnextsym();
    char PM;
    int Type;
    if(currenttype==IDENT)//读到标识符，说明是标识符或数组元素或有返回值的函数
    {
        if(look_up_type()==-1)  return Arg1;;
        Type = look_up_type();//如果是函数，则记录函数的类型
        strcpy(Arg1.ident_type_value,currentsym);//记录标识符名字
        if(Find_const()==1)//如果是常量则替换为相应的整数值
        {
            Arg1.int_type_value = Rep_int_const();
            Arg1.arg_type = 1;
        }
        else if(Find_const()==2)//如果是常量则替换为相应的字符值
        {
             Arg1.char_type_value = Rep_char_const();
             Arg1.arg_type = 2;
        }
        else    Arg1.arg_type = (look_up_type()==INTKIND || look_up_type()==CONSTINTKIND) ? 3 : 4;
        getnextsym();

        if(currenttype==L_BRACKET)//数组元素
        {
            Arg2 = expression();//"表达式"
            getnextsym();//再读一个单词应该是右中括号
            if(currenttype==R_BRACKET)//右中括号说明数组元素定义完整
            {
                printf("---%d---这是一个数组元素\n",line);
                Arg1 = Emit(ARRAY,Arg1,Arg2,Result);
            }
            else
            {
                printf("line:%d 括号不匹配\n",line);
                SkipTo('\n');
            }
        }
        else if(currenttype==L_PARENTHESES)//如果是左括号，函数调用//////////////////////
        {
            sendbacksym();
            sendbacksym();//回退两个单词
            return_function();//有返回值的函数处理
            Arg1 = Emit(CALL,Arg1,Arg2,Result);
            if(Type==INTFUNCTIONKIND)   Arg1 = Emit(GETINTRVALUE,Arg1,Arg2,Result);
            if(Type==CHARFUNCTIONKIND)  Arg1 = Emit(GETCHARRVALUE,Arg1,Arg2,Result);
        }
        else//只是一个单独的标识符
        {
            sendbacksym();//退回误读的单词
            printf("---%d---这是一个标识符\n",line);
        }
    }
    else if(currenttype==INTEGER || currenttype==CHARACTER || currenttype==PLUS || currenttype==MINUS)
    {
        if(currenttype==PLUS || currenttype==MINUS)
        {
            if(currenttype==PLUS)   PM = '+';
            if(currenttype==MINUS)  PM = '-';
            getnextsym();
            if(currenttype==INTEGER)
            {
                Arg1.arg_type = 1;
                if(PM=='+') Arg1.int_type_value = StringToInt(currentsym);
                if(PM=='-')
                {
                    Arg1.int_type_value = 0-StringToInt(currentsym);
                }
            }
        }
        else if(currenttype==INTEGER)
        {
            Arg1.arg_type = 1;
            Arg1.int_type_value = StringToInt(currentsym);
        }
        else if(currenttype==CHARACTER)
        {
            Arg1.arg_type = 2;
            Arg1.char_type_value = currentsym[0];
        }
        printf("---%d---这是一个整数或字符\n",line);
    }
    else if(currenttype==L_PARENTHESES)
    {
        Arg1 = expression();
        getnextsym();
        if(currenttype!=R_PARENTHESES)
        {
            printf("line:%d 括号不匹配\n",line);
            SkipTo('\n');
        }
    }
    else
    {
        printf("line:%d 非法字符\n",line);
        SkipTo('\n');
    }
    return Arg1;
}

void return_function()//有返回值的函数调用语句处理
{
    getnextsym();//应该读到了函数名
    if(currenttype==IDENT)
    {
        getnextsym();

        if(currenttype==L_PARENTHESES)
        {
            parameter_sentence();
            getnextsym();//当前符号应该是右括号
            if(currenttype==R_PARENTHESES)
            {
                printf("---%d---这是一个有返回值的函数调用语句\n",line);
            }
            else    printf("line:%d 括号不匹配\n",line);
        }
        else    printf("line:%d 没有左括号\n",line);
    }
    else    printf("line:%d 非法字符\n",line);
}

void Parse()
{
    procedure();
    //print_symbol_table();
    //print_procedure_table();
    //print_ICL();
    genmips();
    return;
}
