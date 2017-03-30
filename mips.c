#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"

const int BEGIN = 0x00500000;//起始地址

extern Reg RegisterList[1000];//记录识别出来的单词
extern SymTable Table[200];//符号表
extern ProTable PTable[100];//分程序表
extern void error(int errornum);//错误处理

extern int line;
extern void print_ICL();
extern ArgNode InitArgNode();
extern int ICL_index;

extern InterCode ICL[2000];//四元式InterCodeList
extern int ICL_index;//四元式列表指针
extern ArgNode Emit(int OP,ArgNode Arg1,ArgNode Arg2);//生成四元式
void gensentence(FILE *out);//生成MIPS代码
void fprint_stack(FILE *out);//输出栈
void fprint_ICL(FILE *out);//输出四元式
int search(ArgNode A);//找标识符
int searchindex;//返回值
void scan_string(FILE *out);//扫一遍有没有字符串


int use[32] = {0};//表示寄存器是否已经被使用
//$8-$15临时寄存器、$16-$23存储用、$24-$25临时寄存器

Node T[0x00500000];
int Tableindex = -1;
int VAR = 0;
int VARMAIN = 0;
int global_end = 0;
int fun_bound = 0;//记录当前函数的符号表开始位置

int IA = 0x00500000;

///////////////////////////////////////////////////////////////////////////
InterCode IC;//当前四元式
int i;//第i个四元式
int value0,value1;//临时存储数字
char IntToString[10];
char RegName[10];//寄存器名字
char opname1[100],opname2[100];//当前四元式的操作数
int opvalue1,opvalue2;
///////////////////////////////////////////////////////////////////////////
void in_stack(ArgNode A,FILE *out);//入栈
ArgNode cmpArg1,cmpArg2;//存储用于比较的两个寄存器
int jumparray[100];//存放需要加标签的四元式地址
int jindex = -1;//上面数组的指针

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void genmips()
{
    FILE *out;
    out = fopen("mips.txt","w");

    fprintf(out,"#MIPS_CODE:\n");
    fprintf(out,".data\n");
    scan_string(out);
    fprintf(out,"\n.text\n");
    fprintf(out,"addi $s7,$zero,0x%x\n",IA);//把地址加载到S7，栈顶地址
    fprintf(out,"addi $fp,$zero,0x00500000\n\n");//主函数的fp是0
    VARMAIN = 0;
    for(i = 0;i<ICL_index;i++)//全局常量定义
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
    for(i = i;i<ICL_index;i++)//变量定义
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
            fprintf(out,"addi $s7,$s7,-4\n");//栈针-4
            fprintf(out,"addi $t0,$zero,20\n");//向数组首地址存入20
            fprintf(out,"sw $t0,0($s7)\n");
            fprintf(out,"addi $s7,$s7,-%d\n",4*(IC.Arg2.int_type_value-1));
            T[++Tableindex].addr = ++VARMAIN;//记录首地址
            strcpy(T[Tableindex].name,IC.Result.ident_type_value);//记录数组的名字
            VARMAIN = VARMAIN + IC.Arg2.int_type_value - 1;//开辟区域给数组
            Tableindex = Tableindex + IC.Arg2.int_type_value - 1;

        }
        else    break;
    }
    global_end = VARMAIN-1;//最后一个全局变量的位置
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
        for(temp = 0;temp <= jindex;temp++)//判断是不是需要加标签
        {
            if(jumparray[temp]==i)
            {
                fprintf(out,"A%d:\n",i);
                break;
            }
        }

        if(IC.OP==LABEL && strcmp(IC.Result.ident_type_value,"main")==0)//是main函数
        {
            fun_bound = Tableindex;
            VAR = VARMAIN;
            fprintf(out,"%s:\n",IC.Result.ident_type_value);//输出函数名作为LABEL
        }


        else if(IC.OP==LABEL && IC.Result.arg_type==9)//函数
        {
            fun_bound = Tableindex;
            VAR = 0;
            fprintf(out,"%s:\n",IC.Result.ident_type_value);//输出函数名作为LABEL
            fprintf(out,"addi $s7,$s7,-4\n");
            fprintf(out,"sw $fp,0($s7)\n");//存下上一层的fp，也就是上一层需要返回的栈头部
            fprintf(out,"move $fp,$s7#save stack index to fp\n");//把当前栈指针存入fp
        }
        else if(IC.OP==LABEL && IC.Result.arg_type==3)//单纯的label
        {
            fprintf(out,"%s:\n",IC.Result.ident_type_value);//输出函数名作为LABEL
        }
        else if(IC.OP==GENCONST)
        {
            fprintf(out,"addi $s7,$s7,-4#gen const\n");
            if(IC.Result.arg_type==1)   value0 = IC.Result.int_type_value;
            if(IC.Result.arg_type==2)   value0 = IC.Result.char_type_value;
            fprintf(out,"addi $t0,$zero,%d\n",value0);
            fprintf(out,"sw $t0,0($s7)\n");
            T[++Tableindex].addr = ++VAR;//存入符号表
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
            fprintf(out,"addi $s7,$s7,-4#gen array\n");//栈针-4
            fprintf(out,"addi $t0,$zero,20\n");//向数组首地址存入20
            fprintf(out,"sw $t0,0($s7)\n");
            fprintf(out,"addi $s7,$s7,-%d\n",4*(IC.Arg2.int_type_value-1));

            T[++Tableindex].addr = ++VAR;//记录首地址
            VAR = VAR + IC.Arg2.int_type_value - 1;//开辟区域给数组
            strcpy(T[Tableindex].name,IC.Result.ident_type_value);//记录数组的名字
        }
        else if(IC.OP==ARRAY)//把数组元素的值赋给一个临时变量
        {
            if(IC.Result.arg_type==7 && search(IC.Result)==-1)//数组名不能是临时变量，只能是result入栈
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Result.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Result));
                fprintf(out,"move $s7,$t0\n");//$t0是当前最后一个位置
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
            //首先判断数组的下标是不是整数
            if(IC.Arg2.arg_type==1)
            {
				if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Arg1)-10000)-4*IC.Arg2.int_type_value);
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Arg1)+4*IC.Arg2.int_type_value);//取出数组首地址，加偏移地址
            }
            else
            {
				if(search(IC.Arg2)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Arg2)-10000));
                else
                {
                    fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Arg2));//找到标识符的地址

                }
                /////////////////////////////////////////////////////////
                //fprintf(out,"#search:%d  global:%d\n",search(IC.Arg2),global_end);
                //fprint_stack(out);
                /////////////////////////////////////////////////////////
                fprintf(out,"lw $t0,0($t0)#index\n");//把标识符的值存入t0
                fprintf(out,"sll $t0,$t0,2#index X 4\n");//标识符X4=地址
                if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Arg1)-10000));
				else
                {
                    fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Arg1));//找到数组的首地址

                }
                fprintf(out,"sub $t0,$t1,$t0\n");//这是元素的地址
            }
            fprintf(out,"lw $t0,0($t0)\n");//读入到寄存器
            fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//保存临时变量的地址
            fprintf(out,"sw $t0,0($t1)\n");//把t0中的内容存到目标地址
        }
        else if(IC.OP==ASSIARRAY)//给数组元素赋值
        {
            //先判断要赋的值是不是变量
            if(IC.Arg1.arg_type==1) fprintf(out,"addi $t0,$zero,%d\n",IC.Arg1.int_type_value);
            else if(IC.Arg1.arg_type==2)    fprintf(out,"addi $t0,$zero,%d\n",IC.Arg1.char_type_value);
            else
            {
				if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Arg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Arg1));
                fprintf(out,"lw $t0,0($t0)\n");//存到t0
            }
            //再判断数组下标是不是变量
            if(IC.Arg2.arg_type==1)
            {
                fprintf(out,"addi $t1,$zero,-%d\n",4*IC.Arg2.int_type_value);
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t2,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t2,$fp,-%d\n",4*search(IC.Result));//找到数组的首地址
                fprintf(out,"add $t1,$t1,$t2\n");//这是元素的地址存到t1
            }
            else
            {
				if(search(IC.Arg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Arg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Arg2));//找到标识符的地址
                fprintf(out,"lw $t1,0($t1)\n");//把标识符的值存入t1
                fprintf(out,"sll $t1,$t1,2\n");//地址X4
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t2,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t2,$fp,-%d\n",4*search(IC.Result));//找到数组的首地址
                fprintf(out,"sub $t1,$t2,$t1\n");//这是元素的地址存到t1
            }
            fprintf(out,"sw $t0,0($t1)\n");
        }
        else if(IC.OP==ASSI)//是赋值语句lw//////////////////////////////////////////////////////////////这里好像有错
        {
            if(IC.Arg1.arg_type==7 && search(IC.Arg1)==-1)//如果Arg1是临时变量则入栈
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Arg1.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Arg1));
                fprintf(out,"move $s7,$t0\n");//$t0是当前最后一个位置
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
            if(IC.Result.arg_type==7 && search(IC.Result)==-1)//如果结果是临时变量则入栈
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Result.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Result));
                fprintf(out,"move $s7,$t0\n");//$t0是当前最后一个位置
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }

            if(IC.Arg1.arg_type==1 || IC.Arg1.arg_type==2)//赋值后面是数字
            {
                if(IC.Arg1.arg_type==1) fprintf(out,"add $t0,$zero,%d\n",IC.Arg1.int_type_value);
                if(IC.Arg1.arg_type==2) fprintf(out,"add $t0,$zero,%d\n",IC.Arg1.char_type_value);
                if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
				else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//找到要被赋值的标识符
                fprintf(out,"sw $t0,0($t1)\n");
            }
            else
            {
				if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Arg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Arg1));//保存第一个操作数的地址
                fprintf(out,"lw $t0,0($t0)\n");//读入到寄存器
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//保存第二个操作数的地址
                fprintf(out,"sw $t0,0($t1)\n");//把t0中的内容存到目标地址
            }
        }
        else if(IC.OP==ADD)//如果是加法运算
        {
            if(IC.Arg1.arg_type==7 && search(IC.Arg1)==-1)//有临时变量则入栈
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Arg1.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Arg1));
                fprintf(out,"move $s7,$t0\n");//$t0是当前最后一个位置
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
            if(IC.Arg2.arg_type==7 && search(IC.Arg2)==-1)//有临时变量则入栈
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Arg2.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Arg2));
                fprintf(out,"move $s7,$t0\n");//$t0是当前最后一个位置
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
            if(IC.Result.arg_type==7 && search(IC.Result)==-1)//有临时变量则入栈
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Result.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Result));
                fprintf(out,"move $s7,$t0\n");//$t0是当前最后一个位置
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
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//保存结果的地址
                fprintf(out,"sw $t0,0($t1)\n");//写回内存
            }
            else if((IC.Arg1.arg_type==1||IC.Arg1.arg_type==2) && (IC.Arg2.arg_type==3 || IC.Arg2.arg_type==4 || IC.Arg2.arg_type==5 || IC.Arg2.arg_type==6 ||IC.Arg2.arg_type==7))//如果第一个操作数是数字或者字符
            {
                if(IC.Arg1.arg_type==1) fprintf(out,"addi $t0,$zero,%d#add 13\n",IC.Arg1.int_type_value);//把第一个操作数读到t1
                if(IC.Arg1.arg_type==2) fprintf(out,"addi $t0,$zero,%d#add 13\n",IC.Arg1.char_type_value);//把第一个操作数读到t1

				if(search(IC.Arg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Arg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Arg2));//保存第二个操作数的地址
                fprintf(out,"lw $t1,0($t1)\n");//读入到寄存器

                fprintf(out,"add $t0,$t0,$t1\n");//计算

				if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//保存第一个操作数的地址，因为t0被占用
                fprintf(out,"sw $t0,0($t1)\n");//写回内存
            }
            else if((IC.Arg1.arg_type==3 || IC.Arg1.arg_type==4 || IC.Arg1.arg_type==5 || IC.Arg1.arg_type==6 || IC.Arg1.arg_type==7) && (IC.Arg2.arg_type==1||IC.Arg2.arg_type==2))//如果第二个操作数是数字或者字符
            {
				if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Arg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Arg1));//保存第一个操作数的地址
                fprintf(out,"lw $t0,0($t0)\n");//读入到寄存器

                if(IC.Arg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d#add 31\n",IC.Arg2.int_type_value);
                if(IC.Arg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d#add 31\n",IC.Arg2.char_type_value);

                fprintf(out,"add $t0,$t0,$t1\n");//计算

				if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//保存第一个操作数的地址，因为t0被占用
                fprintf(out,"sw $t0,0($t1)\n");//写回内存
            }
            else//两个都是标识符
            {
				if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d#add 33\n",IA-4*(search(IC.Arg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d#add 33\n",4*search(IC.Arg1));//保存第一个操作数的地址
                fprintf(out,"lw $t0,0($t0)\n");//读入到寄存器
				if(search(IC.Arg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Arg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Arg2));//保存第一个操作数的地址
                fprintf(out,"lw $t1,0($t1)\n");//读入到寄存器

                fprintf(out,"add $t0,$t0,$t1\n");
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//把要存入的地址放到t1
                fprintf(out,"sw $t0,0($t1)\n");//存入内存
            }
        }
        else if(IC.OP==SUB)
        {
            if(IC.Arg1.arg_type==7 && search(IC.Arg1)==-1)
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Arg1.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Arg1));
                fprintf(out,"move $s7,$t0\n");//$t0是当前最后一个位置
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
            if(IC.Arg2.arg_type==7 && search(IC.Arg2)==-1)
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Arg2.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Arg2));
                fprintf(out,"move $s7,$t0\n");//$t0是当前最后一个位置
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
            if(IC.Result.arg_type==7 && search(IC.Result)==-1)
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Result.ident_type_value);
                fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Result));
                fprintf(out,"move $s7,$t0\n");//$t0是当前最后一个位置
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
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//保存结果的地址
                fprintf(out,"sw $t0,0($t1)\n");//写回内存
            }
            else if((IC.Arg1.arg_type==1 || IC.Arg1.arg_type==2) && (IC.Arg2.arg_type==3 || IC.Arg2.arg_type==4 ||IC.Arg2.arg_type==5 || IC.Arg2.arg_type==6 || IC.Arg2.arg_type==7))//如果第一个操作数是数字或者字符
            {
                if(IC.Arg1.arg_type==1) fprintf(out,"addi $t0,$zero,%d\n",IC.Arg1.int_type_value);
                if(IC.Arg1.arg_type==2) fprintf(out,"addi $t0,$zero,%d\n",IC.Arg1.char_type_value);

				if(search(IC.Arg2)>=10000)	fprintf(out,"addi $t1,$zero,%d#sub 13\n",IA-4*(search(IC.Arg2)-10000));
                else fprintf(out,"addi $t1,$fp,-%d#sub 13\n",4*search(IC.Arg2));//保存第二个操作数的地址
                fprintf(out,"lw $t1,0($t1)\n");//读入到寄存器

                fprintf(out,"sub $t0,$t0,$t1\n");//计算
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//保存结果的地址
                fprintf(out,"sw $t0,0($t1)\n");//写回内存
            }
            else if((IC.Arg1.arg_type==3 || IC.Arg1.arg_type==4 || IC.Arg1.arg_type==5 || IC.Arg1.arg_type==6 || IC.Arg1.arg_type==7)&& (IC.Arg2.arg_type==1||IC.Arg2.arg_type==2))//如果第二个操作数是数字或者字符
            {
				if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d#sub 31\n",IA-4*(search(IC.Arg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d#sub 31\n",4*search(IC.Arg1));//保存第一个操作数的地址
                fprintf(out,"lw $t0,0($t0)\n");//读入到寄存器

                if(IC.Arg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d\n",IC.Arg2.int_type_value);
                if(IC.Arg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d\n",IC.Arg2.char_type_value);
                fprintf(out,"sub $t0,$t0,$t1\n");//计算
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//保存结果地址
                fprintf(out,"sw $t0,0($t1)\n");//写回内存
            }
            else//两个都是标识符
            {
				if(search(IC.Arg1)>=10000)	fprintf(out,"addi $t0,$zero,%d#sub 33\n",IA-4*(search(IC.Arg1)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d#sub 33\n",4*search(IC.Arg1));//保存第一个操作数的地址
                fprintf(out,"lw $t0,0($t0)\n");//读入到寄存器
				if(search(IC.Arg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Arg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Arg2));//保存第一个操作数的地址
                fprintf(out,"lw $t1,0($t1)\n");//读入到寄存器

                fprintf(out,"sub $t0,$t0,$t1\n");
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(IC.Result));//把要存入的地址放到t1
                fprintf(out,"sw $t0,0($t1)\n");//存入内存
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
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(cmpArg1));//把要存入的地址放到t0
                fprintf(out,"lw $t0,0($t0)\n");
            }
            if(cmpArg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.int_type_value);
            else if(cmpArg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.char_type_value);
            else
            {
				if(search(cmpArg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(cmpArg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(cmpArg2));//把要存入的地址放到t1
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
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(cmpArg1));//把要存入的地址放到t0
                fprintf(out,"lw $t0,0($t0)\n");
            }
            if(cmpArg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.int_type_value);
            else if(cmpArg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.char_type_value);
            else
            {
				if(search(cmpArg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(cmpArg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(cmpArg2));//把要存入的地址放到t1
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
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(cmpArg1));//把要存入的地址放到t0
                fprintf(out,"lw $t0,0($t0)\n");
            }
            if(cmpArg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.int_type_value);
            else if(cmpArg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.char_type_value);
            else
            {
				if(search(cmpArg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(cmpArg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(cmpArg2));//把要存入的地址放到t1
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
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(cmpArg1));//把要存入的地址放到t0
                fprintf(out,"lw $t0,0($t0)\n");
            }
            if(cmpArg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.int_type_value);
            else if(cmpArg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.char_type_value);
            else
            {
				if(search(cmpArg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(cmpArg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(cmpArg2));//把要存入的地址放到t1
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
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(cmpArg1));//把要存入的地址放到t0
                fprintf(out,"lw $t0,0($t0)\n");
            }
            if(cmpArg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.int_type_value);
            else if(cmpArg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.char_type_value);
            else
            {
				if(search(cmpArg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(cmpArg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(cmpArg2));//把要存入的地址放到t1
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
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(cmpArg1));//把要存入的地址放到t0
                fprintf(out,"lw $t0,0($t0)\n");
            }
            if(cmpArg2.arg_type==1) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.int_type_value);
            else if(cmpArg2.arg_type==2) fprintf(out,"addi $t1,$zero,%d\n",cmpArg2.char_type_value);
            else
            {
				if(search(cmpArg2)>=10000)	fprintf(out,"addi $t1,$zero,%d\n",IA-4*(search(cmpArg2)-10000));
                else	fprintf(out,"addi $t1,$fp,-%d\n",4*search(cmpArg2));//把要存入的地址放到t1
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
            else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));//找到这个位置
            fprintf(out,"move $s7,$t0\n");//$t0是当前最后一个位置
            fprintf(out,"addi $t0,$s7,%d\n",8+4*paranum);
            fprintf(out,"lw $t0,0($t0)\n");//读到传进来的参数
            fprintf(out,"sw $t0,0($s7)\n");//存到栈顶
        }
        else if(IC.OP==PARAM)
        {
            if(IC.Result.arg_type==1 || IC.Result.arg_type==2)
            {
                VAR++;//栈针+1
                fprintf(out,"addi $s7,$s7,-4\n");
                fprintf(out,"addi $t0,$zero,0x%x\n",IC.Result.int_type_value);
                fprintf(out,"sw $t0,0($s7)\n");//把这个整数存入栈
            }
            else
            {
                VAR++;//栈针+1
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
            fprintf(out,"sw $ra,0($s7)\n");//把上一层的返回地址存下
            fprintf(out,"jal %s\n",IC.Result.ident_type_value);//跳转并存下返回地址
            fprintf(out,"lw $ra,0($s7)\n");//把上一层的返回值送回
            fprintf(out,"addi $s7,$s7,4#sendback stack\n");//退栈
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
                else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));//找到要返回变量的地址
                fprintf(out,"lw $v0,0($t0)\n");//存下返回值

            }

            fprintf(out,"addi $s7,$s7,-4\n");
            fprintf(out,"move $s7,$fp\n");//把上一层的sp给当前sp
            fprintf(out,"lw $fp,0($s7)\n");//把上一层的fp指针送回
            fprintf(out,"addi $s7,$s7,4\n");

            fprintf(out,"jr $ra\n");//跳回
        }
        else if(IC.OP==RETURNVOID)
        {

            fprintf(out,"move $s7,$fp\n");//把上一层的sp给当前sp
            fprintf(out,"lw $fp,0($s7)\n");//把上一层的fp指针送回
            fprintf(out,"addi $s7,$s7,4\n");
            fprintf(out,"jr $ra\n");//跳回
        }
        else if(IC.OP==GETINTRVALUE)
        {
            if(IC.Result.arg_type==5 && search(IC.Result)==-1)
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Result.ident_type_value);
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d#new temp\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Result));
                fprintf(out,"move $s7,$t0\n");//$t0是当前最后一个位置
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
			if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Result)-10000));
            else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));//找到要存回的地址
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
                fprintf(out,"move $s7,$t0\n");//$t0是当前最后一个位置
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
			if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Result)-10000));
            else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));//找到要存回的地址
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
            else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));//找到要存回的地址
            fprintf(out,"lw $t0,0($t0)\n");//把要打印的数存到t0
            fprintf(out,"li $v0,1#print int var\n");
            fprintf(out,"move $a0,$t0\n");
            fprintf(out,"syscall\n");
        }
        else if(IC.OP==PRINTCHARIDENT)
        {
			if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Result)-10000));
            else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));//找到要存回的地址
            fprintf(out,"lw $t0,0($t0)\n");//把要打印的字符存到t0
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
            else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));//找到接收地址
            fprintf(out,"sw $v0,0($t0)\n");//存到栈里
        }
        else if(IC.OP==SCANCHAR)
        {
            fprintf(out,"li $v0,12#scanf char\n");
            fprintf(out,"syscall\n");
			if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d\n",IA-4*(search(IC.Result)-10000));
            else	fprintf(out,"addi $t0,$fp,-%d\n",4*search(IC.Result));//找到接收地址
            fprintf(out,"sw $v0,0($t0)\n");//存到栈里
        }
        else if(IC.OP==RETURNMAIN)
        {
            fprintf(out,"#End of Program.\n");
        }
        else if(IC.OP==MUL)
        {
            if(IC.Result.arg_type==7 && search(IC.Result)==-1)//如果有临时变量就入栈
            {
                T[++Tableindex].addr = ++VAR;
                strcpy(T[Tableindex].name,IC.Result.ident_type_value);
				if(search(IC.Result)>=10000)	fprintf(out,"addi $t0,$zero,%d#new temp\n",IA-4*(search(IC.Result)-10000));
                else	fprintf(out,"addi $t0,$fp,-%d#new temp\n",4*search(IC.Result));
                fprintf(out,"move $s7,$t0\n");//$t0是当前最后一个位置
                fprintf(out,"addi $t0,$zero,10\n");
                fprintf(out,"sw $t0,0($s7)\n");
            }
            if((IC.Arg1.arg_type==3 || IC.Arg1.arg_type==7 || IC.Arg1.arg_type==5)&& (IC.Arg2.arg_type==3 || IC.Arg2.arg_type==7 || IC.Arg2.arg_type==5))//两个都是标识符
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
                fprintf(out,"move $s7,$t0\n");//$t0是当前最后一个位置
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
    fprintf(out,"#---------------栈---------------\n");
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
    return -1;//未找到
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
            fprintf(out,"S%d: .asciiz \"%s\"\n",serial++,ICL[j].Result.string_type_value);//如果有字符串，先定义
            ICL[j].Result.string_type_value[0] = 'S';
            ICL[j].Result.string_type_value[1] = '\0';
            itoa(serial-1,str,10);
            strcat(ICL[j].Result.string_type_value,str);
        }
    }
}
