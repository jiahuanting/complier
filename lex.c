#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"

extern void error(int errornum);

Reg RegisterList[10000];//��¼ʶ������ĵ���
int ListIndex = 0;//���

char SourceFile[MAXFILE];//һ�ΰ��ļ����ݶ�������
char ch;//�洢��ǰ�ַ�
int symbol;//�洢��ǰ�ַ�������
int index = 0;//SourceFile�����ָ�룬��ʾ��Ҫ��ȡ�ַ���λ��
char tokenbuf[MAXTOKENBUF];//��ǰ��ȡ�ַ��Ļ�����
int tokenbufindex = -1;//������ָ��
int line = 1;//��ǰ�����ڼ���
int number;//�洢����
char ReserverList[RESERVELIST][30] =
    {
        " ","BREAK","CASE","CHAR","CONST","DEFAULT","IF","ELSE","INT","RETURN","SWITCH","VOID","WHILE",
        "PLUS","MINUS","MULTIPLY","DIVIDE","L_PARENTHESES","R_PARENTHESES","L_BRACKET","R_BRACKET",
        "L_BRACE","R_BRACE","SEMICOLON","COMMA","SINGLEQUOTES","DOUBLEQUOTES","COLON","EQUAL","LARGER",
        "SMALLER","NOT_EQUAL","NOT_LARGER","NOT_SMALLER","ASSIGN",
        "IDENT","INTEGER","CHARACTER","STRING",
        "INT_CONST","CHAR_CONST","INT_VAR","CHAR_VAR","FUNCTION",
        "MAIN","EOFSY","PRINTF","SCANF"
    };

void ReadFile(char filename[])//���ļ�
{
    int i = 0;
    FILE *in;
    in = fopen(filename,"r");
    while(fscanf(in,"%c",&SourceFile[i])!=EOF)  i++;
    fclose(in);
}

void getch()
{
    if(index<=strlen(SourceFile))
    {
        ch = SourceFile[index];
        index++;
    }
    else return;
}

void retract()
{
    index--;
}

void clearToken()//����ַ�������
{
    int i = 0;
    for(i = 0;i<MAXTOKENBUF;i++)    tokenbuf[i] = '\0';
    tokenbufindex = -1;
}

void catToken()//�ѷ����������ַ�����������
{
    if(tokenbufindex < MAXTOKENBUF-1)
    {
        tokenbuf[++tokenbufindex] = ch;
    }
    else
    {
        error(0);
    }
}


int isSpace()//�ǿո��򷵻�1
{
    if(ch==32)  return 1;
    else    return 0;
}

int isNewline()//�ǻس������߻��з��򷵻�1
{
    if(ch==10||ch==13)  return 1;
    else    return 0;
}

int isTab()//��Tab������1
{
    if(ch==9)   return 1;
    else    return 0;
}

int isLetter()//����ĸ����1
{
    if( (ch>='A'&&ch<='Z') || (ch>='a'&&ch<='z') || ch=='_')
    {
        /*if(ch>='A'&&ch<='Z')
        {
            ch = 'a'+ch-'A';
        }*/
        return 1;
    }
    else    return 0;
}

int isDigit()//�������򷵻�1
{
    if(ch>='0'&&ch<='9')    return 1;
    else    return 0;
}

int isDivi()
{
    if(ch=='/') return 1;
    else    return 0;
}

int isStar()
{
    if(ch=='*') return 1;
    else    return 0;
}

int isChar()//���ַ��򷵻�1
{
    if((ch>=35&&ch<=126)||(ch==' ')||(ch=='!')) return 1;
    else    return 0;
}

int transNum()
{
    int i,number = 0;
    for(i = 0;i <= tokenbufindex;i++)
    {
        number = number*10+(tokenbuf[i]-'0');
    }
    return number;
}

int reserver()
{
    int res;
    if(strcmp(tokenbuf,"break")==0)    res = BREAK;
    else if(strcmp(tokenbuf,"case")==0)    res = CASE;
    else if(strcmp(tokenbuf,"char")==0)    res = CHAR;
    else if(strcmp(tokenbuf,"const")==0)    res = CONST;
    else if(strcmp(tokenbuf,"default")==0)    res = DEFAULT;
    else if(strcmp(tokenbuf,"if")==0)    res = IF;
    else if(strcmp(tokenbuf,"else")==0)    res = ELSE;
    else if(strcmp(tokenbuf,"int")==0)    res = INT;
    else if(strcmp(tokenbuf,"return")==0)    res = RETURN;
    else if(strcmp(tokenbuf,"switch")==0)    res = SWITCH;
    else if(strcmp(tokenbuf,"void")==0)    res = VOID;
    else if(strcmp(tokenbuf,"while")==0)    res = WHILE;
    else if(strcmp(tokenbuf,"printf")==0)    res = PRINTF;
    else if(strcmp(tokenbuf,"scanf")==0)    res = SCANF;
    else if(strcmp(tokenbuf,"main")==0)    res = MAIN;
    else res = 0;
    return res;
}

void getsym()//�ʷ�����
{
    int resultValue;
    clearToken();
    getch();
    if(isLetter())
    {
        while(isLetter()||isDigit())
        {
            catToken();
            getch();
        }
        retract();
        resultValue = reserver();

        if(resultValue==0)  symbol = IDENT;
        else    symbol = resultValue;
    }
    else if(isDigit())
    {
        while(isDigit())
        {
            catToken();
            getch();
        }
        if(isLetter())  error(1);
        else if(tokenbuf[0]=='0'&&tokenbufindex>=2) error(2);
        retract();
        number = transNum();
        symbol = INTEGER;
    }
    else if(isDivi())
    {
        getch();
        if(ch==EOF)
        {
            symbol = EOFSY;
            return;
        }
        if(isStar())
        {
            do
            {
                do
                {
                    getch();
                    if(ch==EOF)
                    {
                        symbol = EOFSY;
                        return;
                    }
                }while(!isStar());
                do
                {
                    getch();
                    if(ch==EOF)
                    {
                        symbol = EOFSY;
                        return;
                    }
                    if(isDivi())    return;//ע�ʹ������
                }while(isStar());
            }while(!isStar());
        }
        retract();
        symbol = DIVIDE;
    }

    else if(ch=='+')    {catToken();symbol = PLUS;}
    else if(ch=='-')    {catToken();symbol = MINUS;}
    else if(ch=='*')    {catToken();symbol = MULTIPLY;}
    else if(ch=='(')    {catToken();symbol = L_PARENTHESES;}
    else if(ch==')')    {catToken();symbol = R_PARENTHESES;}
    else if(ch=='[')    {catToken();symbol = L_BRACKET;}
    else if(ch==']')    {catToken();symbol = R_BRACKET;}
    else if(ch=='{')    {catToken();symbol = L_BRACE;}
    else if(ch=='}')    {catToken();symbol = R_BRACE;}
    else if(ch==';')    {catToken();symbol = SEMICOLON;}
    else if(ch==',')    {catToken();symbol = COMMA;}
    else if(ch==':')    {catToken();symbol = COLON;}
    else if(ch==' '||ch=='\t')    {symbol = 0;return;}
	else if(ch=='\n')
    {
        symbol = 0;
        line++;
        return;
    }
    else if(ch=='<')
    {
        catToken();
        getch();
        if(ch=='=')
        {
            catToken();
            symbol = NOT_LARGER;
        }
        else
        {
            retract();
            symbol = SMALLER;
        }
    }
    else if(ch=='>')
    {
        catToken();
        getch();
        if(ch=='=')
        {
            catToken();
            symbol = NOT_SMALLER;
        }
        else
        {
            retract();
            symbol = LARGER;
        }
    }

    else if(ch=='=')
    {
        catToken();
        getch();
        if(ch=='=')
        {
            catToken();
            symbol = EQUAL;
        }
        else
        {
            retract();
            symbol = ASSIGN;
        }
    }
    else if(ch=='!')
    {
        catToken();
        getch();
        if(ch=='=')
        {
            catToken();
            symbol = NOT_EQUAL;
        }
        else
        {
            printf("�﷨����\n");//���ź���û��=��
        }
    }
    else if(ch=='\'')
    {
        getch();
        if(isChar())    catToken();
        else    error(4);
        getch();
        if(ch=='\'')    symbol = CHARACTER;
        else    error(5);
    }

    else if(ch=='\"')
    {
        getch();
        while(ch!='\"')
        {
            if(isChar())
            {
                catToken();
                getch();
            }
            else
            {
                error(6);
                break;
            }
        }
        symbol = STRING;
    }

    else return;
    return;
}


void printlist()
{
    int i = 0;
    for(i = 0;i < ListIndex;i++)
    {
        printf("%d  %-15s  %s\n",RegisterList[i].Type,RegisterList[i].Name,RegisterList[i].Value);
    }
    //printf("%-15s  %s\n",ReserverList[symbol],tokenbuf);
}


void InsertList()
{
    RegisterList[ListIndex].Id = ListIndex;//���
    RegisterList[ListIndex].Type = symbol;//����
    RegisterList[ListIndex].Line = line;//����
    strcpy(RegisterList[ListIndex].Name,ReserverList[symbol]);
    strcpy(RegisterList[ListIndex].Value,tokenbuf);
    ListIndex++;
}


void LexicalAnalysis()
{
    while(index<=strlen(SourceFile))
    {
        getsym();
        if(symbol==0)   continue;
        else InsertList();
    }
    //printlist();
}

