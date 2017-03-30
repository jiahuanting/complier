#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"

const int BEGIN = 0x00500000;//��ʼ��ַ

extern Reg RegisterList[1000];//��¼ʶ������ĵ���
extern SymTable Table[200];//���ű�
extern ProTable PTable[100];//�ֳ����
extern void error(int errornum);//������

extern int line;
extern void print_ICL();
extern ArgNode InitArgNode();
extern int ICL_index;

extern InterCode ICL[2000];//��ԪʽInterCodeList
extern int ICL_index;//��Ԫʽ�б�ָ��
extern ArgNode Emit(int OP,ArgNode Arg1,ArgNode Arg2);//������Ԫʽ
void gensentence(FILE *out);//����MIPS����
void fprint_stack(FILE *out);//���ջ
void fprint_ICL(FILE *out);//�����Ԫʽ
int search(ArgNode A);//�ұ�ʶ��
int searchindex;//����ֵ
void scan_string(FILE *out);//ɨһ����û���ַ���


int use[32] = {0};//��ʾ�Ĵ����Ƿ��Ѿ���ʹ��
//$8-$15��ʱ�Ĵ�����$16-$23�洢�á�$24-$25��ʱ�Ĵ���

Node T[0x00500000];
int Tableindex = -1;
int VAR = 0;
int VARMAIN = 0;
int global_end = 0;
int fun_bound = 0;//��¼��ǰ�����ķ��ű�ʼλ��

int IA = 0x00500000;

///////////////////////////////////////////////////////////////////////////
InterCode IC;//��ǰ��Ԫʽ
int i;//��i����Ԫʽ
int value0,value1;//��ʱ�洢����
char IntToString[10];
char RegName[10];//�Ĵ�������
char opname1[100],opname2[100];//��ǰ��Ԫʽ�Ĳ�����
int opvalue1,opvalue2;
///////////////////////////////////////////////////////////////////////////
void in_stack(ArgNode A,FILE *out);//��ջ
ArgNode cmpArg1,cmpArg2;//�洢���ڱȽϵ������Ĵ���
int jumparray[100];//�����Ҫ�ӱ�ǩ����Ԫʽ��ַ
int jindex = -1;//���������ָ��

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void genmips()
{
    FILE *out;
    out = fopen("mips.txt","w");

    fprintf(out,"#MIPS_CODE:\n");
    fprintf(out,".data\n");
    scan_string(out);
    fprintf(out,"\n.text\n");
    fprintf(out,"addi $s7,$zero,0x%x\n",IA);//�ѵ�ַ���ص�S7��ջ����ַ
    fprintf(out,"addi $fp,$zero,0x00500000\n\n");//��������fp��0
    VARMAIN = 0;
    for(i = 0;i<ICL_index;i++)//ȫ�ֳ�������
    {
        IC = ICL[i];
        if(IC.OP==GENCONST)
        {
            fprintf(out,"addi $s7,$s7,-4\n");
            if(IC.Result.arg_type==1)   value0 = IC.Result.int_type_value;
            if(IC.Result.arg_type==2)   value0 = IC.Result.char_type_value;
            fprintf(out,"addi $t0,$zero,%d\n",value0);
            fprintf(out,"sw $t0,0($s7)\n");
            T[++Tableindex].addr = ++VARMAIN;
            strcpy(T[Tableindex].name,IC.Result.ident_type_value);
        }
        else    break;
    }
    for(i = i;i<ICL_index;i++)//��������
    {
        IC = ICL[i];
        if(IC.OP==GENVAR)
        {
            fprintf(out,"addi $s7,$s7,-4\n");
            fprintf(out,"addi $t0,$zero,10\n");
            fprintf(out,"sw $t0,0($s7)\n");
            T[++Tableindex].addr = ++VARMAIN;
            strcpy(T[Tableindex].name,IC.Result.ident_type_value);
        }
        else if(IC.OP==GENARRAY)
        {
            fprintf(out,"addi $s7,$s7,-4\n");//ջ��-4
            fprintf(out,"addi $t0,$zero,20\n");//�������׵�ַ����20
            fprintf(out,"sw $t0,0($s7)\n");
            fprintf(out,"addi $s7,$s7,-%d\n",4*(IC.Arg2.int_type_value-1));
            T[++Tableindex].addr = ++VARMAIN;//��¼�׵�ַ
            strcpy(T[Tableindex].name,IC.Result.ident_type_value);//��¼���������
            VARMAIN = VARMAIN + IC.Arg2.int_type_value - 1;//�������������
            Tableindex = Tableindex + IC.Arg2.int_type_value - 1;

        }
        else    break;
    }
    global_end = VARMAIN-1;//���һ��ȫ�ֱ�����λ��
    IC = ICL[i];
    fprintf(out,"j main\n");
    gensentence(out);
    fclose(out);
    return;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void gensentence(FILE *out)
{
    int temp = 0;
    int paranum = 0;

    for(i = i;i<=ICL_index;i++)
    {

        IC = ICL[i];
        for(temp = 0;temp <= jindex;temp++)//�ж��ǲ�����Ҫ�ӱ�ǩ
        {
            if(jumparray[temp]==i)
            {
                fprintf(out,"A%d:\n",i);
                break;
            }
        }

        if(IC.OP==LABEL && strcmp(IC.Result.ident_type_value,"main")==0)//��main����
        {
            fun_bound = Tableindex;
            VAR = VARMAIN;
            fprintf(out,"%s:\n",IC.Result.ident_type_value);//�����������ΪLABEL
        }


        else if(IC.OP==LABEL && IC.Result.arg_type==9)//����
        {
            fun_bound = Tableindex;
            VAR = 0;
            fprintf(out,"%s:\n",IC.Result.ident_type_value);//�����������ΪLABEL
            fprintf(out,"addi $s7,$s7,-4\n");
            fprintf(out,"sw $fp,0($s7)\n");//������һ���fp��Ҳ������һ����Ҫ���ص�ջͷ��
            fprintf(out,"move $fp,$s7#save stack index to fp\n");//�ѵ�ǰջָ�����fp
        }
        else if(IC.OP==LABEL && IC.Result.arg_type==3)//������label
        {
            fprintf(out,"%s:\n",IC.Result.ident_type_value);//�����������ΪLABEL
        }
        else if(IC.OP==GENCONST)
        {
            fprintf(out,"addi $s7,$s7,-4#gen const\n");
            if(IC.Result.arg_type==1)   value0 = IC.Result.int_type_value;
            if(IC.Result.arg_type==2)   value0 = IC.Result.char_type_value;
            fprintf(out,"addi $t0,$zero,%d\n",value0);
            fprintf(out,"sw $t0,0($s7)\n");
            T[++Tableindex].addr = ++VAR;//������ű�
            strcpy(T[Tableindex].name,IC.Result.ident_type_value);
        }
        else if(IC.OP==GENVAR)
        {
            fprintf(out,"addi $s7,$s7,-4#gen var\n");
            fprintf(out,"addi $t0,$zero,10\n");
            fprintf(out,"sw $t0,0($s7)\n");
            T[++Tableindex].addr = ++VAR;
            strcpy(T[Tableindex].name,IC.Result.ident_type_value);
        }
        else if(IC.OP==GENARRAY)
        {
            fprintf(out,"addi $s7,$s7,-4#gen array\n");//ջ��-4
            fprintf(out,"addi $t0,$zero,20\n");//�������׵�ַ����20
            fprintf(out,"sw $t0,0($s7)\n");
            fprintf(out,"addi $s7,$s7,-%d\n",4*(IC.Arg2.int_type_value-1));

            T[++Tableindex].addr = ++VAR;//��¼�׵�ַ
            VAR = VAR + IC.Arg2.int_type_value - 1;//�������������
            strcpy(T[Tableindex].name,IC.Result.ident_type_value);//��¼���������
        }
        else if(IC.OP==ARRAY)//������Ԫ�ص�ֵ����һ����ʱ����
        {
            if(IC.Result.arg_type==7 && search(IC.Result)==-1)//��������������ʱ������ֻ����result��ջ
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Result.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Result));
                fprintf(out,"move $s7,$t0\n");//$t0�ǵ�ǰ���һ��λ��
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
            //�����ж�������±��ǲ�������
            if(IC.Arg2.arg_type==1)
            {
				if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Arg1)-10000)-4*IC.Arg2.int_type_value);
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Arg1)+4*IC.Arg2.int_type_value);//ȡ�������׵�ַ����ƫ�Ƶ�ַ
            }
            else
            {
				if(search(IC.Arg2)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Arg2)-10000));
                else
                {
                    fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Arg2));//�ҵ���ʶ���ĵ�ַ

                }
                /////////////////////////////////////////////////////////
                //fprintf(out,"#search:%d  global:%d\n",search(IC.Arg2),global_end);
                //fprint_stack(out);
                /////////////////////////////////////////////////////////
                fprintf(out,"lw $t0,0($t0)#index\n");//�ѱ�ʶ����ֵ����t0
                fprintf(out,"sll $t0,$t0,2#index X 4\n");//��ʶ��X4=��ַ
                if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Arg1)-10000));
				else
                {
                    fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Arg1));//�ҵ�������׵�ַ

                }
                fprintf(out,"sub $t0,$t1,$t0\n");//����Ԫ�صĵ�ַ
            }
            fprintf(out,"lw $t0,0($t0)\n");//���뵽�Ĵ���
            fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//������ʱ�����ĵ�ַ
            fprintf(out,"sw $t0,0($t1)\n");//��t0�е����ݴ浽Ŀ���ַ
        }
        else if(IC.OP==ASSIARRAY)//������Ԫ�ظ�ֵ
        {
            //���ж�Ҫ����ֵ�ǲ��Ǳ���
            if(IC.Arg1.arg_type==1) fprintf(out,"addi $t0,$zero,%d\n",IC.Arg1.int_type_value);
            else if(IC.Arg1.arg_type==2)    fprintf(out,"addi $t0,$zero,%d\n",IC.Arg1.char_type_value);
            else
            {
				if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Arg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Arg1));
                fprintf(out,"lw $t0,0($t0)\n");//�浽t0
            }
            //���ж������±��ǲ��Ǳ���
            if(IC.Arg2.arg_type==1)
            {
                fprintf(out,"addi $t1,$zero,-%d\n",4*IC.Arg2.int_type_value);
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t2,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t2,$fp,-%d\n",4*search(IC.Result));//�ҵ�������׵�ַ
                fprintf(out,"add $t1,$t1,$t2\n");//����Ԫ�صĵ�ַ�浽t1
            }
            else
            {
				if(search(IC.Arg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Arg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Arg2));//�ҵ���ʶ���ĵ�ַ
                fprintf(out,"lw $t1,0($t1)\n");//�ѱ�ʶ����ֵ����t1
                fprintf(out,"sll $t1,$t1,2\n");//��ַX4
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t2,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t2,$fp,-%d\n",4*search(IC.Result));//�ҵ�������׵�ַ
                fprintf(out,"sub $t1,$t2,$t1\n");//����Ԫ�صĵ�ַ�浽t1
            }
            fprintf(out,"sw $t0,0($t1)\n");
        }
        else if(IC.OP==ASSI)//�Ǹ�ֵ���lw//////////////////////////////////////////////////////////////��������д�
        {
            if(IC.Arg1.arg_type==7 && search(IC.Arg1)==-1)//���Arg1����ʱ��������ջ
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Arg1.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Arg1));
                fprintf(out,"move $s7,$t0\n");//$t0�ǵ�ǰ���һ��λ��
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
            if(IC.Result.arg_type==7 && search(IC.Result)==-1)//����������ʱ��������ջ
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Result.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Result));
                fprintf(out,"move $s7,$t0\n");//$t0�ǵ�ǰ���һ��λ��
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }

            if(IC.Arg1.arg_type==1 || IC.Arg1.arg_type==2)//��ֵ����������
            {
                if(IC.Arg1.arg_type==1) fprintf(out,"add $t0,$zero,%d\n",IC.Arg1.int_type_value);
                if(IC.Arg1.arg_type==2) fprintf(out,"add $t0,$zero,%d\n",IC.Arg1.char_type_value);
                if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
				else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//�ҵ�Ҫ����ֵ�ı�ʶ��
                fprintf(out,"sw $t0,0($t1)\n");
            }
            else
            {
				if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Arg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Arg1));//�����һ���������ĵ�ַ
                fprintf(out,"lw $t0,0($t0)\n");//���뵽�Ĵ���
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//����ڶ����������ĵ�ַ
                fprintf(out,"sw $t0,0($t1)\n");//��t0�е����ݴ浽Ŀ���ַ
            }
        }
        else if(IC.OP==ADD)//����Ǽӷ�����
        {
            if(IC.Arg1.arg_type==7 && search(IC.Arg1)==-1)//����ʱ��������ջ
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Arg1.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Arg1));
                fprintf(out,"move $s7,$t0\n");//$t0�ǵ�ǰ���һ��λ��
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
            if(IC.Arg2.arg_type==7 && search(IC.Arg2)==-1)//����ʱ��������ջ
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Arg2.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Arg2));
                fprintf(out,"move $s7,$t0\n");//$t0�ǵ�ǰ���һ��λ��
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
            if(IC.Result.arg_type==7 && search(IC.Result)==-1)//����ʱ��������ջ
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Result.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Result));
                fprintf(out,"move $s7,$t0\n");//$t0�ǵ�ǰ���һ��λ��
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");

            }
            if((IC.Arg1.arg_type==1||IC.Arg1.arg_type==2) && (IC.Arg2.arg_type==1||IC.Arg2.arg_type==2))
            {
                if(IC.Arg1.arg_type==1) fprintf(out,"addi $t0,$zero,%d#add 11\n",IC.Arg1.int_type_value);
                if(IC.Arg1.arg_type==2) fprintf(out,"addi $t0,$zero,%d#add 11\n",IC.Arg1.char_type_value);
                if(IC.Arg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d#add 11\n",IC.Arg2.int_type_value);
                if(IC.Arg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d#add 11\n",IC.Arg2.char_type_value);
                fprintf(out,"add $t0,$t0,$t1\n");
                if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//�������ĵ�ַ
                fprintf(out,"sw $t0,0($t1)\n");//д���ڴ�
            }
            else if((IC.Arg1.arg_type==1||IC.Arg1.arg_type==2) && (IC.Arg2.arg_type==3 || IC.Arg2.arg_type==4 || IC.Arg2.arg_type==5 || IC.Arg2.arg_type==6 ||IC.Arg2.arg_type==7))//�����һ�������������ֻ����ַ�
            {
                if(IC.Arg1.arg_type==1) fprintf(out,"addi $t0,$zero,%d#add 13\n",IC.Arg1.int_type_value);//�ѵ�һ������������t1
                if(IC.Arg1.arg_type==2) fprintf(out,"addi $t0,$zero,%d#add 13\n",IC.Arg1.char_type_value);//�ѵ�һ������������t1

				if(search(IC.Arg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Arg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Arg2));//����ڶ����������ĵ�ַ
                fprintf(out,"lw $t1,0($t1)\n");//���뵽�Ĵ���

                fprintf(out,"add $t0,$t0,$t1\n");//����

				if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//�����һ���������ĵ�ַ����Ϊt0��ռ��
                fprintf(out,"sw $t0,0($t1)\n");//д���ڴ�
            }
            else if((IC.Arg1.arg_type==3 || IC.Arg1.arg_type==4 || IC.Arg1.arg_type==5 || IC.Arg1.arg_type==6 || IC.Arg1.arg_type==7) && (IC.Arg2.arg_type==1||IC.Arg2.arg_type==2))//����ڶ��������������ֻ����ַ�
            {
				if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Arg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Arg1));//�����һ���������ĵ�ַ
                fprintf(out,"lw $t0,0($t0)\n");//���뵽�Ĵ���

                if(IC.Arg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d#add 31\n",IC.Arg2.int_type_value);
                if(IC.Arg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d#add 31\n",IC.Arg2.char_type_value);

                fprintf(out,"add $t0,$t0,$t1\n");//����

				if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//�����һ���������ĵ�ַ����Ϊt0��ռ��
                fprintf(out,"sw $t0,0($t1)\n");//д���ڴ�
            }
            else//�������Ǳ�ʶ��
            {
				if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d#add 33\n",IA-4*(search(IC.Arg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d#add 33\n",4*search(IC.Arg1));//�����һ���������ĵ�ַ
                fprintf(out,"lw $t0,0($t0)\n");//���뵽�Ĵ���
				if(search(IC.Arg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Arg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Arg2));//�����һ���������ĵ�ַ
                fprintf(out,"lw $t1,0($t1)\n");//���뵽�Ĵ���

                fprintf(out,"add $t0,$t0,$t1\n");
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//��Ҫ����ĵ�ַ�ŵ�t1
                fprintf(out,"sw $t0,0($t1)\n");//�����ڴ�
            }
        }
        else if(IC.OP==SUB)
        {
            if(IC.Arg1.arg_type==7 && search(IC.Arg1)==-1)
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Arg1.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Arg1));
                fprintf(out,"move $s7,$t0\n");//$t0�ǵ�ǰ���һ��λ��
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
            if(IC.Arg2.arg_type==7 && search(IC.Arg2)==-1)
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Arg2.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Arg2));
                fprintf(out,"move $s7,$t0\n");//$t0�ǵ�ǰ���һ��λ��
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
            if(IC.Result.arg_type==7 && search(IC.Result)==-1)
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Result.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Result));
                fprintf(out,"move $s7,$t0\n");//$t0�ǵ�ǰ���һ��λ��
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
            if((IC.Arg1.arg_type==1 || IC.Arg1.arg_type==2)&&(IC.Arg2.arg_type==1 || IC.Arg2.arg_type==2))
            {
                if(IC.Arg1.arg_type==1) fprintf(out,"addi $t0,$zero,%d#sub 11\n",IC.Arg1.int_type_value);
                if(IC.Arg1.arg_type==2) fprintf(out,"addi $t0,$zero,%d#sub 11\n",IC.Arg1.char_type_value);
                if(IC.Arg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d#sub 11\n",IC.Arg2.int_type_value);
                if(IC.Arg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d#sub 11\n",IC.Arg2.char_type_value);
                fprintf(out,"sub $t0,$t0,$t1\n");
                if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//�������ĵ�ַ
                fprintf(out,"sw $t0,0($t1)\n");//д���ڴ�
            }
            else if((IC.Arg1.arg_type==1 || IC.Arg1.arg_type==2) && (IC.Arg2.arg_type==3 || IC.Arg2.arg_type==4 ||IC.Arg2.arg_type==5 || IC.Arg2.arg_type==6 || IC.Arg2.arg_type==7))//�����һ�������������ֻ����ַ�
            {
                if(IC.Arg1.arg_type==1) fprintf(out,"addi $t0,$zero,%d\n",IC.Arg1.int_type_value);
                if(IC.Arg1.arg_type==2) fprintf(out,"addi $t0,$zero,%d\n",IC.Arg1.char_type_value);

				if(search(IC.Arg2)>=10000)	fprintf(out,"addi $t1,$zero,%d#sub 13\n",IA-4*(search(IC.Arg2)-10000));
                else fprintf(out,"addi $t1,$fp,-%d#sub 13\n",4*search(IC.Arg2));//����ڶ����������ĵ�ַ
                fprintf(out,"lw $t1,0($t1)\n");//���뵽�Ĵ���

                fprintf(out,"sub $t0,$t0,$t1\n");//����
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//�������ĵ�ַ
                fprintf(out,"sw $t0,0($t1)\n");//д���ڴ�
            }
            else if((IC.Arg1.arg_type==3 || IC.Arg1.arg_type==4 || IC.Arg1.arg_type==5 || IC.Arg1.arg_type==6 || IC.Arg1.arg_type==7)&& (IC.Arg2.arg_type==1||IC.Arg2.arg_type==2))//����ڶ��������������ֻ����ַ�
            {
				if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d#sub 31\n",IA-4*(search(IC.Arg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d#sub 31\n",4*search(IC.Arg1));//�����һ���������ĵ�ַ
                fprintf(out,"lw $t0,0($t0)\n");//���뵽�Ĵ���

                if(IC.Arg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d\n",IC.Arg2.int_type_value);
                if(IC.Arg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d\n",IC.Arg2.char_type_value);
                fprintf(out,"sub $t0,$t0,$t1\n");//����
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//��������ַ
                fprintf(out,"sw $t0,0($t1)\n");//д���ڴ�
            }
            else//�������Ǳ�ʶ��
            {
				if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d#sub 33\n",IA-4*(search(IC.Arg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d#sub 33\n",4*search(IC.Arg1));//�����һ���������ĵ�ַ
                fprintf(out,"lw $t0,0($t0)\n");//���뵽�Ĵ���
				if(search(IC.Arg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Arg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Arg2));//�����һ���������ĵ�ַ
                fprintf(out,"lw $t1,0($t1)\n");//���뵽�Ĵ���

                fprintf(out,"sub $t0,$t0,$t1\n");
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//��Ҫ����ĵ�ַ�ŵ�t1
                fprintf(out,"sw $t0,0($t1)\n");//�����ڴ�
            }
        }
        else if(IC.OP==CMP)
        {
            fprintf(out,"nop\n");
            cmpArg1 = IC.Arg1;
            cmpArg2 = IC.Arg2;
        }
        else if(IC.OP==JGE)// jump if greater or equal
        {
            if(cmpArg1.arg_type==1) fprintf(out,"addi $t0,$zero,%d\n",cmpArg1.int_type_value);
            else if(cmpArg1.arg_type==2) fprintf(out,"addi $t0,$zero,%d\n",cmpArg1.char_type_value);
            else
            {
				if(search(cmpArg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(cmpArg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(cmpArg1));//��Ҫ����ĵ�ַ�ŵ�t0
                fprintf(out,"lw $t0,0($t0)\n");
            }
            if(cmpArg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.int_type_value);
            else if(cmpArg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.char_type_value);
            else
            {
				if(search(cmpArg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(cmpArg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(cmpArg2));//��Ҫ����ĵ�ַ�ŵ�t1
                fprintf(out,"lw $t1,0($t1)\n");
            }
            fprintf(out,"bge $t0,$t1,A%d\n",IC.Result.int_type_value);
            jumparray[++jindex] = IC.Result.int_type_value;
        }
        else if(IC.OP==JE)
        {
            if(cmpArg1.arg_type==1) fprintf(out,"addi $t0,$zero,%d\n",cmpArg1.int_type_value);
            else if(cmpArg1.arg_type==2) fprintf(out,"addi $t0,$zero,%d\n",cmpArg1.char_type_value);
            else
            {
				if(search(cmpArg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(cmpArg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(cmpArg1));//��Ҫ����ĵ�ַ�ŵ�t0
                fprintf(out,"lw $t0,0($t0)\n");
            }
            if(cmpArg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.int_type_value);
            else if(cmpArg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.char_type_value);
            else
            {
				if(search(cmpArg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(cmpArg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(cmpArg2));//��Ҫ����ĵ�ַ�ŵ�t1
                fprintf(out,"lw $t1,0($t1)\n");
            }
            fprintf(out,"beq $t0,$t1,A%d\n",IC.Result.int_type_value);
            jumparray[++jindex] = IC.Result.int_type_value;
        }
        else if(IC.OP==JNE)
        {
            if(cmpArg1.arg_type==1) fprintf(out,"addi $t0,$zero,%d\n",cmpArg1.int_type_value);
            else if(cmpArg1.arg_type==2) fprintf(out,"addi $t0,$zero,%d\n",cmpArg1.char_type_value);
            else
            {
				if(search(cmpArg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(cmpArg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(cmpArg1));//��Ҫ����ĵ�ַ�ŵ�t0
                fprintf(out,"lw $t0,0($t0)\n");
            }
            if(cmpArg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.int_type_value);
            else if(cmpArg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.char_type_value);
            else
            {
				if(search(cmpArg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(cmpArg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(cmpArg2));//��Ҫ����ĵ�ַ�ŵ�t1
                fprintf(out,"lw $t1,0($t1)\n");
            }
            fprintf(out,"bne $t0,$t1,A%d\n",IC.Result.int_type_value);
            jumparray[++jindex] = IC.Result.int_type_value;
        }
        else if(IC.OP==JL)
        {
            if(cmpArg1.arg_type==1) fprintf(out,"addi $t0,$zero,%d\n",cmpArg1.int_type_value);
            else if(cmpArg1.arg_type==2) fprintf(out,"addi $t0,$zero,%d\n",cmpArg1.char_type_value);
            else
            {
				if(search(cmpArg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(cmpArg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(cmpArg1));//��Ҫ����ĵ�ַ�ŵ�t0
                fprintf(out,"lw $t0,0($t0)\n");
            }
            if(cmpArg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.int_type_value);
            else if(cmpArg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.char_type_value);
            else
            {
				if(search(cmpArg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(cmpArg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(cmpArg2));//��Ҫ����ĵ�ַ�ŵ�t1
                fprintf(out,"lw $t1,0($t1)\n");
            }
            fprintf(out,"blt $t0,$t1,A%d\n",IC.Result.int_type_value);
            jumparray[++jindex] = IC.Result.int_type_value;
        }
        else if(IC.OP==JLE)
        {
            if(cmpArg1.arg_type==1) fprintf(out,"addi $t0,$zero,%d\n",cmpArg1.int_type_value);
            else if(cmpArg1.arg_type==2) fprintf(out,"addi $t0,$zero,%d\n",cmpArg1.char_type_value);
            else
            {
				if(search(cmpArg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(cmpArg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(cmpArg1));//��Ҫ����ĵ�ַ�ŵ�t0
                fprintf(out,"lw $t0,0($t0)\n");
            }
            if(cmpArg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.int_type_value);
            else if(cmpArg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.char_type_value);
            else
            {
				if(search(cmpArg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(cmpArg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(cmpArg2));//��Ҫ����ĵ�ַ�ŵ�t1
                fprintf(out,"lw $t1,0($t1)\n");
            }
            fprintf(out,"ble $t0,$t1,A%d\n",IC.Result.int_type_value);
            jumparray[++jindex] = IC.Result.int_type_value;
        }
        else if(IC.OP==JG)
        {
            if(cmpArg1.arg_type==1) fprintf(out,"addi $t0,$zero,%d\n",cmpArg1.int_type_value);
            else if(cmpArg1.arg_type==2) fprintf(out,"addi $t0,$zero,%d\n",cmpArg1.char_type_value);
            else
            {
				if(search(cmpArg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(cmpArg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(cmpArg1));//��Ҫ����ĵ�ַ�ŵ�t0
                fprintf(out,"lw $t0,0($t0)\n");
            }
            if(cmpArg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.int_type_value);
            else if(cmpArg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.char_type_value);
            else
            {
				if(search(cmpArg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(cmpArg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(cmpArg2));//��Ҫ����ĵ�ַ�ŵ�t1
                fprintf(out,"lw $t1,0($t1)\n");
                //T[++Tableindex].addr = ++VAR;
                //strcpy(T[Tableindex].name,cmpArg2.ident_type_value);
            }
            fprintf(out,"bgt $t0,$t1,A%d\n",IC.Result.int_type_value);
            jumparray[++jindex] = IC.Result.int_type_value;
        }
        else if(IC.OP==JMP && IC.Result.int_type_value!=0)
        {
            fprintf(out,"j A%d\n",IC.Result.int_type_value);
            jumparray[++jindex] = IC.Result.int_type_value;
        }
        else if(IC.OP==PARAMX)
        {
            T[++Tableindex].addr = ++VAR;
            strcpy(T[Tableindex].name,IC.Result.ident_type_value);

            paranum = IC.Arg2.int_type_value;
			if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Result)-10000));
            else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));//�ҵ����λ��
            fprintf(out,"move $s7,$t0\n");//$t0�ǵ�ǰ���һ��λ��
            fprintf(out,"addi $t0,$s7,%d\n",8+4*paranum);
            fprintf(out,"lw $t0,0($t0)\n");//�����������Ĳ���
            fprintf(out,"sw $t0,0($s7)\n");//�浽ջ��
        }
        else if(IC.OP==PARAM)
        {
            if(IC.Result.arg_type==1 || IC.Result.arg_type==2)
            {
                VAR++;//ջ��+1
                fprintf(out,"addi $s7,$s7,-4\n");
                fprintf(out,"addi $t0,$zero,0x%x\n",IC.Result.int_type_value);
                fprintf(out,"sw $t0,0($s7)\n");//�������������ջ
            }
            else
            {
                VAR++;//ջ��+1
                fprintf(out,"addi $s7,$s7,-4\n");
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));
                fprintf(out,"lw $t0,0($t0)\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }

        }
        else if(IC.OP==CALL)
        {
            fprintf(out,"addi $s7,$s7,-4\n");
            fprintf(out,"sw $ra,0($s7)\n");//����һ��ķ��ص�ַ����
            fprintf(out,"jal %s\n",IC.Result.ident_type_value);//��ת�����·��ص�ַ
            fprintf(out,"lw $ra,0($s7)\n");//����һ��ķ���ֵ�ͻ�
            fprintf(out,"addi $s7,$s7,4#sendback stack\n");//��ջ
        }
        else if(IC.OP==RETURNNVOID)
        {
            if(IC.Result.arg_type==1)
            {
                fprintf(out,"addi $t0,$zero,%d\n",IC.Result.int_type_value);
                fprintf(out,"add $v0,$zero,$t0\n");
            }
            else if(IC.Result.arg_type==2)
            {
                fprintf(out,"addi $t0,$zero,%d\n",IC.Result.char_type_value);
                fprintf(out,"add $v0,$zero,$t0\n");
            }
            else
            {
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));//�ҵ�Ҫ���ر����ĵ�ַ
                fprintf(out,"lw $v0,0($t0)\n");//���·���ֵ

            }

            fprintf(out,"addi $s7,$s7,-4\n");
            fprintf(out,"move $s7,$fp\n");//����һ���sp����ǰsp
            fprintf(out,"lw $fp,0($s7)\n");//����һ���fpָ���ͻ�
            fprintf(out,"addi $s7,$s7,4\n");

            fprintf(out,"jr $ra\n");//����
        }
        else if(IC.OP==RETURNVOID)
        {

            fprintf(out,"move $s7,$fp\n");//����һ���sp����ǰsp
            fprintf(out,"lw $fp,0($s7)\n");//����һ���fpָ���ͻ�
            fprintf(out,"addi $s7,$s7,4\n");
            fprintf(out,"jr $ra\n");//����
        }
        else if(IC.OP==GETINTRVALUE)
        {
            if(IC.Result.arg_type==5 && search(IC.Result)==-1)
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Result.ident_type_value);
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d#new temp\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Result));
                fprintf(out,"move $s7,$t0\n");//$t0�ǵ�ǰ���һ��λ��
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
			if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Result)-10000));
            else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));//�ҵ�Ҫ��صĵ�ַ
            fprintf(out,"sw $v0,0($t0)\n");
        }
        else if(IC.OP==GETCHARRVALUE)
        {
            if(IC.Result.arg_type==6 && search(IC.Result)==-1)
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Result.ident_type_value);
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d#new temp#new temp\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Result));
                fprintf(out,"move $s7,$t0\n");//$t0�ǵ�ǰ���һ��λ��
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
			if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Result)-10000));
            else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));//�ҵ�Ҫ��صĵ�ַ
            fprintf(out,"sw $v0,0($t0)\n");
        }
        else if(IC.OP==PRINTSTR)
        {
            fprintf(out,"la $a0,%s\n",IC.Result.string_type_value);
            fprintf(out,"li $v0,4#print str\n");
            fprintf(out,"syscall\n");

        }
        else if(IC.OP==PRINTINTIDENT)
        {
			if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Result)-10000));
            else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));//�ҵ�Ҫ��صĵ�ַ
            fprintf(out,"lw $t0,0($t0)\n");//��Ҫ��ӡ�����浽t0
            fprintf(out,"li $v0,1#print int var\n");
            fprintf(out,"move $a0,$t0\n");
            fprintf(out,"syscall\n");
        }
        else if(IC.OP==PRINTCHARIDENT)
        {
			if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Result)-10000));
            else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));//�ҵ�Ҫ��صĵ�ַ
            fprintf(out,"lw $t0,0($t0)\n");//��Ҫ��ӡ���ַ��浽t0
            fprintf(out,"li $v0,11#print char var\n");
            fprintf(out,"move $a0,$t0\n");
            fprintf(out,"syscall\n");
        }
        else if(IC.OP==PRINTINT)
        {
            fprintf(out,"addi $a0,$zero,%d\n",IC.Result.int_type_value);
            fprintf(out,"li $v0,1#print integer\n");
            fprintf(out,"syscall\n");
        }
        else if(IC.OP==PRINTCHAR)
        {
            fprintf(out,"addi $a0,$zero,%d\n",IC.Result.char_type_value);
            fprintf(out,"li $v0,11#print character\n");
            fprintf(out,"syscall\n");
        }
        else if(IC.OP==SCANINT)
        {
            fprintf(out,"li $v0,5#scanf int\n");
            fprintf(out,"syscall\n");
			if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Result)-10000));
            else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));//�ҵ����յ�ַ
            fprintf(out,"sw $v0,0($t0)\n");//�浽ջ��
        }
        else if(IC.OP==SCANCHAR)
        {
            fprintf(out,"li $v0,12#scanf char\n");
            fprintf(out,"syscall\n");
			if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Result)-10000));
            else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));//�ҵ����յ�ַ
            fprintf(out,"sw $v0,0($t0)\n");//�浽ջ��
        }
        else if(IC.OP==RETURNMAIN)
        {
            fprintf(out,"#End of Program.\n");
        }
        else if(IC.OP==MUL)
        {
            if(IC.Result.arg_type==7 && search(IC.Result)==-1)//�������ʱ��������ջ
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Result.ident_type_value);
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d#new temp\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Result));
                fprintf(out,"move $s7,$t0\n");//$t0�ǵ�ǰ���һ��λ��
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
            if((IC.Arg1.arg_type==3 || IC.Arg1.arg_type==7 || IC.Arg1.arg_type==5)&& (IC.Arg2.arg_type==3 || IC.Arg2.arg_type==7 || IC.Arg2.arg_type==5))//�������Ǳ�ʶ��
            {
                if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Arg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Arg1));
                fprintf(out,"lw $t0,0($t0)\n");
                if(search(IC.Arg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Arg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Arg2));
                fprintf(out,"lw $t1,0($t1)\n");
            }
            else if(IC.Arg1.arg_type==1 && (IC.Arg2.arg_type==3 || IC.Arg2.arg_type==7 || IC.Arg2.arg_type==5))
            {
                fprintf(out,"addi $t0,$zero,%d\n",IC.Arg1.int_type_value);
                if(search(IC.Arg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Arg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Arg2));
                fprintf(out,"lw $t1,0($t1)\n");
            }
			else if((IC.Arg1.arg_type==3 || IC.Arg1.arg_type==7 || IC.Arg1.arg_type==5) && IC.Arg2.arg_type==1)
            {
                if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Arg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Arg1));
                fprintf(out,"lw $t0,0($t0)\n");
                fprintf(out,"addi $t1,$zero,%d\n",IC.Arg2.int_type_value);
            }
            else if(IC.Arg1.arg_type==1 && IC.Arg2.arg_type==1)
            {
                fprintf(out,"addi $t0,$zero,%d\n",IC.Arg1.int_type_value);
                fprintf(out,"addi $t1,$zero,%d\n",IC.Arg2.int_type_value);
            }

            fprintf(out,"mult $t0,$t1\n");
			if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Result)-10000));
            else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));
            fprintf(out,"mflo $t1\n");
            fprintf(out,"sw $t1,0($t0)\n");
        }
        else if(IC.OP==DIV)
        {
            if(IC.Result.arg_type==7 && search(IC.Result)==-1)
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Result.ident_type_value);
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d#new temp\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Result));
                fprintf(out,"move $s7,$t0\n");//$t0�ǵ�ǰ���һ��λ��
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
            if((IC.Arg1.arg_type==3 || IC.Arg1.arg_type==5 || IC.Arg1.arg_type==7) && (IC.Arg2.arg_type==3 || IC.Arg2.arg_type==5 || IC.Arg2.arg_type==7))
            {
                if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Arg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Arg1));
                fprintf(out,"lw $t0,0($t0)\n");
                if(search(IC.Arg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Arg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Arg2));
                fprintf(out,"lw $t1,0($t1)\n");
            }
            else if(IC.Arg1.arg_type==1 && (IC.Arg2.arg_type==3 || IC.Arg2.arg_type==5 || IC.Arg2.arg_type==7))
            {
                fprintf(out,"addi $t0,$zero,%d\n",IC.Arg1.int_type_value);
                if(search(IC.Arg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Arg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Arg2));
                fprintf(out,"lw $t1,0($t1)\n");
            }
			else if((IC.Arg1.arg_type==3 || IC.Arg1.arg_type==5 || IC.Arg1.arg_type==7) && IC.Arg2.arg_type==1)
            {
                if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Arg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Arg1));
                fprintf(out,"lw $t0,0($t0)\n");
                fprintf(out,"addi $t1,$zero,%d\n",IC.Arg2.int_type_value);
            }
            else if(IC.Arg1.arg_type==1 && IC.Arg2.arg_type==1)
            {
                fprintf(out,"addi $t0,$zero,%d\n",IC.Arg1.int_type_value);
                fprintf(out,"addi $t1,$zero,%d\n",IC.Arg2.int_type_value);
            }
            fprintf(out,"div $t0,$t1\n");
			if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Result)-10000));
            else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));
            fprintf(out,"mflo $t1\n");
            fprintf(out,"sw $t1,0($t0)\n");
        }
        else continue;
    }
}

void fprint_stack(FILE *out)
{
    int i = 0;
    fprintf(out,"#---------------ջ---------------\n");
    for(i = 0;i<=200;i++)    fprintf(out,"#%5d %10s\n",i,T[i].name);
    fprintf(out,"#--------------------------------\n");
}

void fprint_ICL(FILE *out)
{
    char Name[][30] = {"ADD","SUB","MUL","DIV","ARRAYVALUE","ARRAYADDR","OPPO","ASSI","ASSIGNRVALUE","ARRAYASSIGN",
                       "CMP","JE","JNE","JL","JLE","JG","JGE","JMP","PARAM","CALL","GETRVALUE","RETURNNVOID","VRETURN",
                       "PRINTSTR","PRINTINT","PRINTCHAR","PRINTIDENT","SCANINT","SCANCHAR","LABEL","GENCONST","GENVAR","PARAMX","RETURNVOID","RETURNMAIN","GENARRAY","ARRAY",
                       "ASSIARRAY","ONEORZERO","UNKNOWN"};
    int i= 0;
    InterCode IC;
    for(i = 0;i<=ICL_index;i++)
    {
        IC = ICL[i];
        fprintf(out,"%5d%10s",i,Name[IC.OP]);
        if(IC.Arg1.arg_type==1) fprintf(out,"%8d",IC.Arg1.int_type_value);
        else if(IC.Arg1.arg_type==2) fprintf(out,"%8c",IC.Arg1.char_type_value);
        else    fprintf(out,"%8s",IC.Arg1.ident_type_value);
        if(IC.Arg2.arg_type==1) fprintf(out,"%8d",IC.Arg2.int_type_value);
        else if(IC.Arg2.arg_type==2) fprintf(out,"%8c",IC.Arg2.char_type_value);
        else    fprintf(out,"%8s",IC.Arg2.ident_type_value);

        if(IC.Result.arg_type==1) fprintf(out,"%8d",IC.Result.int_type_value);
        else if(IC.Result.arg_type==2) fprintf(out,"%8c",IC.Result.char_type_value);
        else if(IC.Result.arg_type==8) fprintf(out,"%20s",IC.Result.string_type_value);
        else    fprintf(out,"%8s",IC.Result.ident_type_value);
        fprintf(out,"\n");
    }
}

int search(ArgNode A)
{
    int j = 0;
    for(j = Tableindex;j > fun_bound;j--)
    {
        if(strcmp(A.ident_type_value,T[j].name)==0)
        {
            return T[j].addr;
        }
    }
    for(j = global_end;j >= 0;j--)
    {
        if(strcmp(A.ident_type_value,T[j].name)==0)
        {
            return T[j].addr+10000;
        }
    }
    return -1;//δ�ҵ�
}

void scan_string(FILE *out)
{
    int serial = 0;
    char str[10];
    int j = 0;
    for(j = 0;j<=ICL_index;j++)
    {
        if(ICL[j].OP==PRINTSTR && ICL[j].Result.arg_type==8)
        {
            fprintf(out,"S%d: .asciiz \"%s\"\n",serial++,ICL[j].Result.string_type_value);//������ַ������ȶ���
            ICL[j].Result.string_type_value[0] = 'S';
            ICL[j].Result.string_type_value[1] = '\0';
            itoa(serial-1,str,10);
            strcat(ICL[j].Result.string_type_value,str);
        }
    }
}
