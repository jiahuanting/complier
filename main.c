#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"

extern void getsym();
extern void ReadFile();
extern char SourceFile[];
extern void LexicalAnalysis();
extern void Parse();

int main()
{
    char Filename[128];
    printf("�������ļ�·��:\n");
    scanf("%s",Filename);
    ReadFile(Filename);
    //ReadFile("14061185_test.txt");
    LexicalAnalysis();                   //�ʷ�����
    Parse();
    return 0;
}
