#include <stdlib.h>
#include <stdio.h>


#define true 1
#define false 0
#define MAXFILE 10240           //可以读入的最大文件10KB
#define MAXFILENAME 255         //文件名最大长度
#define MAXTOKENBUF 100         //字符缓冲区的长度
#define ERRORNUM 128            //定义的错误数量
#define ERRORLEN 128            //每个错误提示句长度
#define RESERVELIST 100         //保留字数量
//////////////////////// 词法分析 /////////////////////////////////////////////
//保留字
#define BREAK 1
#define CASE 2
#define CHAR 3
#define CONST 4
#define DEFAULT 5
#define IF 6
#define ELSE 7
#define INT 8
#define RETURN 9
#define SWITCH 10
#define VOID 11
#define WHILE 12

//符号
#define PLUS 13             // '+'
#define MINUS 14            // '-'
#define MULTIPLY 15         // '*'
#define DIVIDE 16           // '/'
#define L_PARENTHESES 17    // '('
#define R_PARENTHESES 18    // ')'
#define L_BRACKET 19        // '['
#define R_BRACKET 20        // ']'
#define L_BRACE 21          // '{'
#define R_BRACE 22          // '}'
#define SEMICOLON 23        // ';'
#define COMMA 24            // ','
#define SINGLEQUOTES 25     // '''
#define DOUBLEQUOTES 26     // '"'
#define COLON 27            // ':'
#define EQUAL 28            // '=='
#define LARGER 29           // '>'
#define SMALLER 30          // '<'
#define NOT_EQUAL 31        // '!='
#define NOT_LARGER 32       // '<='
#define NOT_SMALLER 33      // '>='
#define ASSIGN 34           // '='

//单词种类
#define IDENT 35            //标识符
#define INTEGER 36          //整数
#define CHARACTER 37        //字符
#define STRING 38           //字符串

//常量 变量 函数
#define INT_CONST 39        //整型常量
#define CHAR_CONST 40       //字符型常量
#define INT_VAR 41          //i整型变量
#define CHAR_VAR 42         //字符型变量
#define FUNCTION 43         //函数

//其它
#define MAIN 44             //主函数
#define EOFSY 45              //结束符
#define PRINTF 46           //printf函数
#define SCANF 47            //scanf函数
///////////////////////////////// 语义分析 ////////////////////////////////////////////////
#define INTKIND 48
#define CHARKIND 49
#define CONSTINTKIND 50
#define CONSTCHARKIND 51
#define INTFUNCTIONKIND 52
#define CHARFUNCTIONKIND 53
#define VOIDFUNCTIONKIND 54
#define PARAMETER 55
#define INTARRAYKIND 56
#define CHARARRAYKIND 57
#define STRINGKIND 58

typedef struct
{
    int Id;
    int Type;
    int Line;
    char Name[50];
    char Value[100];
}Reg;

typedef struct
{
    int Addr;//符号在表中的位置
    char Name[100];
    int Kind;
    int line;//符号在程序的第几行
    int DigitValue;//如果Kind是数字
    char CharValue;//如果Kind是字符
    char StringValue[1000];//如果Kind是字符串
    int Size;//如果Kind是数组
    int ProIndex;//函数在分程序表中的位置
}SymTable;

typedef struct
{
    char PName[30];
    int PAddr;//在分程序表中的序号
    int TAddr;//在符号表中的基地址
    int parameter_num;//记录参数个数
    int const_num;
    int var_num;
}ProTable;

typedef enum
{
    ADD,SUB,MUL,DIV,ARRAYVALUE,ARRAYADDR,OPPO,ASSI,ASSIGNRVALUE,ARRAYASSIGN,
    CMP,JE,JNE,JL,JLE,JG,JGE,JMP,PARAM,CALL,GETINTRVALUE,GETCHARRVALUE,RETURNNVOID,VRETURN,PRINTSTR,PRINTINTIDENT,PRINTCHARIDENT,PRINTINT,PRINTCHAR,SCANINT,SCANCHAR,
    LABEL,GENCONST,GENVAR,PARAMX,RETURNVOID,RETURNMAIN,GENARRAY,ARRAY,ASSIARRAY,ONEORZERO,UNKNOWN
}OpType;

typedef struct
{
    int arg_type;//操作数类型，1——int，2——char，3——intident，4——charident，5——inttemp，6——chartemp，7——temp，8——string，9——function
    int int_type_value;
    char char_type_value;
    char string_type_value[200];
    char ident_type_value[100];
}ArgNode;

typedef struct
{
    OpType OP;//操作符种类
    ArgNode Arg1;
    ArgNode Arg2;
    ArgNode Result;
    int JumpTo;//记录跳转位置
}InterCode;//四元式

///////////////代码生成///////////////////////////
typedef struct
{
    char name[30];
    int addr;
}Node;
//////////////////////////////////////////////////
