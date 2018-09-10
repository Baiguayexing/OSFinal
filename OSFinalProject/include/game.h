#include "stdio.h"
#include "type.h"
#include "randnum.h"


int q[20];
int amount = 0;
int n;
char bufr[128] = { '\0' };
int goal = 10;
int rel = 0;

///////////////////
void printqueen(int n)
{
	int i, j;
	amount++;
	printf("\n");
	for (i = 1; i <= n; ++i)
	{
		for (int j = 1; j <= n; ++j)
		{
			if (q[i] != j)
			{
				printf("0 ");
			}
			else
				printf("X ");
		}

		printf("\n");
	}

}

int find(int i, int k)
{
	int j = 1;
	while (j < i)
	{
		if (q[j] == k || abs(j - i) == abs(q[j] - k))
			return 0;
		j++;
	}
	return 1;
}

void place(int k, int n)
{
	int j;
	if (k > n)
		printqueen(n);
	else
	{
		for (j = 1; j <= n; j++)
		{
			if (find(k, j))
			{
				q[k] = j;
				place(k + 1, n);
			}
		}
	}
}

PUBLIC void queen(int fd_stdin, int fd_stdout)
{

	int i; char buqe[128] = { '\0' };
	printf("Input the value\n");

	//printf( "input\n");
	i = read(0, buqe, 128);
	n = getNum(buqe);
	//  printf("n is %d\n", n);
	place(1, n);
	printf("result is %d\n", amount);

	return 0;
}

int abs(int a)
{
	if (a >= 0)
		return a;
	if (a<0)
		return -a;
}
/////////////////////////////////////


PUBLIC void Guess(int fd_stdin, int fd_stdout) {
	// Specify the rules of the game
	int i = 0;
	printf("Welcome to NUM Game!Input a number!\n");
	srandnum(1231);
	goal = randnum() % 100;
	//  printf("value %d\n",goal);

	while (1)
	{
		i = read(0, bufr, 128);
		rel = getNum(bufr);
		if (rel == goal)
		{
			printf("you win!\n"); break;
		}
		if (rel>goal)
			printf("big!\n");
		if (rel<goal)
			printf("small!\n");
	}

}


int getNum(char*bufr)
{
	int ten = 1, i = 0, res = 0;
	for (i = 0; i<strlen(bufr) - 1; i++)
		ten *= 10;
	for (i = 0; i<strlen(bufr); i++)
	{
		res += (bufr[i] - '0')*ten;
		ten /= 10;
	}
	return res;

}
