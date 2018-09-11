#include "stdio.h"
#define chartonumber(x) (x-'0')
#define width 15
#define height 15
#define me 1
#define enemy 2
short Table[width][height];
short Computer[width][height][4];
short Player[width][height][4];

int chess_main();
int cal_main();
void select_point();
void main_function();
int  AI_player();
int two_players();
int game_start();
int win();
void print();
int getNum(char * bufr);
int _getNum(char * bufr);

int main()
{
        char bufr[128]={0};

	printf("    Mini Program    \n");
	flag1:
	printf("Input'1' calculator ,input'2' chess ,input'q' quit\n");
	//scanf_s("%d", &a);
	read(0, bufr, 128);
	if (bufr[0] == '1')
		cal_main();
	else if (bufr[0] == '2')
		chess_main();
	else if (bufr[0] == 'q') {
		memset(bufr, 0, 100);
		return 0;
	}
	else
	{
		printf("Input '1'or'2'or'q'\n");
		goto flag1;
	}
        
}

int chess_main()
{
	int i = 0, j = 0;
        for(i = 0; i < 15; i++)
                for(j = 0; j < 15; j++)
                        Table[i][j] = 0;
        game_start();
	return 0;
}

void print()
{
	printf("*********************************\n");
	printf("   0 1 2 3 4 5 6 7 8 9 1011121314\n");
	for (int i = 0; i<width; i++)
	{
		if (i<10)
			printf("%d  ", i);
		else
			printf("%d ", i);
		for (int j = 0; j<height; j++)
		{
			if (Table[i][j] == 1)
			{
				printf("@ ");
			}
			else if (Table[i][j] == 2)
			{
				printf("0 ");
			}
			else
				printf("+ ");
		}
		printf("\n");
	}
}

int win()
{
	for (int i = 0; i<width; i++) {
		for (int j = 0; j<height; j++) {
			for(int k = 0; k < 4; k++) {
				if (Computer[i][j][k] == 5)
					return 1;
				if (Player[i][j][k] == 5)
					return 2;
			}
		}
	}
	return 0;
}

int game_start()
{
	char bufr[128]={0};

	printf("    Welcome    \n");
	flag1:
	printf("Input'1' pve ,input'2' pvp ,input'q' quit\n");
	//scanf_s("%d", &a);
	read(0, bufr, 128);
	if (bufr[0] == '1')
		AI_player();
	else if (bufr[0] == '2')
		two_players();
	else if (bufr[0] == 'q') {
		memset(bufr, 0, 100);
		return 0;
	}
	else
	{
		printf("Input '1'or'2'or'q'\n");
		goto flag1;
	}
	memset(bufr, 0, 100);
}

int two_players()
{
	char bufr[128]={0};
        int x = 0, y = 0;
	while (1) {
		main_function();
		if (win() == 2)
		{
			printf("player2win");
			memset(bufr, 0, 100);
			return 0;
		}
		print();
	flag3:
		printf("P1row:");
		//scanf_s("%d", &x);
		read(0, bufr, 128);
		if (bufr[0] == 'q') {
			memset(bufr, 0, 100);
			return 0;
		}
		x = getNum(bufr);
		printf("P1col:");
		//scanf_s("%d", &y);
		read(0, bufr, 128);
		if (bufr[0] == 'q') {
			memset(bufr, 0, 100);
			return 0;
		}
		y = getNum(bufr);
		if (x >= 0 && x <= 14 && y >= 0 && y <= 14 && Table[x][y] == 0) {
			Table[x][y] = me;
		}
		else {
			printf("Error,input again.\n");
			goto flag3;
		}
		main_function();
		print();
		if (win() == 1)
		{
			printf("player1win");
			memset(bufr, 0, 100);
			return 0;
		}
	flag4:
		printf("P2row:");
		//scanf_s("%d", &x);
		read(0, bufr, 128);
		if (bufr[0] == 'q') {
			memset(bufr, 0, 100);
			return 0;
		}
		x = getNum(bufr);
		printf("P2col:");
		//scanf_s("%d", &y);
		read(0, bufr, 128);
		if (bufr[0] == 'q') {
			memset(bufr, 0, 100);
			return 0;
		}
		y = getNum(bufr);
		if (x >= 0 && x <= 14 && y >= 0 && y <= 14 && Table[x][y] == 0) {
			Table[x][y] = enemy;
		}
		else {
			printf("Error,input again.\n");
			goto flag4;
		}
		main_function();
		print();
		if (win() == 2)
		{
			printf("player2win");
			memset(bufr, 0, 100);
			return 0;
		}

	}
	memset(bufr, 0, 100);
	return 0;
}

int AI_player()
{
	char bufr[128]={0};
        int x = 0, y = 0;
	while (1)
	{
		main_function();
		if (win() == 2)
		{
			printf("Player Win!");
			memset(bufr, 0, 100);
			return 0;
		}
		select_point();
		print();
		main_function();
		if (win() == 1)
		{
			printf("Computer Win!");
			memset(bufr, 0, 100);
			return 0;
		}
	flag2:
		printf("Player row:");
		//scanf_s("%d", &x);
		read(0, bufr, 128);
		if (bufr[0] == 'q') {
			memset(bufr, 0, 100);
			return 0;
		}
		x = getNum(bufr);
                //printf("Pbcol=%c",bufr[0]);
                //printf("Pxcol=%d",x);
		printf("Player col:");
		//scanf_s("%d", &y);
		read(0, bufr, 128);
		if (bufr[0] == 'q') {
			memset(bufr, 0, 100);
			return 0;
		}
		y = getNum(bufr);
                //printf("Pbrow=%c",bufr[0]);
                //printf("Pyrow=%d",y);
                
		if (x >= 0 && x <= 14 && y >= 0 && y <= 14 && Table[x][y] == 0) {
			Table[x][y] = enemy;
		}
		else {
			printf("Error,input again.\n");
			goto flag2;
		}
	}
}

void main_function()
{
	int now = 0;
	int last = 0;
	for (int i = 0; i<width; i++)
		for (int j = 0; j<height; j++)
			for (int a = 0; a<4; a++)
			{
				Computer[i][j][a] = 0;
				Player[i][j][a] = 0;
			}
	for (int i = 0; i<width; i++)
	{
		for (int j = 0; j<height; j++)
		{
			if (Table[i][j] == 0)
			{
				/*
				
				*/
				//
				now = 0;
				last = 0;
				int left = j - 1;
				for (; left >= 0; left--)
				{
					if (Table[i][left] == me)
					{
						now = me;
						if (left != j - 1)
						{
							if (now != last) break;
						}
						Computer[i][j][0]++;
						last = me;
					}
					else if (Table[i][left] == enemy)
					{
						now = enemy;
						if (left != j - 1)
						{
							if (now != last) break;
						}
						Player[i][j][0]++;
						last = enemy;
					}
					else break;
				}
				//right
				now = 0;
				last = 0;
				int right = j + 1;
				for (; right<width; right++)
				{
					if (Table[i][right] == me)
					{
						now = me;
						if (right != j + 1)
						{
							if (now != last) break;
						}
						Computer[i][j][0]++;
						last = me;
					}
					else if (Table[i][right] == enemy)
					{
						now = enemy;
						if (right != j + 1)
						{
							if (now != last) break;
						}
						Player[i][j][0]++;
						last = enemy;
					}
					else break;
				}
				/********************************/
				/*
				* 
				*/
				//up
				int up = i - 1;
				now = 0;
				last = 0;
				for (; up >= 0; up--)
				{
					if (Table[up][j] == me)
					{
						now = me;
						if (up != i - 1)
						{
							if (now != last) break;
						}
						Computer[i][j][1]++;
						last = me;
					}
					else if (Table[up][j] == enemy)
					{
						now = enemy;
						if (up != i - 1)
						{
							if (now != last) break;
						}
						Player[i][j][1]++;
						last = enemy;
					}
					else break;
				}
				int down = i + 1;
				now = 0;
				last = 0;
				for (; down<height; down++)
				{
					if (Table[down][j] == me)
					{
						now = me;
						if (down != i + 1)
						{
							if (now != last) break;
						}
						Computer[i][j][1]++;
						last = me;
					}
					else if (Table[down][j] == enemy)
					{
						now = enemy;
						if (down != i + 1)
						{
							if (now != last) break;
						}
						Player[i][j][1]++;
						last = enemy;
					}
					else break;
				}
				/*******************************/
				right = j + 1;
				up = i - 1;
				now = 0;
				last = 0;
				while (right<width&&up >= 0)
				{
					if (Table[up][right] == me)
					{
						now = me;
						if (right != j + 1)
						{
							if (now != last) break;
						}
						Computer[i][j][2]++;
						last = me;
					}
					else if (Table[up][right] == enemy)
					{
						now = enemy;
						if (right != j + 1)
						{
							if (now != last) break;
						}
						Player[i][j][2]++;
						last = enemy;
					}
					else break;
					right++;
					up--;
				}
                                //
				left = j - 1;
				down = i + 1;
				now = 0;
				last = 0;
				while (left >= 0 && down<height)
				{
					if (Table[down][left] == me)
					{
						now = me;
						if (left != j - 1)
						{
							if (now != last) break;
						}
						Computer[i][j][2]++;
						last = me;
					}
					else if (Table[down][left] == enemy)
					{
						now = enemy;
						if (left != j - 1)
						{
							if (now != last) break;
						}
						Player[i][j][2]++;
						last = enemy;
					}
					else break;
					left--;
					down++;
				}
				/**********************************/
				/*
				
				*/
				//
				left = j - 1;
				up = i - 1;
				now = 0;
				last = 0;
				while (left >= 0 && up >= 0)
				{
					if (Table[up][left] == me)
					{
						now = me;
						if (left != j - 1)
						{
							if (now != last) break;
						}
						Computer[i][j][3]++;
						last = me;
					}
					else if (Table[up][left] == enemy)
					{
						now = enemy;
						if (left != j - 1)
						{
							if (now != last) break;
						}
						Player[i][j][3]++;
						last = enemy;
					}
					else break;
					left--;
					up--;
				}
				//
				right = j + 1;
				down = i + 1;
				now = 0;
				last = 0;
				while (right<width&&down<height)
				{
					if (Table[down][right] == me)
					{
						now = me;
						if (right != j + 1)
						{
							if (now != last) break;
						}
						Computer[i][j][3]++;
						last = me;
					}
					else if (Table[down][right] == enemy)
					{
						now = enemy;
						if (right != j + 1)
						{
							if (now != last) break;
						}
						Player[i][j][3]++;
						last = enemy;
					}
					else break;
					right++;
					down++;
				}
			}
		}
	}
}

void select_point()
{
	int max = 0;
	int i, j;
	int x = 7, y = 7;
	int temp;
	for (i = 0; i<width; i++)
	{
		for (j = 0; j<height; j++)
		{
			temp = Computer[i][j][0] + Computer[i][j][1] + Computer[i][j][2] + Computer[i][j][3];
			if (temp>max)
			{
				x = i;
				y = j;
				max = temp;
			}
			temp = Player[i][j][0] + Player[i][j][1] + Player[i][j][2] + Player[i][j][3];
			if (temp>max)
			{
				x = i; y = j;
				max = temp;
			}

			temp = Computer[i][j][0];

			if (temp>max)
			{
				max = temp;
				x = i;
				y = j;
			}
			if (temp == 4)
			{
				x = i; y = j;
				printf("x=%d,y=%d\n", x, y);
				Table[x][y] = me;
				return;
			}
			temp = Computer[i][j][1];
			if (temp>max)
			{
				max = temp;
				x = i;
				y = j;
			}
			if (temp == 4)
			{
				x = i; y = j;
				printf("x=%d,y=%d\n", x, y);
				Table[x][y] = me;
				return;
			}
			temp = Computer[i][j][2];
			if (temp>max)
			{
				max = temp;
				x = i;
				y = j;
			}
			if (temp == 4)
			{
				x = i; y = j;
				printf("x=%d,y=%d\n", x, y);
				Table[x][y] = me;
				return;
			}
			temp = Computer[i][j][3];
			if (temp>max)
			{
				max = temp;
				x = i;
				y = j;
			}
			if (temp == 4)
			{
				x = i; y = j;
				printf("x=%d,y=%d\n", x, y);
				Table[x][y] = me;
				return;
			}

			temp = Player[i][j][0];
			if (temp>max)
			{
				max = temp;
				x = i;
				y = j;
			}
			if (temp == 4)
			{
				x = i;
				y = j;
				printf("x=%d,y=%d\n", x, y);
				Table[x][y] = me;
				return;
			}
			temp = Player[i][j][1];
			if (temp>max)
			{
				max = temp;
				x = i;
				y = j;
			}
			if (temp == 4)
			{
				x = i;
				y = j;
				printf("x=%d,y=%d\n", x, y);
				Table[x][y] = me;
				return;
			}
			temp = Player[i][j][2];
			if (temp>max)
			{
				max = temp;
				x = i;
				y = j;
			}
			if (temp == 4)
			{
				x = i;
				y = j;
				printf("x=%d,y=%d\n", x, y);
				Table[x][y] = me;
				return;
			}
			temp = Player[i][j][3];
			if (temp>max)
			{
				max = temp;
				x = i;
				y = j;
			}
			if (temp == 4)
			{
				x = i;
				y = j;
				printf("x=%d,y=%d\n", x, y);
				Table[x][y] = me;
				return;
			}
		}
	}
	for (i = 0; i<width; i++)
	{
		for (j = 0; j<height; j++)
		{
			temp = Computer[i][j][0];
			if (temp == 3)
			{
				if (j - 1 >= 0 && j + 4<width&&Table[i][j - 1] == 0 && Table[i][j + 1] == me && Table[i][j + 2] == me && Table[i][j + 3] == me && Table[i][j + 4] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (j + 1<width&&j - 4 >= 0 && Table[i][j + 1] == 0 && Table[i][j - 1] == me && Table[i][j - 2] == me && Table[i][j - 3] == me && Table[i][j - 4] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (j + 2<width&&j - 3 >= 0 && Table[i][j - 1] == me && Table[i][j - 2] == me && Table[i][j - 3] == 0 && Table[i][j + 1] == me && Table[i][j + 2] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (j + 3<width&&j - 2 >= 0 && Table[i][j + 1] == me && Table[i][j + 2] == me && Table[i][j + 3] == 0 && Table[i][j - 1] == me && Table[i][j - 2] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
			}
			temp = Computer[i][j][1];
			if (temp == 3)
			{
				if (i - 1 >= 0 && i + 4<height&&Table[i - 1][j] == 0 && Table[i + 1][j] == me && Table[i + 2][j] == me && Table[i + 3][j] == me && Table[i + 4][j] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i + 1<height&&i - 4 >= 0 && Table[i + 1][j] == 0 && Table[i - 1][j] == me && Table[i - 2][j] == me && Table[i - 3][j] == me && Table[i - 4][j] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i + 2<height&&i - 3 >= 0 && Table[i - 1][j] == me && Table[i - 2][j] == me && Table[i - 3][j] == 0 && Table[i + 1][j] == me && Table[i + 2][j] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i + 3<height&&i - 2 >= 0 && Table[i + 1][j] == me && Table[i + 2][j] == me && Table[i + 3][j] == 0 && Table[i - 1][j] == me && Table[i - 2][j] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
			}
			temp = Computer[i][j][2];
			if (temp == 3)
			{
				if (i + 4<height&&i - 1 >= 0 && j + 1<width&&j - 4 >= 0 && Table[i - 1][j + 1] == 0 && Table[i + 1][j - 1] == me && Table[i + 2][j - 2] == me && Table[i + 3][j - 3] == me && Table[i + 4][j - 4] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i - 4 >= 0 && i + 1<height&&j + 4<width&&j - 1 >= 0 && Table[i + 1][j - 1] == 0 && Table[i - 1][j + 1] == me && Table[i - 2][j + 2] == me && Table[i - 3][j + 3] == me && Table[i - 4][j + 4] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i + 2<height&&i - 3 >= 0 && j + 3<width&&j - 2 >= 0 && Table[i + 2][j - 2] == 0 && Table[i + 1][j - 1] == me && Table[i - 1][j + 1] == me && Table[i - 2][j + 2] == me && Table[i - 3][j + 3] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i + 3<height&&i - 2 >= 0 && j - 3 >= 0 && j + 2<width&&Table[i + 3][j - 3] == 0 && Table[i + 2][j - 2] == me && Table[i + 1][j - 1] == me && Table[i - 1][j + 1] == me && Table[i - 2][j + 2] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}

			}

			temp = Computer[i][j][3];
			if (temp == 3)
			{
				if (i + 4<height&&i - 1 >= 0 && j + 4<width&&j - 1 >= 0 && Table[i - 1][j - 1] == 0 && Table[i + 1][j + 1] == me && Table[i + 2][j + 2] == me && Table[i + 3][j + 3] == me && Table[i + 4][j + 4] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i - 4 >= 0 && i + 1<height&&j + 1<width&&j - 4 >= 0 && Table[i + 1][j + 1] == 0 && Table[i - 1][j - 1] == me && Table[i - 2][j - 2] == me && Table[i - 3][j - 3] == me && Table[i - 4][j - 4] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i + 2<height&&i - 3 >= 0 && j + 2<width&&j - 3 >= 0 && Table[i + 2][j + 2] == 0 && Table[i + 1][j + 1] == me && Table[i - 1][j - 1] == me && Table[i - 2][j - 2] == me && Table[i - 3][j - 3] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i + 3<height&&i - 2 >= 0 && j - 2 >= 0 && j + 3<width&&Table[i + 3][j + 3] == 0 && Table[i + 2][j + 2] == me && Table[i + 1][j + 1] == me && Table[i - 1][j - 1] == me && Table[i - 2][j - 2] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
			}

			temp = Player[i][j][0];
			if (temp == 3)
			{
				if (j - 1 >= 0 && j + 4<width&&Table[i][j - 1] == 0 && Table[i][j + 1] == enemy && Table[i][j + 2] == enemy && Table[i][j + 3] == enemy && Table[i][j + 4] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (j + 1<width&&j - 4 >= 0 && Table[i][j + 1] == 0 && Table[i][j - 1] == enemy && Table[i][j - 2] == enemy && Table[i][j - 3] == enemy && Table[i][j - 4] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (j + 2<width&&j - 3 >= 0 && Table[i][j - 1] == enemy && Table[i][j - 2] == enemy && Table[i][j - 3] == 0 && Table[i][j + 1] == enemy && Table[i][j + 2] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (j + 3<width&&j - 2 >= 0 && Table[i][j + 1] == enemy && Table[i][j + 2] == enemy && Table[i][j + 3] == 0 && Table[i][j - 1] == enemy && Table[i][j - 2] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
			}
			temp = Player[i][j][1];
			if (temp == 3)
			{
				if (i - 1 >= 0 && i + 4<height&&Table[i - 1][j] == 0 && Table[i + 1][j] == enemy && Table[i + 2][j] == enemy && Table[i + 3][j] == enemy && Table[i + 4][j] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i + 1<height&&i - 4 >= 0 && Table[i + 1][j] == 0 && Table[i - 1][j] == enemy && Table[i - 2][j] == enemy && Table[i - 3][j] == enemy && Table[i - 4][j] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i + 2<height&&i - 3 >= 0 && Table[i - 1][j] == enemy && Table[i - 2][j] == enemy && Table[i - 3][j] == 0 && Table[i + 1][j] == enemy && Table[i + 2][j] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i + 3<height&&i - 2 >= 0 && Table[i + 1][j] == enemy && Table[i + 2][j] == enemy && Table[i + 3][j] == 0 && Table[i - 1][j] == enemy && Table[i - 2][j] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
			}
			temp = Player[i][j][2];
			if (temp == 3)
			{
				if (i + 4<height&&i - 1 >= 0 && j + 1<width&&j - 4 >= 0 && Table[i - 1][j + 1] == 0 && Table[i + 1][j - 1] == enemy && Table[i + 2][j - 2] == enemy && Table[i + 3][j - 3] == enemy && Table[i + 4][j - 4] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i - 4 >= 0 && i + 1<height&&j + 4<width&&j - 1 >= 0 && Table[i + 1][j - 1] == 0 && Table[i - 1][j + 1] == enemy && Table[i - 2][j + 2] == enemy && Table[i - 3][j + 3] == enemy && Table[i - 4][j + 4] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i + 2<height&&i - 3 >= 0 && j + 3<width&&j - 2 >= 0 && Table[i + 2][j - 2] == 0 && Table[i + 1][j - 1] == enemy && Table[i - 1][j + 1] == enemy && Table[i - 2][j + 2] == enemy && Table[i - 3][j + 3] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i + 3<height&&i - 2 >= 0 && j - 3 >= 0 && j + 2<width&&Table[i + 3][j - 3] == 0 && Table[i + 2][j - 2] == enemy && Table[i + 1][j - 1] == enemy && Table[i - 1][j + 1] == enemy && Table[i - 2][j + 2] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}

			}

			temp = Player[i][j][3];
			if (temp == 3)
			{
				if (i + 4<height&&i - 1 >= 0 && j + 4<width&&j - 1 >= 0 && Table[i - 1][j - 1] == 0 && Table[i + 1][j + 1] == enemy && Table[i + 2][j + 2] == enemy && Table[i + 3][j + 3] == enemy && Table[i + 4][j + 4] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i - 4 >= 0 && i + 1<height&&j + 1<width&&j - 4 >= 0 && Table[i + 1][j + 1] == 0 && Table[i - 1][j - 1] == enemy && Table[i - 2][j - 2] == enemy && Table[i - 3][j - 3] == enemy && Table[i - 4][j - 4] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i + 2<height&&i - 3 >= 0 && j + 2<width&&j - 3 >= 0 && Table[i + 2][j + 2] == 0 && Table[i + 1][j + 1] == enemy && Table[i - 1][j - 1] == enemy && Table[i - 2][j - 2] == enemy && Table[i - 3][j - 3] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
				else if (i + 3<height&&i - 2 >= 0 && j - 2 >= 0 && j + 3<width&&Table[i + 3][j + 3] == 0 && Table[i + 2][j + 2] == enemy && Table[i + 1][j + 1] == enemy && Table[i - 1][j - 1] == enemy && Table[i - 2][j - 2] == 0)
				{
					x = i; y = j;
					printf("x=%d,y=%d\n", x, y);
					Table[x][y] = me;
					return;
				}
			}

		}
	}
	printf("x=%d,y=%d\n", x, y);

	Table[x][y] = me;
}

int getNum(char * bufr)
{
	int res = 0;
	if(bufr[1] >= 48 && bufr[1] <= 57)
        {
                 res = (bufr[0] - 48) * 10 + bufr[1] - 48;
        }
        else
        {
                 res = bufr[0] - 48;
        }
	return res;
}


int cal_main()
{
	int i, num1 = 0, num2 = 0, flag = 1, res = 0;
        char bufr[128]={0};

	printf("===================================================\n");
	printf("=                  Calculator                     =\n");
	printf("=  Input two numbers                              =\n");
	printf("=  Enter q to quit                                =\n");
	printf("===================================================\n\n");

	while(flag == 1){	
		printf("Input num1:");
		i = read(0, bufr, 128);
		if (bufr[0] == 'q')
			break;
		num1 = _getNum(bufr);
		printf("Num1= %d\n", num1);

		printf("Input num2:");
		i = read(0, bufr, 128);	
		if (bufr[0] == 'q')
			break;
		num2 = _getNum(bufr);
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
			case 'q':
				flag = 0;
				break;
			default:
				printf("Error!\n");
		}
        memset(bufr,0,100);
	}
    
	return 0;
}

int _getNum(char * bufr)
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

