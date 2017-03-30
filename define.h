#include <stdlib.h>
#include <stdio.h>


#define true 1
#define false 0
#define MAXFILE 10240           //���Զ��������ļ�10KB
#define MAXFILENAME 255         //�ļ�����󳤶�
#define MAXTOKENBUF 100         //�ַ��������ĳ���
#define ERRORNUM 128            //����Ĵ�������
#define ERRORLEN 128            //ÿ��������ʾ�䳤��
#define RESERVELIST 100         //����������
//////////////////////// �ʷ����� /////////////////////////////////////////////
//������
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

//����
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

//��������
#define IDENT 35            //��ʶ��
#define INTEGER 36          //����
#define CHARACTER 37        //�ַ�
#define STRING 38           //�ַ���

//���� ���� ����
#define INT_CONST 39        //���ͳ���
#define CHAR_CONST 40       //�ַ��ͳ���
#define INT_VAR 41          //i���ͱ���
#define CHAR_VAR 42         //�ַ��ͱ���
#define FUNCTION 43         //����

//����
#define MAIN 44             //������
#define EOFSY 45              //������
#define PRINTF 46           //printf����
#define SCANF 47            //scanf����
///////////////////////////////// ������� ////////////////////////////////////////////////
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
    int Addr;//�����ڱ��е�λ��
    char Name[100];
    int Kind;
    int line;//�����ڳ���ĵڼ���
    int DigitValue;//���Kind������
    char CharValue;//���Kind���ַ�
    char StringValue[1000];//���Kind���ַ���
    int Size;//���Kind������
    int ProIndex;//�����ڷֳ�����е�λ��
}SymTable;

typedef struct
{
    char PName[30];
    int PAddr;//�ڷֳ�����е����
    int TAddr;//�ڷ��ű��еĻ���ַ
    int parameter_num;//��¼��������
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
    int arg_type;//���������ͣ�1����int��2����char��3����intident��4����charident��5����inttemp��6����chartemp��7����temp��8����string��9����function
    int int_type_value;
    char char_type_value;
    char string_type_value[200];
    char ident_type_value[100];
}ArgNode;

typedef struct
{
    OpType OP;//����������
    ArgNode Arg1;
    ArgNode Arg2;
    ArgNode Result;
    int JumpTo;//��¼��תλ��
}InterCode;//��Ԫʽ

///////////////��������///////////////////////////
typedef struct
{
    char name[30];
    int addr;
}Node;
//////////////////////////////////////////////////
