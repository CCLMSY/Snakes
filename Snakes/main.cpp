#include<iostream>
#include<graphics.h>
#include<conio.h>
#include<deque>
#include<map>
#include<time.h>
using namespace std;

// ���� Windows Multimedia API
#include<Windows.h>
#pragma comment(lib, "winmm.lib")

// ���øÿ����ʹ�� AlphaBlend ����
#pragma comment( lib, "MSIMG32.LIB")
void transparentimage(IMAGE*, int , int , IMAGE* );

const auto SPEED = 300; //�ٶȣ�ÿSPEED�����ƶ�һ��

const auto WINDOW_WIDTH = 800;
const auto WINDOW_HEIGHT = 600;
const auto CELL_SIZE = 25; //ÿ�����ӵĴ�С
const auto CELL_WIDTH = WINDOW_WIDTH / CELL_SIZE;
const auto CELL_HEIGHT = WINDOW_HEIGHT / CELL_SIZE;
const auto FOOD_KINDS = 8; //ʳ������

enum class Dir { UP, DOWN, LEFT, RIGHT };
const auto SNAKE_INIT_DIR = Dir::UP; //��ʼ����

typedef struct _Pos {
	int x, y;
	void move(Dir dir) {
		switch (dir) {
		case Dir::UP: y--; break;
		case Dir::DOWN: y++; break;
		case Dir::LEFT: x--; break;
		case Dir::RIGHT: x++; break;
		}
		if (x < 0) x += CELL_WIDTH;
		if (x >= CELL_WIDTH) x -= CELL_WIDTH;
		if (y < 0) y += CELL_HEIGHT;
		if (y >= CELL_HEIGHT) y -= CELL_HEIGHT;
	}
	bool operator < (const _Pos& p) const {
		if (x == p.x) return y < p.y;
		return x < p.x;
	}
} Pos;

IMAGE imgBg;//����
IMAGE imgLogo;//logo
IMAGE imgFood[FOOD_KINDS + 1];//ʳ��
IMAGE imgSnakeHead[4];//��ͷ
IMAGE imgColor[2];//����

int PlayerNum;//�������
deque <Pos> Snake[2];//�ߵ�����
Dir SnakeDir[2];//�ߵķ���
map<Pos, int> mp;//��¼����״̬
Pos Food;//ʳ������
int FoodKind;//ʳ������

//����ʼʱ
void Load_Img();//����ͼƬ
void Game_Start();//��Ϸ��ʼ���棺ѡ�������������ʼ��
	void Game_Init();//��Ϸ��ʼ��

//��Ϸ��
void Update();//ˢ�£����ƣ�����
void OnPress();//������Ӧ��ֻ��ˢ��ǰ���һ�ΰ�����Ч
bool Update_Snake();//�����ߵ����꣬����Collision()�ķ���ֵ
	bool Eat_Food(int);//ʳ����
	void Update_Food();//����ʳ������
	bool Collision();//��ײ��⣺��ͷ��������
void Game_Over();//��Ϸ����


void Game_Loop() {
	int t;
	while (1) {
		Update();
		Sleep(SPEED);
		OnPress();
		if(t=Update_Snake()){
			Update();
			Game_Over();
			break;
		}
	}
}

int main() {
	Load_Img();//����ͼƬ
	initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);//��ʼ������
	setbkcolor(WHITE);//���ñ�����ɫ
	settextcolor(BLACK);//����������ɫ
	setfillstyle(BS_NULL);
	Game_Start();//��Ϸ��ʼ���棺ѡ�������������ʼ��

	Game_Loop();//��Ϸѭ��

	return 0;
}

void Load_Img() {//����ʼʱ ����ͼƬ
	loadimage(&imgBg, _T("src/Bg.png"));//���ر���
	loadimage(&imgLogo, _T("src/Logo.jpg"));//����logo
	TCHAR path[256];
	for (int i = 1; i <= FOOD_KINDS; i++) {//����ʳ��
		_stprintf_s(path, _T("src/food%d.png"), i);
		loadimage(&imgFood[i], path);
	}
	loadimage(&imgSnakeHead[0], _T("src/head_w.png"));//������ͷ
	loadimage(&imgSnakeHead[1], _T("src/head_s.png"));
	loadimage(&imgSnakeHead[2], _T("src/head_a.png"));
	loadimage(&imgSnakeHead[3], _T("src/head_d.png"));
	loadimage(&imgColor[0], _T("src/color1.png"));//��������
	loadimage(&imgColor[1], _T("src/color2.png"));
}

void Game_Start() {//��Ϸ��ʼ���棺ѡ���������
	BeginBatchDraw();
	putimage(0, 0, &imgBg);//���Ʊ���
	putimage(250, 10, &imgLogo);//����logo

	TCHAR mode[100];//ѡ��ģʽ
	_stprintf_s(mode, _T("����ģʽ"));
	settextstyle(60, 0, _T("Consolas"));
	outtextxy(130, 360, mode);
	_stprintf_s(mode, _T("˫��ģʽ"));
	settextstyle(60, 0, _T("Consolas"));
	outtextxy(450, 360, mode);
	_stprintf_s(mode, _T("Player1:WSAD / Player2:��������"));
	settextstyle(50, 0, _T("Consolas"));
	outtextxy(90, 450, mode);

	EndBatchDraw();

	PlaySound(_T("src/bgm.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

	ExMessage msg;
	while (1) {
		peekmessage(&msg);
		if (msg.message == WM_LBUTTONDOWN) {
			if (msg.x >= 130 && msg.x <= 353 && msg.y >= 360 && msg.y <= 420) {
				PlayerNum = 1;
				break;
			}
			else if (msg.x >= 450 && msg.x <= 673 && msg.y >= 360 && msg.y <= 420) {
				PlayerNum = 2;
				break;
			}
		}
	}

	Game_Init();//��Ϸ��ʼ��
}

void Game_Init() {//��Ϸ��ʼ��
	SnakeDir[0] = SnakeDir[1] = SNAKE_INIT_DIR;//�����ʼ��
	Snake[0].clear(); Snake[1].clear();//�����
	Pos pos;
	if (PlayerNum == 1) {//����ģʽ
		pos.x = CELL_WIDTH / 2;
		pos.y = CELL_HEIGHT / 2;
		Snake[0].push_back(pos);
		for (int i = 0; i < 2; i++) {
			pos.y++;
			Snake[0].push_back(pos);
		}
	}
	else if (PlayerNum == 2) {//˫��ģʽ
		pos.x = CELL_WIDTH / 2 - 1;
		pos.y = CELL_HEIGHT / 2;
		Snake[0].push_back(pos);mp[pos] = 1;
		pos.y++; Snake[0].push_back(pos); mp[pos] = 1;
		pos.y++; Snake[0].push_back(pos); mp[pos] = 1;

		pos.x = CELL_WIDTH / 2 + 1;
		pos.y = CELL_HEIGHT / 2;
		Snake[1].push_back(pos); mp[pos] = 1;
		pos.y++; Snake[1].push_back(pos); mp[pos] = 1;
		pos.y++; Snake[1].push_back(pos); mp[pos] = 1;
	}
	//����ʳ��
	srand(time(0));
	Food.x = rand() % CELL_WIDTH;
	Food.y = rand() % CELL_HEIGHT;
	while (mp[Food]) {
		Food.x = rand() % CELL_WIDTH;
		Food.y = rand() % CELL_HEIGHT;
	}mp[Food] = 2;
	FoodKind = rand() % FOOD_KINDS + 1;
}

void Update() {//ˢ�£����ƣ�����
	BeginBatchDraw();
	putimage(0, 0, &imgBg);//���Ʊ���
	//����ʳ��
	transparentimage(NULL, Food.x * CELL_SIZE, Food.y * CELL_SIZE, &imgFood[FoodKind]);
	//������
	for (int i = 0; i < PlayerNum; i++) {
		Pos pos;
		for (int j = 1; j < Snake[i].size(); j++) {
			pos = Snake[i][j];
			transparentimage(NULL, pos.x * CELL_SIZE, pos.y * CELL_SIZE, &imgColor[i]);
		}
		pos = Snake[i].front();
		transparentimage(NULL, pos.x* CELL_SIZE, pos.y* CELL_SIZE, &imgSnakeHead[(int)SnakeDir[i]]);
	}
	EndBatchDraw();
}

void OnPress() {
	//������Ӧ
	ExMessage msg;
	Dir OriDir[2] = { SnakeDir[0] , SnakeDir[1] };
	while (1) {
		if(peekmessage(&msg)){
			if (msg.message == WM_KEYDOWN) {
				switch (msg.vkcode) {
				case VK_UP:
					if (PlayerNum > 1 && OriDir[1] != Dir::DOWN) SnakeDir[1] = Dir::UP;
					break;
				case VK_DOWN:
					if (PlayerNum > 1 && OriDir[1] != Dir::UP) SnakeDir[1] = Dir::DOWN;
					break;
				case VK_LEFT:
					if (PlayerNum > 1 && OriDir[1] != Dir::RIGHT) SnakeDir[1] = Dir::LEFT;
					break;
				case VK_RIGHT:
					if (PlayerNum > 1 && OriDir[1] != Dir::LEFT) SnakeDir[1] = Dir::RIGHT;
					break;
				case 'W':
					if (OriDir[0] != Dir::DOWN) SnakeDir[0] = Dir::UP;
					break;
				case 'S':
					if (OriDir[0] != Dir::UP)  SnakeDir[0] = Dir::DOWN;
					break;
				case 'A':
					if (OriDir[0] != Dir::RIGHT) SnakeDir[0] = Dir::LEFT;
					break;
				case 'D':
					if (OriDir[0] != Dir::LEFT) SnakeDir[0] = Dir::RIGHT;
					break; 
				}
			}
		}else break;
	}
}

bool Update_Snake() {//�����ߵ�����
	Pos pos;
	int co=0;
	for (int i = 0; i < PlayerNum; i++) {
		if (Eat_Food(i)) {
			Update_Food();//����ʳ������
		}
		else {//������β
			mp[Snake[i].back()] = 0;
			Snake[i].pop_back();
		}
		co = Collision();//��ײ���

		pos = Snake[i].front();
		pos.move(SnakeDir[i]);

		Snake[i].push_front(pos);
		mp[pos] = 1;
	}
	return co;
}

bool Eat_Food(int i) {//ʳ����
	Pos pos = Snake[i].front();
	pos.move(SnakeDir[i]);
	if (pos.x == Food.x && pos.y == Food.y) return true;
	return false;
}

void Update_Food() {//����ʳ������
	Food.x = rand() % CELL_WIDTH;
	Food.y = rand() % CELL_HEIGHT;
	int cnt = 0;//������
	while (mp[Food]&&cnt<1000) {
		Food.x = rand() % CELL_WIDTH;
		Food.y = rand() % CELL_HEIGHT;
		cnt++;
	}
	if (cnt < 10000) {
		mp[Food] = 2;
		FoodKind = rand() % FOOD_KINDS + 1;
		return ;
	}
	else {
	//��ֹ��Ϸ���ڿ�����������հ׸���
	//�����������10000�������հ׸��ӷ���ʳ��
	//������������հ׸��ӣ���������ʳ��
		for (int i = 0; i < CELL_WIDTH; i++) {
			for (int j = 0; j < CELL_HEIGHT; j++) {
				Food.x = i;
				Food.y = j;
				if (!mp[Food]) {
					mp[Food] = 2;
					FoodKind = rand() % FOOD_KINDS + 1;
					return;
				}
			}
		}
	}
}

bool Collision() {//��ײ���
	Pos pos;
	for (int i = 0; i < PlayerNum; i++) {
		pos = Snake[i].front();
		pos.move(SnakeDir[i]);
		if (mp[pos] == 1) return true;
	}
	return false;
}

void Game_Over() {//��Ϸ�����������˸3��
	PlaySound(_T("src/gameover.wav"), NULL, SND_FILENAME | SND_ASYNC);
	for (int i = 0; i < 5; i++) {
		BeginBatchDraw();
		putimage(0, 0, &imgBg);//���Ʊ���
		//����ʳ��
		transparentimage(NULL, Food.x * CELL_SIZE, Food.y * CELL_SIZE, &imgFood[FoodKind]);
		EndBatchDraw();
		Sleep(300);
		Update();
		Sleep(300);
	}
}

void transparentimage(IMAGE* dstimg, int x, int y, IMAGE* srcimg)
{
	HDC dstDC = GetImageHDC(dstimg);
	HDC srcDC = GetImageHDC(srcimg);
	int w = srcimg->getwidth();
	int h = srcimg->getheight();
	BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	AlphaBlend(dstDC, x, y, w, h, srcDC, 0, 0, w, h, bf);
}// ͸����ͼ������by ������
//	dstimg: Ŀ�� IMAGE ����ָ�롣NULL ��ʾĬ�ϴ���
//	x, y:	Ŀ����ͼλ��
//	srcimg: Դ IMAGE ����ָ�롣NULL ��ʾĬ�ϴ���