#include<iostream>
#include<graphics.h>
#include<conio.h>
#include<deque>
#include<map>
#include<time.h>
using namespace std;

// 引用 Windows Multimedia API
#include<Windows.h>
#pragma comment(lib, "winmm.lib")

// 引用该库才能使用 AlphaBlend 函数
#pragma comment( lib, "MSIMG32.LIB")
void transparentimage(IMAGE*, int , int , IMAGE* );

const auto SPEED = 300; //速度：每SPEED毫秒移动一次

const auto WINDOW_WIDTH = 800;
const auto WINDOW_HEIGHT = 600;
const auto CELL_SIZE = 25; //每个格子的大小
const auto CELL_WIDTH = WINDOW_WIDTH / CELL_SIZE;
const auto CELL_HEIGHT = WINDOW_HEIGHT / CELL_SIZE;
const auto FOOD_KINDS = 8; //食物种类

enum class Dir { UP, DOWN, LEFT, RIGHT };
const auto SNAKE_INIT_DIR = Dir::UP; //初始方向

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

IMAGE imgBg;//背景
IMAGE imgLogo;//logo
IMAGE imgFood[FOOD_KINDS + 1];//食物
IMAGE imgSnakeHead[4];//蛇头
IMAGE imgColor[2];//蛇身

int PlayerNum;//玩家数量
deque <Pos> Snake[2];//蛇的坐标
Dir SnakeDir[2];//蛇的方向
map<Pos, int> mp;//记录格子状态
Pos Food;//食物坐标
int FoodKind;//食物种类

//程序开始时
void Load_Img();//载入图片
void Game_Start();//游戏开始界面：选择玩家数量、初始化
	void Game_Init();//游戏初始化

//游戏中
void Update();//刷新（绘制）画面
void OnPress();//按键响应，只有刷新前最后一次按键生效
bool Update_Snake();//更新蛇的坐标，接收Collision()的返回值
	bool Eat_Food(int);//食物检测
	void Update_Food();//更新食物坐标
	bool Collision();//碰撞检测：蛇头碰到蛇身
void Game_Over();//游戏结束


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
	Load_Img();//载入图片
	initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);//初始化窗口
	setbkcolor(WHITE);//设置背景颜色
	settextcolor(BLACK);//设置字体颜色
	setfillstyle(BS_NULL);
	Game_Start();//游戏开始界面：选择玩家数量、初始化

	Game_Loop();//游戏循环

	return 0;
}

void Load_Img() {//程序开始时 载入图片
	loadimage(&imgBg, _T("src/Bg.png"));//加载背景
	loadimage(&imgLogo, _T("src/Logo.jpg"));//加载logo
	TCHAR path[256];
	for (int i = 1; i <= FOOD_KINDS; i++) {//加载食物
		_stprintf_s(path, _T("src/food%d.png"), i);
		loadimage(&imgFood[i], path);
	}
	loadimage(&imgSnakeHead[0], _T("src/head_w.png"));//加载蛇头
	loadimage(&imgSnakeHead[1], _T("src/head_s.png"));
	loadimage(&imgSnakeHead[2], _T("src/head_a.png"));
	loadimage(&imgSnakeHead[3], _T("src/head_d.png"));
	loadimage(&imgColor[0], _T("src/color1.png"));//加载蛇身
	loadimage(&imgColor[1], _T("src/color2.png"));
}

void Game_Start() {//游戏开始界面：选择玩家数量
	BeginBatchDraw();
	putimage(0, 0, &imgBg);//绘制背景
	putimage(250, 10, &imgLogo);//绘制logo

	TCHAR mode[100];//选择模式
	_stprintf_s(mode, _T("单人模式"));
	settextstyle(60, 0, _T("Consolas"));
	outtextxy(130, 360, mode);
	_stprintf_s(mode, _T("双人模式"));
	settextstyle(60, 0, _T("Consolas"));
	outtextxy(450, 360, mode);
	_stprintf_s(mode, _T("Player1:WSAD / Player2:↑↓←→"));
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

	Game_Init();//游戏初始化
}

void Game_Init() {//游戏初始化
	SnakeDir[0] = SnakeDir[1] = SNAKE_INIT_DIR;//方向初始化
	Snake[0].clear(); Snake[1].clear();//清空蛇
	Pos pos;
	if (PlayerNum == 1) {//单人模式
		pos.x = CELL_WIDTH / 2;
		pos.y = CELL_HEIGHT / 2;
		Snake[0].push_back(pos);
		for (int i = 0; i < 2; i++) {
			pos.y++;
			Snake[0].push_back(pos);
		}
	}
	else if (PlayerNum == 2) {//双人模式
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
	//生成食物
	srand(time(0));
	Food.x = rand() % CELL_WIDTH;
	Food.y = rand() % CELL_HEIGHT;
	while (mp[Food]) {
		Food.x = rand() % CELL_WIDTH;
		Food.y = rand() % CELL_HEIGHT;
	}mp[Food] = 2;
	FoodKind = rand() % FOOD_KINDS + 1;
}

void Update() {//刷新（绘制）画面
	BeginBatchDraw();
	putimage(0, 0, &imgBg);//绘制背景
	//绘制食物
	transparentimage(NULL, Food.x * CELL_SIZE, Food.y * CELL_SIZE, &imgFood[FoodKind]);
	//绘制蛇
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
	//按键响应
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

bool Update_Snake() {//更新蛇的坐标
	Pos pos;
	int co=0;
	for (int i = 0; i < PlayerNum; i++) {
		if (Eat_Food(i)) {
			Update_Food();//更新食物坐标
		}
		else {//更新蛇尾
			mp[Snake[i].back()] = 0;
			Snake[i].pop_back();
		}
		co = Collision();//碰撞检测

		pos = Snake[i].front();
		pos.move(SnakeDir[i]);

		Snake[i].push_front(pos);
		mp[pos] = 1;
	}
	return co;
}

bool Eat_Food(int i) {//食物检测
	Pos pos = Snake[i].front();
	pos.move(SnakeDir[i]);
	if (pos.x == Food.x && pos.y == Food.y) return true;
	return false;
}

void Update_Food() {//更新食物坐标
	Food.x = rand() % CELL_WIDTH;
	Food.y = rand() % CELL_HEIGHT;
	int cnt = 0;//计数器
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
	//防止游戏后期可能随机不到空白格子
	//如果计数超过10000，搜索空白格子放置食物
	//如果搜索不到空白格子，放弃放置食物
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

bool Collision() {//碰撞检测
	Pos pos;
	for (int i = 0; i < PlayerNum; i++) {
		pos = Snake[i].front();
		pos.move(SnakeDir[i]);
		if (mp[pos] == 1) return true;
	}
	return false;
}

void Game_Over() {//游戏结束：玩家闪烁3秒
	PlaySound(_T("src/gameover.wav"), NULL, SND_FILENAME | SND_ASYNC);
	for (int i = 0; i < 5; i++) {
		BeginBatchDraw();
		putimage(0, 0, &imgBg);//绘制背景
		//绘制食物
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
}// 透明贴图函数（by 慢羊羊）
//	dstimg: 目标 IMAGE 对象指针。NULL 表示默认窗体
//	x, y:	目标贴图位置
//	srcimg: 源 IMAGE 对象指针。NULL 表示默认窗体