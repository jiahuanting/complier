#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"

extern void error(int errornum);

Reg RegisterList[10000];//记录识别出来的单词
int ListIndex = 0;//编号

char SourceFile[MAXFILE];//一次把文件内容都读进来
char ch;//存储当前字符
int symbol;//存储当前字符的类型
int index = 0;//SourceFile数组的指针，表示将要读取字符的位置
char tokenbuf[MAXTOKENBUF];//当前读取字符的缓冲区
int tokenbufindex = -1;//缓冲区指针
int line = 1;//当前读到第几行
int number;//存储数字
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

void ReadFile(char filename[])//读文件
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

void clearToken()//清空字符缓冲区
{
    int i = 0;
    for(i = 0;i<MAXTOKENBUF;i++)    tokenbuf[i] = '\0';
    tokenbufindex = -1;
}

void catToken()//把符合条件的字符传进缓冲区
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


int isSpace()//是空格则返回1
{
    if(ch==32)  return 1;
    else    return 0;
}

int isNewline()//是回车键或者换行符则返回1
{
    if(ch==10||ch==13)  return 1;
    else    return 0;
}

int isTab()//是Tab键返回1
{
    if(ch==9)   return 1;
    else    return 0;
}

int isLetter()//是字母返回1
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

int isDigit()//是数字则返回1
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

int isChar()//是字符则返回1
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

void getsym()//词法分析
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
                    if(isDivi())    return;//注释处理完毕
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
            printf("语法错误\n");//！号后面没有=号
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
    RegisterList[ListIndex].Id = ListIndex;//编号
    RegisterList[ListIndex].Type = symbol;//类型
    RegisterList[ListIndex].Line = line;//行数
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

