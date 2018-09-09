#include <stdio.h>
#define chartonumber(x) (x-'0')
#define width 15
#define height 15
#define me 1
#define enemy 2
short Table[width][height];
short Computer[width][height][4];
short Player[width][height][4];

void select_point();
void main_function();
int  AI_player();
int two_players();
int game_start();
int win();
void print();
int getNum(char * bufr);

int main()
{
	game_start();
	return 0;
}

void print()
{
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
	char bufr[128];
	int a;
	printf("    欢迎来到五子棋    \n");
	flag1:
	printf("输入“1”进行人机对战，输入“2”进行本地双人对战，输入“q”退出：\n");
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
		printf("请输入“1”、“2”或“q”。\n");
		goto flag1;
	}
	memset(bufr, 0, 100);
}

int two_players()
{
	char bufr[128];
	while (1) {
		int x, y;
		main_function();
		if (win() == 2)
		{
			printf("玩家2胜");
			memset(bufr, 0, 100);
			return 0;
		}
		print();
	flag3:
		printf("请玩家1输入纵坐标：");
		//scanf_s("%d", &x);
		read(0, bufr, 128);
		if (bufr[0] == 'q') {
			memset(bufr, 0, 100);
			return 0;
		}
		x = getNum(bufr);
		printf("请玩家1输入横坐标：");
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
			printf("输入坐标已有棋子或不在棋盘范围内，请重新输入：\n");
			goto flag3;
		}
		main_function();
		print();
		if (win() == 1)
		{
			printf("玩家1胜");
			memset(bufr, 0, 100);
			return 0;
		}
	flag4:
		printf("请玩家2输入纵坐标：");
		//scanf_s("%d", &x);
		read(0, bufr, 128);
		if (bufr[0] == 'q') {
			memset(bufr, 0, 100);
			return 0;
		}
		x = getNum(bufr);
		printf("请玩家2输入横坐标：");
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
			printf("输入坐标已有棋子或不在棋盘范围内，请重新输入：\n");
			goto flag4;
		}
		main_function();
		print();
		if (win() == 2)
		{
			printf("玩家2胜");
			memset(bufr, 0, 100);
			return 0;
		}

	}
	memset(bufr, 0, 100);
	return 0;
}

int AI_player()
{
	char bufr[128];
	while (1)
	{
		main_function();
		int x, y;
		if (win() == 2)
		{
			printf("玩家胜");
			memset(bufr, 0, 100);
			return 0;
		}
		select_point();
		print();
		main_function();
		if (win() == 1)
		{
			printf("电脑胜");
			memset(bufr, 0, 100);
			return 0;
		}
	flag2:
		printf("请玩家输入纵坐标：");
		//scanf_s("%d", &x);
		read(0, bufr, 128);
		if (bufr[0] == 'q') {
			memset(bufr, 0, 100);
			return 0;
		}
		x = getNum(bufr);
		printf("请玩家输入横坐标：");
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
			printf("输入坐标已有棋子或不在棋盘范围内，请重新输入：\n");
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
				* 横向
				*/
				//left
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
				* 纵向
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
				/*
				* 左斜
				*/
				//左上
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
				//左下
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
				* 右斜
				*/
				//右上
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
				//右下
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
