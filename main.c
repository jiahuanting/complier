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
    printf("请输入文件路径:\n");
    scanf("%s",Filename);
    ReadFile(Filename);
    //ReadFile("14061185_test.txt");
    LexicalAnalysis();                   //词法分析
    Parse();
    return 0;
}
