#include "stdio.h"
int main(int argc, char * argv[])
{
	int i, num1 = 0, num2 = 0, flag = 1, res = 0;
        char bufr[128];

	printf("===================================================\n");
	printf("=                  Calculator                     =\n");
	printf("===================================================\n");
	printf("=  Please enter two numbers                       =\n");
	printf("=  Enter e to quit                                =\n");
	printf("===================================================\n\n");

	while(flag == 1){	
		printf("Input num1:");
		i = read(0, bufr, 128);
		if (bufr[0] == 'e')
			break;
		num1 = getNum(bufr);
		printf("Num1= %d\n", num1);

		printf("Input num2:");
		i = read(0, bufr, 128);	
		if (bufr[0] == 'e')
			break;
		num2 = getNum(bufr);
		printf("Num2= %d\n", num2);

		printf("Input  + - * / :");
		i = read(0, bufr, 1);
		switch(bufr[0])
		{
			case '+':
				res = num1 + num2;
				printf("%d + %d = %d\n", num1, num2, res);
				break;
			case '-':
				res = num1 - num2;
				printf("%d - %d = %d\n", num1, num2, res);
				break;
			case '*':
				res = num1 * num2;
                                printf("%d * %d = %d\n", num1, num2, res);
				break;
			case '/':
				if(num2 <= 0)
				{
					printf("Num2 = 0!\n");
					break;
				}
				res = num1 / num2;
				printf("%d / %d = %d\n", num1, num2, res);
			case 'e':
				flag = 0;
				break;
			default:
				printf("Error!\n");
		}
        memset(bufr,0,100);
	}
    
	return 0;
}

int getNum(char * bufr)
{
	int ten = 1, i = 0, res = 0;
	for (i = 0; i < strlen(bufr) - 1; i++)
	{
		ten *= 10;
	}
	for (i = 0; i < strlen(bufr); i++)
	{
		res += (bufr[i] - '0') * ten;
		ten /= 10;
	}
	return res;
}
