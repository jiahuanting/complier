#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"

extern Reg RegisterList[1000];//��¼ʶ������ĵ���
extern int ListIndex;//��������
extern void error(int errornum);
extern int line;
extern void print_ICL();
extern ArgNode InitArgNode();
extern int ICL_index;
extern void genmips();


extern InterCode ICL[1000];//��ԪʽInterCodeList
extern int ICL_index;//��Ԫʽ�б�ָ��
extern ArgNode Emit(int OP,ArgNode Arg1,ArgNode Arg2,ArgNode R);//������Ԫʽ

char currentsym[100];//currentsym�ǵ�ǰ�ĵ��ʣ�����տ�ʼ��Ϊ���鶨��̫С��û���洢�ϳ����ַ������´���
int currenttype;//��ǰ���ʵ�����
char lastsym[100];//lastsym��һ������
char lasttype;//��һ�����ʵ�����
int Index = 0;//ָ���λ��ָ����һ������
int Level = 0;//��¼��ǰ����εĲ��
int bound = 0;//��¼��ǰ�����ڷ��ű��еĿ�ʼλ��
int global_bound = 10000;//ȫ�ֱ���������

/*���ű���Ҫ�õ��ı���*/
SymTable Table[200];//���ű�
char Name[30];
int Kind;
int DigitValue;
char CharValue;
char StringValue[1000];
int Size;
int Level;
/*�ֳ����*/
ProTable PTable[100];//�ֳ����
int P_parameter_num;//�����������
int P_const_number;//�����еĳ�������
int P_var_number;//�����еı�������
int Pindex = 0;//�ֳ����ָ��

int Tindex = 0;
/**/
int Recall = 0;//���ڼ�¼�����ж�ʧ����Ҫ���ص�λ��

int const_num;//��¼���һ��������λ��
int var_num;//��¼���һ��������λ��
int function_num;//��¼���һ��������λ��
//////////////////////////////////////////////////////
int returnflag = 0;//0��void 1��int 2��char 3��main
/////////////////////////////////////////////////////
int Fillindex[100];//��Ҫ�����ָ��λ�á�����if��while���Ƚ��ȳ�
int FI = -1;
extern void Fill(int instruct,int address);//���ڻ�����ת��ַ
int casenum = 0;//��¼switch����µ�case����
int Fillswitch[20];
int Fillswitchend = 0;
/////////////////////////////////////////////////////
extern void genmips();
/////////////////////////////////////////////////////////
void Parse();
void getnextsym();//��ȡ��һ���ַ�
void sendbacksym();//�˻�һ���ַ�
void procedure();//���ĳ���
void const_declare_sentence();//����˵�����
void const_define_sentence();//�����������
void variable_declare_sentence();//����˵�����
void variable_define_sentence();//�����������
void parameter_table();//�����������
void insert_paranumber_to_function(int i);//�����в������
void head_define();//ͷ������
int StringToInt();//���ַ�����ʽ�����ֱ�ʾΪ����
void InsertTable();//������ű�

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
void return_function();//�з���ֵ�ĺ���������䴦��
void para_table();//
void parameter_sentence();//��¼�������ù����еĲ�������
void switch_sentence();
void situation_table(ArgNode A1);
void situation_sentence(ArgNode A1);
void default_sentence();
void InsertTable();//������ű�
int look_up_type();//���ص�ǰ���ʵ�����
int Find_const();//�鿴�Ƿ��ǳ���
int Rep_int_const();//�滻int����
char  Rep_char_const();//�滻char����
int Check_Dulplicate();//�����Ƿ����ظ�����
char Name_[][30] = {"ADD","SUB","MUL","DIV","ARRAYVALUE","ARRAYADDR","OPPO","ASSI","ASSIGNRVALUE","ARRAYASSIGN",
                       "CMP","JE","JNE","JL","JLE","JG","JGE","JMP","PARAM","CALL","GETRVALUE","RRETURN","VRETURN",
                       "PRINT","SCAN","LABEL","GENCONST","PARAMX","RETURNVOID","RETURNMAIN","GENARRAY","UNKNOWN"};


int Find_const()//���ҵ�ǰʶ��ı�ʶ���ǲ��ǳ���
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

int Rep_int_const()//�������滻Ϊֵ
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

char  Rep_char_const()//�������滻Ϊֵ
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

void InsertTable()//������ű�
{
    if(Kind!=INTFUNCTIONKIND && Kind!=CHARFUNCTIONKIND && Kind!=VOIDFUNCTIONKIND)   Check_Dulplicate();
    Table[Tindex].Addr = Tindex;
    strcpy(Table[Tindex].Name,Name);//name ��������ſ�����������
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
        printf("%s ������ű����û�е�ǰ����\n",currentsym);
        Tindex--;
    }
    //����
    int i = 0;
    for(i = 0;i<strlen(Name);i++)   Name[i] = '\0';
    DigitValue = 0;
    CharValue = '\0';
    for(i = 0;i<strlen(StringValue);i++)   StringValue[i] = '\0';
    Size = 0;
    //ָ��+1
    Tindex++;
}

int look_up_type()//�鿴��ǰ���ʵ����ͣ���scanf_sentence�б�����
{
    int i = 0;
    for(i = Tindex;i>=0;i--)
    {
        if(strcmp(currentsym,Table[i].Name)==0)
        {
            return Table[i].Kind;
        }
    }
    printf("line:%d %s����δ����\n",line,currentsym);
    return -1;
}

int Check_Dulplicate()
{
    int i = 0;
    if(global_bound==10000)//˵����ʱ����ȫ�ֱ�������׶�
    {
        for(i = 0;i <= Tindex && i <= global_bound;i++)
        {
            if(strcmp(Table[i].Name,Name)==0)
            {
                printf("line:%d %s �ظ����壡\n",line,currentsym);
                return false;
            }
        }
        for(i = 0;i < Pindex;i++)
        {
            if(strcmp(PTable[i].PName,Name)==0)
            {
                printf("line:%d %s �ظ����壡\n",line,currentsym);
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
                printf("line:%d %s �ظ����壡\n",line,Name);
                return false;
            }
        }
    }
    return true;
}

void print_symbol_table()//��ӡ���ű����ڵ���
{
    int i;
    printf("---------------���ű�----------------\n");
    printf("                   ������         ����      ����\n");
    for(i = 0;i<Tindex;i++)     printf("���%3d    %15s  %8d  %8d\n",i,Table[i].Name,Table[i].Kind,Table[i].line);
    printf("-------------------------------------\n");
    return;
}

void print_procedure_table()//��ӡ�ֳ��������ûʲô��
{
    int i;
    printf("---------------�ֳ����---------------\n");
    printf("            ������            PADDR           TADDR        ��������        ������       ������\n");
    for(i = 0;i<Pindex;i++)     printf("%15s  ���%3d %5d %5d %5d %5d\n",PTable[i].PName,PTable[i].PAddr,PTable[i].TAddr,PTable[i].parameter_num,PTable[i].const_num,PTable[i].var_num);
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

void getnextsym()//��ȡ��һ������
{
    strcpy(lastsym,currentsym);
    lasttype = currenttype;//�ѵ�ǰ���ʹ��ݸ���һ������
    strcpy(currentsym,RegisterList[Index].Value);//��ȡ��һ������
    currenttype = RegisterList[Index].Type;
    line = RegisterList[Index].Line;//��¼���������λ�ڵڼ��У����ڱ���
    Index++;
}

void sendbacksym()//�˻ص���
{
    Index--;//ָ��ǰ����
    strcpy(currentsym,lastsym);//Index-1
    currenttype = lasttype;//��һ�����͸�ֵ����ǰ����
    strcpy(lastsym,RegisterList[Index-2].Value);
    lasttype = RegisterList[Index-2].Type;
    line = RegisterList[Index-1].Line;
}
//������::=�ۣ�����˵�����ݣۣ�����˵������{���з���ֵ�������壾|���޷���ֵ�������壾}����������
void  procedure()//���ĳ���
{
    int i = 0;
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    getnextsym();
    while(currenttype==CONST)//����˵��
    {
        sendbacksym();//�˻�const
        const_declare_sentence();//��ʱ��currenttype�Ƿֺ�
        getnextsym();//�������������const��ִ��while
    }
    const_num = Tindex-1;//����const����������
    if(currenttype==INT || currenttype==CHAR)//���泣��˵�����ֶ�����һ��INT��CHAR���Զ��������˵������
    {
        sendbacksym();
        variable_declare_sentence();
    }
    global_bound = Tindex;//������ȫ�ֱ���

    //��������
    Level = 1;
    while(currenttype==INT || currenttype==CHAR || currenttype==VOID)
    {
        bound = Tindex;
        ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
        P_parameter_num = 0;//�����������
        P_const_number = 0;//�����еĳ�������
        P_var_number = 0;//�����еı�������
        if(currenttype==INT || currenttype==CHAR)
        {
            if(currenttype==INT)    returnflag = 1;
            if(currenttype==CHAR)   returnflag = 2;
            Kind = (currenttype==INT) ? INTFUNCTIONKIND : CHARFUNCTIONKIND;
            getnextsym();//Ӧ�ö����˺���������
            if(currenttype!=IDENT)
            {
                printf("line:%d û���ҵ���ʶ��\n",line);
            }
            else    strcpy(Name,currentsym);//int��char֮��һ���Ǳ�ʶ�������ڱ�ʶ�������ǲ��ǻ���[,���д�����
            //�жϺ��������ǲ����ظ�����
            for(i = 0;i <= Tindex && i <= global_bound;i++)
            {
                if(strcmp(Table[i].Name,currentsym)==0) printf("line:%d %s �ظ����壡\n",line,currentsym);
            }
            for(i = 0;i <= Pindex;i++)
            {
                if(strcmp(PTable[i].PName,currentsym)==0)   printf("line:%d %s �ظ����壡\n",line,currentsym);
            }
            strcpy(Arg1.ident_type_value,currentsym);
            Arg1.arg_type = 9;
            Emit(LABEL,Arg1,Arg2,Result);
            getnextsym();//Ӧ�ö���һ��������
            if(currenttype!=L_PARENTHESES)//����������
            {
                printf("line:%d �����������ȱ��������\n",line);
                while(currenttype!=INT && currenttype!=CHAR && currenttype!=VOID)    getnextsym();
                continue;
            }
            InsertTable();//��¼�����ű���һ����������
            parameter_table();//���в�������
            //����ͷ����������
            if(currenttype!=R_PARENTHESES)
            {
                printf("line:%d ȱ��������\n",line);
                SkipTo('\n');
            }
            printf("---%d---����һ�������������\n",line);
            getnextsym();
            if(currenttype!=L_BRACE)
            {
                printf("line:%d ȱ���������\n",line);
                SkipTo('\n');
            }
            else    compound_sentence();//���к����ڵĸ���������
            getnextsym();
            if(currenttype!=R_BRACE)
            {
                SkipTo('\n');
                printf("line:%d ������βȱ���Ҵ�����\n",line);
            }
            PTable[Pindex].parameter_num = P_parameter_num;
            PTable[Pindex].const_num = P_const_number;
            PTable[Pindex].var_num = P_var_number;
        }
        else if(currenttype==VOID)//�����void��������
        {
            returnflag = 0;
            Kind = VOIDFUNCTIONKIND;
            getnextsym();//�����˺�������
            for(i = 0;i <= Tindex && i <= global_bound;i++)
            {
                if(strcmp(Table[i].Name,currentsym)==0) printf("line:%d %s �ظ����壡\n",line,currentsym);
            }
            for(i = 0;i <= Pindex;i++)
            {
                if(strcmp(PTable[i].PName,currentsym)==0)   printf("line:%d %s �ظ����壡\n",line,currentsym);
            }
            if(currenttype==MAIN)//������������
            {
                break;
            }
            strcpy(Arg1.ident_type_value,currentsym);
            Arg1.arg_type = 9;
            Emit(LABEL,Arg1,Arg2,Result);
            if(currenttype!=IDENT)//���void���治�Ǳ�ʶ��
            {
                SkipTo('\n');
                printf("line:%d void����ȱ�ٱ�ʶ��\n",line);
            }
            else
            {
                strcpy(Name,currentsym);//��¼��������
                InsertTable();
                //print_symbol_table();
                getnextsym();
                if(currenttype==L_PARENTHESES)//�������ţ���ȷ
                {
                    parameter_table();//���в�������
                    //����ͷ����������
                    if(currenttype!=R_PARENTHESES)
                    {
                        SkipTo('\n');
                        printf("line:%d ����ͷ��ȱ��������\n",line);
                    }
                    printf("---%d---����һ��void�����������\n",line);
                    getnextsym();
                    if(currenttype!=L_BRACE)
                    {
                        SkipTo('\n');
                        printf("line:%d ��������ȱ���������\n",line);
                    }
                    compound_sentence();
                    getnextsym();
                    if(currenttype!=R_BRACE)
                    {
                        SkipTo('\n');
                        printf("line:%d ��������ȱ���Ҵ�����\n",line);
                    }
                    Emit(RETURNVOID,Arg1,Arg2,Result);
                    PTable[Pindex].parameter_num = P_parameter_num;
                    PTable[Pindex].const_num = P_const_number;
                    PTable[Pindex].var_num = P_var_number;
                }
                else
                {
                    SkipTo('\n');
                    printf("line:%d �������ֺ���ȱ������\n",line);
                }
            }
        }
        getnextsym();
        Pindex++;
    }
    //��ʼ����������
    P_parameter_num = 0;//�����������
    P_const_number = 0;//�����еĳ�������
    P_var_number = 0;//�����еı�������
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
            printf("---%d---����main��������\n",line);
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
                    printf("line:%d ������ȱ��������ƥ��\n",line);
                }
            }
            else
            {
                SkipTo('\n');
                printf("line:%d ������ȱ���������\n",line);
            }
        }
        else
        {
            SkipTo('\n');
            printf("line:%d main��С���Ų�ƥ��\n",line);
        }
    }
    else
    {
        SkipTo('\n');
        printf("line:%d main��û����С����\n",line);
    }
}


//int a = 10;
void const_declare_sentence()
{
    getnextsym();//����const
    getnextsym();
    if(currenttype==INT || currenttype==CHAR)
    {
        sendbacksym();
        const_define_sentence();
    }
    else
    {
        printf("line:%d �Ƿ��ַ�\n",line);
    }
    getnextsym();
    if(currenttype==SEMICOLON)
    {
        printf("---%d---����һ������˵�����\n",line);
    }
    else
    {
        SkipTo('\n');
        printf("line:%d ȱ�ٷֺ�\n",line);
    }
    return;
}
//a = 10,b = 12 û�зֺ�
void const_define_sentence()//��onst_declare_sentence������
{
    getnextsym();//����int��char
    Kind = (currenttype==INT) ? CONSTINTKIND : CONSTCHARKIND;//�洢��ǰ����
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    int temp;//���ڴ洢����ת���Ľ��
    getnextsym();//��ȡ��һ���ַ���Ӧ���Ǳ�ʶ��
    while(currenttype==IDENT)
    {
        strcpy(Arg1.ident_type_value,currentsym);
        strcpy(Name,currentsym);
        getnextsym();
        if(currenttype==ASSIGN)
        {
            getnextsym();
            if(currenttype==PLUS || currenttype==MINUS)//�Ⱥź���+��-
            {
                getnextsym();
                if(currenttype==INTEGER && Kind==CONSTINTKIND)
                {
                    temp = StringToInt();//�ѵ�ǰ�ַ�����ʽ������ת��Ϊ������ʽ
                    DigitValue = (lasttype==PLUS) ? (temp) : (0-temp);
                    Arg1.arg_type = 1;
                    Arg1.int_type_value = DigitValue;
                    Emit(GENCONST,Arg1,Arg2,Result);
                    InsertTable();//����ű��в���һ������
                    //print_symbol_table();
                    printf("---%d---����һ��int��ֵ���\n",line);
                }
                else
                {
                    SkipTo('\n');
                    printf("line:%d �Ⱥź�Ӧ����intֵ\n",line);
                }
            }
            else if(currenttype==INTEGER)//�Ⱥź�ֱ�����޷�������
            {
                if(Kind==CONSTINTKIND)
                {
                    temp = StringToInt();//string���͵�����ת��Ϊint����
                    DigitValue = temp;
                    Arg1.arg_type = 1;
                    Arg1.int_type_value = DigitValue;
                    Emit(GENCONST,Arg1,Arg2,Result);
                    InsertTable();
                    //print_symbol_table();
                    printf("---%d---����һ��int��ֵ���\n",line);
                }
                else
                {
                    SkipTo('\n');
                    printf("line:%d ���Ͳ���\n",line);
                }
            }
            else if(currenttype==CHARACTER)//���ַ�����
            {
                if(Kind==CONSTCHARKIND)
                {
                    CharValue = currentsym[0] - 0;
                    Arg1.arg_type = 2;
                    Arg1.char_type_value = CharValue;
                    Emit(GENCONST,Arg1,Arg2,Result);
                    InsertTable();//����һ���ַ�
                    //print_symbol_table();
                    printf("---%d---����һ��char��ֵ���\n",line);
                }
                else
                {
                    SkipTo('\n');
                    printf("line:%d ���Ͳ���\n",line);
                }
            }
            else
            {
                SkipTo('\n');
                printf("line:%d ��������������\n",line);
            }
            getnextsym();//������һ��a = 10��䣬Ӧ�ö������Ż�ֺ�
            if(currenttype==COMMA)  getnextsym();
            else if(currenttype==SEMICOLON)
            {
                sendbacksym();//�Ѷ����ķֺ��˻�
                break;
            }
            else
            {
                SkipTo('\n');
                printf("line:%d �Ƿ��ַ�\n",line);
            }
        }
        else
        {
            SkipTo('\n');
            printf("line:%d ������������У���ʶ������û�еȺ�\n",line);
        }
    }
    printf("---%d---����һ�������������\n",line);
    return;
}

//int a;
//int b,c,d;
//int sum(int a,int b){ }

void variable_declare_sentence()//int ��û��
{
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    getnextsym();//����int��char
    while(currenttype==INT || currenttype==CHAR)
    {
        if(currenttype==INT)    Kind = INTKIND;
        if(currenttype==CHAR)   Kind = CHARKIND;
        Recall = Index;//��������ж�ʧ����Ҫ���ص�λ��
        getnextsym();
        if(currenttype==IDENT)
        {
            strcpy(Name,currentsym);
            strcpy(Arg1.ident_type_value,currentsym);
            getnextsym();
            if(currenttype==COMMA || currenttype==L_BRACKET)//����','��'['˵�������Ǳ�������
            {
                Index = Recall-1;//�ص�Indexָ��INT��CHAR֮ǰ��λ��
                variable_define_sentence();//����֮��ֺŵ�ǰ�ַ��Ƿֺ�
                getnextsym();//Ӧ���Ƿֺ�
                if(currenttype!=SEMICOLON)
                {
                    printf("line:%d �����������Ӧ���ԷֺŽ�β\n",line);
                }
                printf("---%d---����һ�������������\n",line);
                getnextsym();
            }
            else if(currenttype==SEMICOLON)//���ֻ��һ���������������ֱ�Ӵ����ˣ�����Ҫ����variable_declare_sentence()
            {

                InsertTable();
                if(Kind==INTKIND)   Arg1.arg_type = 3;
                if(Kind==CHARKIND)   Arg1.arg_type = 4;
                Emit(GENVAR,Arg1,Arg2,Result);
                printf("---%d---����һ�������������\n",line);
                getnextsym();
            }
            else if(currenttype==L_PARENTHESES)//����С����˵�����Ǹ���������
            {
                Index = Recall-1;//����֮ǰ��¼��λ��
                getnextsym();//Ӧ�ö�������һ������������
                return;
            }
            else
            {
                SkipTo('\n');
                printf("line:%d ȱ�ٷֺ�\n",line);
            }
        }
        else
        {
            SkipTo('\n');
            printf("line:%d ���ͷ��ź���û�б�ʶ��\n",line);
        }
    }
}


//----------------------��������---------------------------
//���������壾::=�����ͱ�ʶ����(����ʶ����|����ʶ����'['���޷���������']'){,����ʶ����|����ʶ����'['���޷���������']' }
void variable_define_sentence()
{
    int arraylength = 0;//���ڼ�¼����ĳ���
    int Type;
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    //������ʱ��int��char��û��
    getnextsym();//int  char
    Type = (currenttype==INT) ? INTKIND : CHARKIND;
    do
    {
        Kind = Type;
        getnextsym();//ident
        strcpy(Name,currentsym);//int��char֮��һ���Ǳ�ʶ�������ڱ�ʶ�������ǲ��ǻ���[,���д�����
        strcpy(Arg1.ident_type_value,Name);//�ѱ���������
        getnextsym();
        if(currenttype==COMMA || currenttype==SEMICOLON)//����Ƕ��ţ���˵�������Ѿ���������һ������
        {
            InsertTable();//������ű�
            if(Kind==INTKIND)   Arg1.arg_type = 3;
            if(Kind==CHARKIND)   Arg1.arg_type = 4;
            Emit(GENVAR,Arg1,Arg2,Result);//����һ��������Ԫʽ

            sendbacksym();//�Ѷ��Ż�ֺ��˻�
        }
        else if(currenttype==L_BRACKET)//�������������ţ�˵�������鶨��
        {
            strcpy(Arg1.ident_type_value,Name);//�ѱ���������
            if(Kind==INTKIND)   Kind = INTARRAYKIND;//���֮ǰ�����ͣ������Ϊ��������
            else if(Kind==CHARKIND) Kind = CHARARRAYKIND;//���֮ǰ���ַ��ͣ������Ϊ�ַ�������
            getnextsym();//�ٶ�һ�����ţ�Ӧ�����޷�������
            if(currenttype==INTEGER)
            {
                arraylength = StringToInt();//��¼������ĳ���
                Arg2.arg_type = 1;//�õڶ���Arg����¼����
                Arg2.int_type_value = arraylength;
                getnextsym();//�ٶ�Ӧ������������
                if(currenttype==R_BRACKET)
                {
                    InsertTable();//������ű�
                    Emit(GENARRAY,Arg1,Arg2,Result);//����һ�������������Ԫʽ
                }
                else
                {
                    SkipTo('\n');
                    printf("line:%d ���鶨��ȱ��������\n",line);
                }
            }
            else
            {
                SkipTo('\n');
                printf("line:%d ���鳤��Ӧ�����޷�������\n",line);
            }
        }
        else
        {
            SkipTo('\n');
            printf("line:%d �Ƿ��ַ�\n",line);
        }
        getnextsym();
        if(currenttype==SEMICOLON)  break;
    }while(currenttype==COMMA);
    sendbacksym();//�ѷֺ�(��ȷ���)�������ַ��˻أ�������һ�㺯������
    return;
}

//int fun(int a,int b,int c){   }
void parameter_table()
{
    int i = 0;
    int paranum = 0;
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    int Para_Temp_Type;//��ʱ�洢��������
    getnextsym();
    if(currenttype==INT || currenttype==CHAR)//��һ������������
    {
        do
        {
            Para_Temp_Type = currenttype;//���͸���Para_Temp_Type
            Kind = (currenttype==INT) ? INTKIND : CHARKIND;
            if(currenttype==COMMA)  getnextsym();
            if(Para_Temp_Type==COMMA && (currenttype==INT || currenttype==CHAR))
            {
                Kind = (currenttype==INT) ? INTKIND : CHARKIND;
                Para_Temp_Type = currenttype;
            }
            head_define();//���غ���������Ƕ���
            strcpy(Arg1.ident_type_value,currentsym);
            Arg1.arg_type = 3;
            InsertTable();
            Emit(PARAMX,Arg1,Arg2,Result);//����һ���β���Ԫʽ
            paranum++;
            //print_symbol_table();
            P_parameter_num++;//��¼��������
            getnextsym();
        }while(currenttype==COMMA);//���Ƕ��ţ���Ӧ���������ţ�����Ӧ����������

        for(i = ICL_index;i>=ICL_index-paranum;i--)//��ǹ��м�������
        {
            if(ICL[i].OP==PARAMX)
            {
                ICL[i].Arg2.arg_type = 1;
                ICL[i].Arg2.int_type_value = paranum;
            }
        }
    }
}

void head_define()//ͷ������
{
    if(currenttype==INT || currenttype==CHAR)
    {
        if(currenttype==INT)    Kind = INTKIND;
        else if(currenttype==CHAR)   Kind = CHARKIND;
        getnextsym();//Ӧ�ö���һ����ʶ��
        if(currenttype!=IDENT)
        {
            printf("line:%d int��char����û���ҵ���ʶ��\n",line);
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
        printf("line:%d �Ƿ��ַ�\n",line);
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

//��������䣾   ::=  �ۣ�����˵�����ݣۣ�����˵�����ݣ�����У�
void compound_sentence()//��ǰָ��ĵ���������������////////////////������Ԫʽ/////////////////////
{
    getnextsym();
    if(currenttype==R_BRACE)//�������ţ�˵���������Ϊ��
    {
        sendbacksym();
        return;
    }
    int indexbegin = Tindex;//��¼��ǰ��λ��

    while(currenttype==CONST)//����˵��
    {
        sendbacksym();
        const_declare_sentence();//��ʱ��currenttype�Ƿֺ�
        getnextsym();//�������������const��ִ��while
    }
    P_const_number = Tindex-indexbegin;//��ֵ�Ǻ����еĳ�������
    indexbegin = Tindex;
    if(currenttype==INT || currenttype==CHAR)//���泣��˵�����ֶ�����һ��INT��CHAR���Զ��������˵������
    {
        sendbacksym();//�Ѷ�����int��char�˻�ȥ
        variable_declare_sentence();//����֮��ǰ�ַ��Ƿֺ�
    }
    P_var_number = Tindex-indexbegin;
    sendbacksym();
    sentence_set();//��ʼ����еķ���
    return;
}

void sentence_set()//����еĵ�һ�����ʻ�û�ж����������û��'{''}'
{
    getnextsym();
    if(currenttype==R_BRACE)//˵�������Ϊ�գ����ﺯ����β
    {
        sendbacksym();
        return;
    }
    while(currenttype==SEMICOLON || currenttype==IF || currenttype==WHILE || currenttype==L_BRACE || currenttype==IDENT || currenttype==SCANF || currenttype==PRINTF || currenttype==SWITCH || currenttype==RETURN)
    {
        sendbacksym();//�˻ص���
        sentence();//˵������һ�����
        getnextsym();//�����������ж��ǲ��ǻ������
    }
    sendbacksym();
    printf("---%d---����һ�������\n",line);
    return;
}

//����䣾::=��������䣾����ѭ����䣾| '{'������У�'}'�����з���ֵ����������䣾; | ���޷���ֵ����������䣾;������ֵ��䣾;��������䣾;����д��䣾;�����գ�;|�������䣾����������䣾;

void sentence()//�ֺ���sentence������
{
    getnextsym();
    if(currenttype==IF)
    {
        sendbacksym();//�˻ض�����if
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
            printf("line:%d ȱ���Ҵ�����ƥ��\n",line);
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
        getnextsym();//�����ֺ�
        if(currenttype==SEMICOLON)  return;
    }
    else if(currenttype==SWITCH)
    {
        sendbacksym();//�˻�switch
        switch_sentence();
    }
    else if(currenttype==IDENT)//�������û�ֵ���
    {
        look_up_type();
        int OP;
        ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
        strcpy(Arg1.ident_type_value,currentsym);
        getnextsym();
        if(currenttype==ASSIGN)//��ֵ���
        {
            OP = ASSI;
            Arg1.arg_type = 3;
            Arg2 = expression();
            Arg1 = Emit(OP,Arg1,Arg2,Result);
            getnextsym();
            if(currenttype==SEMICOLON)
            {
                printf("---%d---����һ����ֵ���\n",line);
            }
            else
            {
                SkipTo('\n');
                printf("line:%d û�зֺŽ�β\n",line);
            }
        }
        else if(currenttype==L_BRACKET)//�������ţ�˵��������Ԫ��
        {
            Arg2 = expression();//��Arg2��¼����
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
                        printf("---%d---����һ��������Ԫ�ظ�ֵ�����\n",line);
                    }
                    else
                    {
                        SkipTo('\n');
                        printf("line:%d ȱ�ٷֺ�\n",line);
                    }
                }
                else
                {
                    SkipTo('\n');
                    printf("line:%d û���ҵ���ֵ���ţ��﷨����\n",line);
                }
            }
            else
            {
                SkipTo('\n');
                printf("line:%d �����Ų�ƥ��\n",line);
            }
        }
        else if(currenttype==L_PARENTHESES)//������С���ţ�˵���Ǻ����������
        {
            parameter_sentence();//���в�������
            getnextsym();
            if(currenttype==R_PARENTHESES)//������Ҳƥ��
            {
                getnextsym();
                if(currenttype==SEMICOLON)
                {
                    OP = CALL;
                    Arg1 = Emit(OP,Arg1,Arg2,Result);
                    printf("---%d---����һ���޷���ֵ�ĺ����������\n",line);
                    return;
                }
                else
                {
                    SkipTo('\n');
                    printf("line:%d ȱ�ٷֺ�\n",line);
                }
            }
            else
            {
                SkipTo('\n');
                printf("line:%d ���Ų�ƥ��\n",line);
            }
        }
        else
        {
            SkipTo('\n');
            printf("line:%d �Ƿ��ַ�\n",line);
        }
    }
    else if(currenttype==SEMICOLON)//�����
    {
        printf("---%d---����һ�������\n",line);
        return;
    }
    else if(currenttype==R_BRACE)//�Ҵ�����˵������н���
    {
        printf("---%d---����з�������\n",line);
        sendbacksym();
        return;
    }
    else
    {
        SkipTo('\n');
        printf("line:%d �Ƿ��ַ�\n",line);
    }
}

void parameter_sentence()//��¼�������ù����еĲ�������
{
    int sum = 0;
    int i = 0;
    ArgNode arrA1[100],arrA2[100];//�����100������
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
        printf("---%d---ֵ������������\n",line);
        for(i = 0;i < arrind;i++)
        {
            Emit(PARAM,arrA1[i],arrA2[i],Result);
        }
        sendbacksym();//���������˻��ˣ�������з���ֵ�ĺ������������Ҫ����Ӧ���޸�
        return;
    }
    else
    {
        SkipTo('\n');
        printf("line:%d ���Ų�ƥ��\n",line);
    }
    return;
}

void if_sentence()
{
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    getnextsym();//����if

    getnextsym();
    if(currenttype!=L_PARENTHESES)
    {
        SkipTo('\n');
        printf("line:%d if����ȱ��������\n",line);
    }
    condition_sentence();
    getnextsym();
    if(currenttype!=R_PARENTHESES)
    {
        SkipTo('\n');
        printf("line:%d ����������û��������\n",line);
    }
    else
    {
        printf("---%d---����һ��if���\n",line);//��ǰ������������
    }
    sentence();//if������һ�����
    Fill(Fillindex[FI--],ICL_index+2);//���֮ǰif��������������֮�������ָ��λ��
    Emit(JMP,Arg1,Arg2,Result);//ִ����if����������Ҫ����else������
    Fillindex[++FI] = ICL_index;//�ȴ�����
    getnextsym();//������������ȡ��һ������
    if(currenttype==ELSE)
    {
        sentence();//else����Ҳ��һ�����
        Fill(Fillindex[FI--],ICL_index+1);
    }
    else//û��else������ת���������ת��ԭ��
    {
        //Fill(Fillindex[FI--],Fillindex[FI+1]);
        FI--;
        sendbacksym();//�������else���˻�����ĵ���
    }
    printf("---%d---��������������\n",line);
}
/////////////////////////////////////////////////////////////////////////////////////////
//��������::=�����ʽ������ϵ������������ʽ���������ʽ�� //���ʽΪ0����Ϊ�٣�����Ϊ��
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
        Emit(OP,Arg1,Arg2,Result);//����������ʱ��ת
        Fillindex[++FI] = ICL_index;//��¼��һ����Ҫ�����ָ��

        printf("---%d---����һ���������\n",line);
    }
    else if(currenttype==R_PARENTHESES)//���ֻ��һ�����ʽ
    {
        Arg2.arg_type = 1;//���ֻ��һ�����ʽ�����Ǻ�0��ȣ���������ж�ʧ�ܣ�����ת
        Arg2.int_type_value = 0;
        Arg1 = Emit(CMP,Arg1,Arg2,Result);
        Emit(JE,Arg1,Arg2,Result);
        Fillindex[++FI] = ICL_index;//��¼����Ҫ�����ָ��
        sendbacksym();//�Ѷ������������˻�
    }
    else
    {
        SkipTo('\n');
        printf("line:%d �Ƿ��ַ�\n",line);
    }
    return;
}
//��ѭ����䣾::=while'('��������')'����䣾
void while_sentence()
{
    char str[10];
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    getnextsym();//����while
    getnextsym();//Ӧ�ö���������

    Arg1.arg_type = 3;
    itoa(ICL_index+2,str,10);
    Arg1.ident_type_value[0] = 'A';
    Arg1.ident_type_value[1] = '\0';
    strcat(Arg1.ident_type_value,str);//����һ��label
    Emit(LABEL,Arg1,Arg2,Result);
    Fillindex[++FI] = ICL_index+1;//��������λ��
    if(currenttype==L_PARENTHESES)
    {
        condition_sentence();
        getnextsym();
        if(currenttype==R_PARENTHESES)//��������ƥ��
        {
            sentence();
            Fill(Fillindex[FI--],ICL_index+2);//����ж�ʧ�ܣ���ת����һ��ָ���condition_sentence��������תָ��
            Emit(JMP,Arg1,Arg2,Result);
            Fill(ICL_index,Fillindex[FI--]);//����while��ʼ��
            printf("---%d---����һ��while���\n",line);
        }
        else
        {
            SkipTo('\n');
            printf("line:%d ��������Ҳ����Ų�ƥ��\n",line);
        }
    }
    else
    {
        SkipTo('\n');
        printf("line:%d while����Ӧ����������\n",line);
    }
}

void scanf_sentence()
{
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    getnextsym();//������scanf
    getnextsym();//����������
    if(currenttype==L_PARENTHESES)//����������ţ������﷨�淶
    {
        getnextsym();
        if(currenttype==R_PARENTHESES)
        {
            SkipTo('\n');
            printf("line:%d û�ж�������\n",line);
            return;
        }
        if(currenttype==INT || currenttype==CHAR)
        {
            SkipTo('\n');
            printf("line:%d �������û�б���\n",line);
        }
        while(currenttype==IDENT)//����Ӧ������ű�
        {
            if(look_up_type()==-1)  return;//�����Ƿ���
            if(look_up_type()==INTKIND)    Arg1.arg_type = 3;
            if(look_up_type()==CHARKIND)   Arg1.arg_type = 4;
            strcpy(Arg1.ident_type_value,currentsym);
            if(Arg1.arg_type==3)    Emit(SCANINT,Arg1,Arg2,Result);
            else if(Arg1.arg_type==4)   Emit(SCANCHAR,Arg1,Arg2,Result);
            printf("---%d---����һ����ʶ��\n",line);
            getnextsym();//�ٶ�һ�����ʣ������Ƕ��ţ�������������
            if(currenttype==COMMA)
            {
                getnextsym();
            }
            else if(currenttype==R_PARENTHESES) break;//�����������Ŷ������
            else
            {
                SkipTo('\n');
                printf("line:%d �Ƿ��ַ�\n",line);
            }
        }
        getnextsym();
        if(currenttype==SEMICOLON)
        {
            printf("---%d---����һ�������\n",line);
            sendbacksym();
            return;
        }
        else
        {
            printf("line:%d ������βû�зֺ�\n",line);
            sendbacksym();
            return;
        }
    }
    else
    {
        SkipTo('\n');
        printf("line:%d ���Ų�ƥ��\n",line);
    }
}

void printf_sentence()
{
    getnextsym();//����printf
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    getnextsym();
    if(currenttype==L_PARENTHESES)
    {
        getnextsym();
        if(currenttype==STRING)//���һ���ַ���
        {
            Arg1.arg_type = 8;//��ʾ����ַ���
            strcpy(Arg1.string_type_value,currentsym);//�����ַ�������
            Emit(PRINTSTR,Arg1,Arg2,Result);//���һ���ַ���
            getnextsym();
            if(currenttype==COMMA)
            {
                Arg1 = expression();//������ʽ
                getnextsym();//�ٶ�һ�����ʣ�Ӧ����������
                if(currenttype==R_PARENTHESES)
                {
                    if(Arg1.arg_type==3 || Arg1.arg_type==7)//���int�ͱ���
                    {
                        Emit(PRINTINTIDENT,Arg1,Arg2,Result);
                    }
                    else if(Arg1.arg_type==4)
                    {
                        Emit(PRINTCHARIDENT,Arg1,Arg2,Result);//����ַ��ͱ���
                    }
                    else if(Arg1.arg_type==1)
                    {
                        Emit(PRINTINT,Arg1,Arg2,Result);//�������
                    }
                    else if(Arg1.arg_type==2)
                    {
                        Emit(PRINTCHAR,Arg1,Arg2,Result);//����ַ�
                    }
                    else if(Arg1.arg_type==5)
                    {
                        Emit(PRINTINTIDENT,Arg1,Arg2,Result);//������ֱ���
                    }
                    else if(Arg1.arg_type==6)
                    {
                        Emit(PRINTCHARIDENT,Arg1,Arg2,Result);//����ַ�����
                    }
                    printf("---%d---����һ��������,������ַ����ͱ��ʽ\n",line);
                }
                else
                {
                    SkipTo('\n');
                    printf("line:%d ���Ų�ƥ��\n",line);
                }
            }
            else if(currenttype==R_PARENTHESES)
            {
                printf("---%d---����һ�������䣬�����һ���ַ���\n",line);
            }
            else
            {
                SkipTo('\n');
                printf("line:%d �Ƿ��ַ�\n",line);
            }
        }
        else
        {
            sendbacksym();
            Arg1 = expression();
            if(Arg1.arg_type==3 || Arg1.arg_type==7 || Arg1.arg_type==5)//���int�ͱ���
            {
                Emit(PRINTINTIDENT,Arg1,Arg2,Result);
            }
            else if(Arg1.arg_type==4 || Arg1.arg_type==6)
            {
                Emit(PRINTCHARIDENT,Arg1,Arg2,Result);//����ַ��ͱ���
            }
            else if(Arg1.arg_type==1)
            {
                Emit(PRINTINT,Arg1,Arg2,Result);//�������
            }
            else if(Arg1.arg_type==2)
            {
                Emit(PRINTCHAR,Arg1,Arg2,Result);//����ַ�
            }
            getnextsym();
            if(currenttype==R_PARENTHESES)
            {
                printf("---%d---����һ�������䣬����˱��ʽ\n",line);
            }
            else
            {
                SkipTo('\n');
                printf("line:%d �Ƿ��ַ�\n",line);
            }
        }
        getnextsym();
        if(currenttype==SEMICOLON)
        {
            sendbacksym();//�˻طֺ�
            printf("---%d---�������������\n",line);
        }
        else
        {
            sendbacksym();
            printf("line:%d ����βû�зֺ�\n",line);
        }
        return;
    }
    else
    {
        SkipTo('\n');
        printf("line:%d printf��ʶ��û������\n",line);
    }
}

void return_sentence()
{
    getnextsym();//����return
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
            printf("line:%d ������󣬺���û�з���ֵ\n",line);
            return;
        }
        Emit(RETURNVOID,Arg1,Arg2,Result);
        sendbacksym();
        printf("---%d---����һ���޷���ֵ�ķ������\n",line);
        return;
    }
    if(currenttype==L_PARENTHESES)//���򣬽��б��ʽ����
    {
        if(returnflag==0)
        {
            printf("line:%d void���������з���ֵ\n",line);
            return;
        }
        Arg1 = expression();
        getnextsym();
        if(currenttype==R_PARENTHESES)
        {
            getnextsym();
            if(currenttype==SEMICOLON)
            {
                if(returnflag==1 && (Arg1.arg_type==2||Arg1.arg_type==4||Arg1.arg_type==6)) printf("line:%d �������Ͳ���\n",line);
                else if(returnflag==2 && (Arg1.arg_type==1||Arg1.arg_type==3||Arg1.arg_type==5)) printf("line:%d �������Ͳ���\n",line);
                else if(returnflag==0)  printf("line:%d void���������з���ֵ\n",line);
                else
                {
                    Emit(OP,Arg1,Arg2,Result);
                    printf("---%d---����һ���з���ֵ�ķ������\n",line);
                    sendbacksym();//�˻طֺ�
                }
                return;
            }
            else
            {
                SkipTo('\n');
                printf("line:%d �Ƿ��ַ�\n",line);
            }
        }
        else
        {
            SkipTo('\n');
            printf("line:%d ����������Ų�ƥ��\n",line);
        }
    }
    else
    {
        SkipTo('\n');
        printf("line:%d return��Ӧ����������\n",line);
    }
}

void switch_sentence()
{
    getnextsym();//����switch
    ArgNode Arg1 = InitArgNode();
    getnextsym();
    if(currenttype==L_PARENTHESES)
    {
        Arg1 = expression();//switch���ж�������һ�����ʽ
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
                printf("line:%d �����Ų�ƥ��\n",line);
            }
        }
        else
        {
            SkipTo('\n');
            printf("line:%d ���Ų�ƥ��\n",line);
        }
    }
    else
    {
        SkipTo('\n');
        printf("line:%d ȱ��С����\n",line);
    }
}

void situation_table(ArgNode A1)//�������ǰ������
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
    if(currenttype==R_BRACE)//������Ҵ����ţ���ƥ��
    {
        for(i = 0;i < casenum;i++)
        {
            Fill(Fillswitch[i],ICL_index+1);
        }
        casenum = 0;
        printf("---%d---����һ��������\n",line);
    }
    return;
}

void situation_sentence(ArgNode A1)//��ʱcase��û�ж�
{
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    getnextsym();//Ӧ�ö�����case
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
            Arg1 = Emit(CMP,A1,Arg1,Result);//�Ƚ�switch���ֵ��case���ֵ
            Arg1 = Emit(JNE,Arg1,Arg2,Result);//�������ȣ���ת
            Fillindex[++FI] = ICL_index;
            getnextsym();
            if(currenttype==COLON)
            {
                sentence_set();
                Arg1 = Emit(JMP,Arg1,Arg2,Result);
                Fillswitch[casenum] = ICL_index;//��casenum��case�õ�ִ�У���Ҫ��ת��switch����
                Fill(Fillindex[FI--],ICL_index+1);//���case�������ȣ�����ת����һ��case
                printf("---%d---����һ��CASE���\n",line);
            }
            else
            {
                SkipTo('\n');
                printf("line:%d ȱ��ð��\n",line);
            }
        }
        else
        {
            SkipTo('\n');
            printf("line:%d ����Ӧ���ǳ���\n",line);
        }
    }
    else
    {
        printf("line:%d ȱ�١�case��",line);
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
            printf("---%d---����һ��default���\n",line);
        }
        else    printf("line:%d ȱ��ð��\n",line);
    }
    else
    {
        printf("line:%d ȱ�١�default��",line);
        SkipTo('}');
    }
    return;
}

ArgNode expression()
{
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    OpType OP = UNKNOWN;//unknown����ʡ��

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
        sendbacksym();//������ǼӼ��ţ��˻�
        Arg1 = item();//��ǰ���Ų��ǳ˳�
        getnextsym();//�����Ӽ��Ż������
    }
    while(currenttype==PLUS || currenttype==MINUS)//���Զ���������Ӽ�
    {
        OP = currenttype==PLUS ? ADD : SUB;
        Arg2 = item();

        Arg1 = Emit(OP,Arg1,Arg2,Result);
        getnextsym();
    }
    sendbacksym();
    OP = UNKNOWN;
    Arg1 = Emit(OP,Arg1,Arg2,Result);
    printf("---%d---����һ�����ʽ\n",line);
    return Arg1;
}

ArgNode item()
{
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    Arg1 = factor();
    getnextsym();//��ȡ���Ӻ���ķ���
    while(currenttype==MULTIPLY || currenttype==DIVIDE)
    {
        if(currenttype==MULTIPLY)
        {
            Arg2 = factor();//�ǳ˷�����
            Arg1 = Emit(MUL,Arg1,Arg2,Result);
            getnextsym();
        }
        else
        {
            Arg2 = factor();//�ǳ�������
            Arg1 = Emit(DIV,Arg1,Arg2,Result);
            getnextsym();
        }
    }
    sendbacksym();
    printf("---%d---����һ����\n",line);//��������һ�����ǳ˳��ķ��ţ�������ķ���

    return Arg1;
}

//�����ӣ�::=����ʶ����������ʶ����'['�����ʽ��']'����������|���ַ��������з���ֵ����������䣾|'('�����ʽ��')'
ArgNode factor()
{
    ArgNode Arg1 = InitArgNode(),Arg2 = InitArgNode(),Result = InitArgNode();
    getnextsym();
    char PM;
    int Type;
    if(currenttype==IDENT)//������ʶ����˵���Ǳ�ʶ��������Ԫ�ػ��з���ֵ�ĺ���
    {
        if(look_up_type()==-1)  return Arg1;;
        Type = look_up_type();//����Ǻ��������¼����������
        strcpy(Arg1.ident_type_value,currentsym);//��¼��ʶ������
        if(Find_const()==1)//����ǳ������滻Ϊ��Ӧ������ֵ
        {
            Arg1.int_type_value = Rep_int_const();
            Arg1.arg_type = 1;
        }
        else if(Find_const()==2)//����ǳ������滻Ϊ��Ӧ���ַ�ֵ
        {
             Arg1.char_type_value = Rep_char_const();
             Arg1.arg_type = 2;
        }
        else    Arg1.arg_type = (look_up_type()==INTKIND || look_up_type()==CONSTINTKIND) ? 3 : 4;
        getnextsym();

        if(currenttype==L_BRACKET)//����Ԫ��
        {
            Arg2 = expression();//"���ʽ"
            getnextsym();//�ٶ�һ������Ӧ������������
            if(currenttype==R_BRACKET)//��������˵������Ԫ�ض�������
            {
                printf("---%d---����һ������Ԫ��\n",line);
                Arg1 = Emit(ARRAY,Arg1,Arg2,Result);
            }
            else
            {
                printf("line:%d ���Ų�ƥ��\n",line);
                SkipTo('\n');
            }
        }
        else if(currenttype==L_PARENTHESES)//����������ţ���������//////////////////////
        {
            sendbacksym();
            sendbacksym();//������������
            return_function();//�з���ֵ�ĺ�������
            Arg1 = Emit(CALL,Arg1,Arg2,Result);
            if(Type==INTFUNCTIONKIND)   Arg1 = Emit(GETINTRVALUE,Arg1,Arg2,Result);
            if(Type==CHARFUNCTIONKIND)  Arg1 = Emit(GETCHARRVALUE,Arg1,Arg2,Result);
        }
        else//ֻ��һ�������ı�ʶ��
        {
            sendbacksym();//�˻�����ĵ���
            printf("---%d---����һ����ʶ��\n",line);
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
        printf("---%d---����һ���������ַ�\n",line);
    }
    else if(currenttype==L_PARENTHESES)
    {
        Arg1 = expression();
        getnextsym();
        if(currenttype!=R_PARENTHESES)
        {
            printf("line:%d ���Ų�ƥ��\n",line);
            SkipTo('\n');
        }
    }
    else
    {
        printf("line:%d �Ƿ��ַ�\n",line);
        SkipTo('\n');
    }
    return Arg1;
}

void return_function()//�з���ֵ�ĺ���������䴦��
{
    getnextsym();//Ӧ�ö����˺�����
    if(currenttype==IDENT)
    {
        getnextsym();

        if(currenttype==L_PARENTHESES)
        {
            parameter_sentence();
            getnextsym();//��ǰ����Ӧ����������
            if(currenttype==R_PARENTHESES)
            {
                printf("---%d---����һ���з���ֵ�ĺ����������\n",line);
            }
            else    printf("line:%d ���Ų�ƥ��\n",line);
        }
        else    printf("line:%d û��������\n",line);
    }
    else    printf("line:%d �Ƿ��ַ�\n",line);
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
