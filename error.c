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
        "�ַ������ȳ����涨ֵ",           //0
        "�Ƿ���ʶ��",                     //1
        "��λ���ֲ�����0��ͷ",            //2
        "Դ�ļ�����",                     //3
        "�������²����ַ�����",           //4
        "�ַ����ͳ��Ȳ�Ϊ1",              //5
        "�Ƿ��ַ�",                       //6
        "int��char����û���ҵ���ʶ��",    //7
        "��ʶ�������ҵ��Ⱥ�",             //8
        "���Ͳ���",                       //9
        "��ֵ���ź�ֻ����+��-����ĸ����", //10
        "�������Ӧ���ԷֺŽ�β",         //11
        "����������ַǷ��ַ�",           //12
        "int��char����û���ҵ���ʶ��",    //13
        "�����Ų�ƥ��",                   //14
        "�����ź�ֻ�ܳ�������",           //15
        "������û�зֺ�",               //16
        "ȱ��С����",                     //17
        "ȱ��������",                     //18
        "ȱ���������",                   //19
        "ȱ���Ҵ�����"                    //19
    };
    if(errornum==100)
    {

        printf("%s  ��%d��    �﷨����\n",currentsym,line);
        return;
    }
    printf("��%d��    %s\n",line,ErrorArray[errornum]);
}
