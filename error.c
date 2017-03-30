#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"

//extern int index;
extern int line;
extern char currentsym[];
void error(int errornum)
{
    char ErrorArray[ERRORNUM][ERRORLEN] =
    {
        "字符串长度超过规定值",           //0
        "非法标识符",                     //1
        "多位数字不能以0开头",            //2
        "源文件错误",                     //3
        "单引号下不是字符类型",           //4
        "字符类型长度不为1",              //5
        "非法字符",                       //6
        "int或char后面没有找到标识符",    //7
        "标识符后面找到等号",             //8
        "类型不符",                       //9
        "赋值符号后只能是+、-和字母数字", //10
        "定义语句应该以分号结尾",         //11
        "变量定义出现非法字符",           //12
        "int和char后面没有找到标识符",    //13
        "中括号不匹配",                   //14
        "中括号后只能出现整数",           //15
        "语句结束没有分号",               //16
        "缺少小括号",                     //17
        "缺少右括号",                     //18
        "缺少左大括号",                   //19
        "缺少右大括号"                    //19
    };
    if(errornum==100)
    {

        printf("%s  第%d行    语法错误\n",currentsym,line);
        return;
    }
    printf("第%d行    %s\n",line,ErrorArray[errornum]);
}
