#include<graphics.h>//在编译前请安装easyx函数库
#include<sstream>
#include<time.h>
#include<fstream>
#include<iostream>
#include<vector>
#include<conio.h>
#include<math.h>
#include<algorithm>
#include<Windows.h>
using namespace std;
const int MAX_BUFFER_LEN = 500;

MOUSEMSG mouse;

COLORREF color_list[9] = { WHITE,BLUE, GREEN,RGB(255,140,0),RGB(255,192,203),BROWN,RED,RGB(148,0,211),BLACK };
//0:白色，1：蓝色，2：绿色，3：橙色，4：粉色，5：棕色，6：红色，7：紫色，8：黑色

void color_text(short x)	//自定义函根据参数改变调试控制台的输出文本的颜色
{
	if (x >= 0 && x <= 256)//参数在0-15的范围颜色
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), x);	//只有一个参数 
	else//默认的颜色白色
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}

struct cell_information {//地图中单个格子的信息
	bool boom_exist;//该格子上是否有患者
	bool footprint;//该格子是否被点击过
	bool flag;//是否被隔离
	short boom_num;//周围的炸弹数
};
vector <vector <cell_information> > map;//注册二维动态数组作为地图
//x->column->横轴，y->row->竖轴

struct record_log {//用来存储记录数据
	int time;
	string difficulty;
	short x, y, boom;
};
vector <record_log> record_list;

struct button_log {//按钮注册函数
	short left, top, right, bottom;//按钮的位置
	short x, y;//文字的位置
	short ellipsewidth, ellipseheight;//（如有）圆角宽高度
	TCHAR mes[];//文字内容
};

const short refresh_MainInterface = 1;//界面刷新用的常量
void refresh_page(short choose) {//界面刷新函数
	if (choose == refresh_MainInterface) {
		for (short i = 0; i < 800; i++) {
			setlinecolor(RGB(0, 0, i * 255 / 800));
			line(0, i, 500, i);
		}
	}
}

bool mouselocation_button(MOUSEMSG& mse, button_log& button) {//用以检测光标是否落于某个按钮上
	if (mse.x >= button.left and mse.x <= button.right and mse.y >= button.top and mse.y <= button.bottom) {
		return true;
	}
	else {
		return false;
	}
}

void record_sort(vector <record_log> & rec) {
	for (short i = 0; i < rec.size();i++) {
		bool flag = true;
		for (short j = 0; j < rec.size() - i - 1; j++) {
			if (rec[j].time > rec[j + 1].time) {
				flag = false;
				record_log n;
				n = rec[j];
				rec[j] = rec[j + 1];
				rec[j + 1] = n;
			}
		}
		if (flag) {
			return;
		}
	}
}

void paint_button(button_log& button) {//用以绘制按钮的函数
	fillroundrect(button.left, button.top, button.right, button.bottom, button.ellipsewidth, button.ellipseheight);
	outtextxy(button.x, button.y, button.mes);
}

void mine_log(vector <short>& ini_x, vector <short>& ini_y, short row, short column, short x, short y, short& ran) {
	//初始化地图时在周围环绕布雷
	short I = 1000;short J = 1000;
	for (short i = row - 1; i <= row + 1; i++) {
		for (short j = column - 1; j <= column + 1; j++) {
			if (i >= 0 and i < y and j >= 0 and j < x and (j != column or i != row)) {
				bool flag = true;
				for (short o = 0; o < ini_x.size(); o++) {
					if (ini_x[o] == j and ini_y[o] == i) {//检测是否有重复
						flag = false;
						break;
					}
				}
				if (flag) {
					ran--;
					ini_x.push_back(j);
					ini_y.push_back(i);
					I = i;
					J = j;
					if (ran <= 0) {
						return;
					}
				}
			}
		}
	}
	if (I != 1000 and J != 1000) {
		mine_log(ini_x, ini_y, I, J, x, y, ran);
	}
}

void mine_clean(short row, short column, short size, short x, short y) {
	//在讯问后，将周围所有周围没有患者的格子显示出来
	if (map[column][row].flag == false and map[column][row].boom_num == 0 and map[column][row].boom_exist == false) {
		short boom = 0;
		for (short i = row - 1; i <= row + 1; i++) {
			for (short j = column - 1; j <= column + 1; j++) {
				if (i >= 0 and i < y and j >= 0 and j < x and (j != column or i != row)) {
					if (map[j][i].boom_exist == true) {
						boom++;
					}
				}
			}
		}

		setfillcolor(WHITE);//重置被点击的方格
		setbkmode(TRANSPARENT);//输出文本的背景透明
		solidrectangle(size * column, size * row, size * column + size, size * row + size);//指定x，y位置及矩形的长宽然后绘制
		settextcolor(color_list[boom]);//设置文本颜色
		settextstyle(size, 0, _T("黑体"));//设置文本的格式
		char repeater = boom + 48;//将剩余病人数量存入中继器
		outtextxy(size * column, size * row, repeater);//输出文本
		map[column][row].footprint = true;
		map[column][row].boom_num = boom;

		if (boom == 0) {//倘若当前格子周围没有患者，则直接递堆检查其周围的格子
			for (short i = row - 1; i <= row + 1; i++) {
				for (short j = column - 1; j <= column + 1; j++) {
					if (i >= 0 and i < y and j >= 0 and j < x and (j != column or i != row)) {
						if (map[j][i].footprint == false) {
							mine_clean(i, j, size, x, y);
						}
					}
				}
			}
		}
	}
}

void game(string difficulty, short x, short y, short boom_num) {
	map.clear();
	short size = 30;
	srand((unsigned)time(0));

	//设定基准时间
	long ini_time = time(0);

	//根据参数重新注册窗口
	initgraph(x * size, y * size + 2 * size, EW_SHOWCONSOLE);
	BeginBatchDraw();
	setlinecolor(BLUE);
	for (short i = 0; i < x * size; i++) {//绘制底色
		line(i, 0, i, y * size);
	}
	setlinecolor(WHITE);
	for (short i = 0; i < x; i++) {//绘制方格
		line(i * size, 0, i * size, y * size);

		vector <cell_information> repeater;
		for (short j = 0; j < y; j++) {//同时初始化map动态数组
			line(0, j * size, x * size, j * size);
			cell_information n = { false,false,false,0 };
			repeater.push_back(n);
		}
		map.push_back(repeater);
	}
	system("cls");

	//绘制状态栏
	settextcolor(YELLOW);
	settextstyle(size, 0, _T("黑体"));
	outtextxy(0, y * size, _T("时间:"));
	outtextxy(0, y * size + size, _T("患者:"));

	//绘制返回按钮
	setbkcolor(WHITE);
	settextstyle(2 * size, 0, _T("黑体"));
	settextcolor(RGB(148, 0, 211));
	button_log button_exit = { x * size - 150,y * size,x * size,(y + 2) * size,x * size - 150,y * size,30,30,_T("返回") };
	paint_button(button_exit);
	EndBatchDraw();

	vector <short> initial_position_x, initial_position_y;
	while (1) {
		if (MouseHit()) {//第一下点中的及其周围的格子上不能有mine,所以用动态数组存储第一下落点后再随机布雷
			mouse = GetMouseMsg();
			short row = round(mouse.y / size);
			short column = round(mouse.x / size);
			if (mouse.mkLButton and row < y and column < x) {
				initial_position_x.push_back(column);
				initial_position_y.push_back(row);
				short ran = rand() % 3 + 5;
				srand((unsigned)time(0));
				mine_log(initial_position_x, initial_position_y, row, column, x, y, ran);
				//需要多个没有mine的初始落点
				break;
			}
			else if(mouse.mkLButton and mouselocation_button(mouse,button_exit)) {
				return;
			}
		}
		Sleep(20);
	}

	for (short i = 0; i < boom_num; i++) {//随机布雷
		short ran_num = rand() % (x * y);
		short row = ran_num / x;
		short column = ran_num % x;
		if (map[column][row].boom_exist == false) {//回避初始落点
			bool flag = true;
			for (short j = 0; j < initial_position_x.size(); j++) {
				if (initial_position_x[j] == column and initial_position_y[j] == row) {
					i--;
					flag = false;
					break;
				}
			}
			if (flag) {
				map[column][row].boom_exist = true;
			}
		}
		else {
			i--;
			continue;
		}
	
	}

	for (short i = 0; i < initial_position_x.size(); i++) {
		mine_clean(initial_position_x[i], initial_position_y[i], size, x, y);
	}
	/*
		setfillcolor(RED);//调试时直接显示mine的位置
		for (short i = 0; i < y; i++) {
			for (short j = 0; j < x; j++) {
				if (map[j][i].boom_exist == true) {
					solidrectangle(size * j, size * i, size * j + size, size * i + size);
				}
			}
		}
	*/

	while (1) {
		if (MouseHit()) {
			mouse = GetMouseMsg();
			short row = round(mouse.y / size);
			short column = round(mouse.x / size);
			if (row <= y and column <= x) {
				if (mouse.mkLButton) {//询问
					bool flag;//检测鼠标左键松开时的位置是否符合点下时的位置
					while (1) {
						if (MouseHit()) {
							MOUSEMSG mouse1 = GetMouseMsg();
							if (mouselocation_button(mouse1, button_exit)) {
								return;
							}
							if (not mouse1.mkLButton) {
								short row2 = round(mouse.y / size);
								short column2 = round(mouse.x / size);
								if (row != row2 or column != column2) {
									flag = false;
								}
								else {
									flag = true;
								}
								break;
							}
						}
						Sleep(5);
					}

					if (flag == true) {//鼠标回弹时与其落下时的位置相同
						if (map[column][row].flag == false) {//点击位置上的病人未被隔离
							if (map[column][row].boom_exist == false) {//该位置上的人未患病
								short boom = 0;
								for (short i = row - 1; i <= row + 1; i++) {//统计其周围的患者数
									for (short j = column - 1; j <= column + 1; j++) {
										if (i >= 0 and i < y and j >= 0 and j < x and (j != column or i != row)) {
											if (map[j][i].boom_exist == true) {
												boom++;
											}
										}
									}
								}

								if (boom == 0) {
									for (short i = row - 1; i <= row + 1; i++) {
										for (short j = column - 1; j <= column + 1; j++) {
											if (i >= 0 and i < y and j >= 0 and j < x and (j != column or i != row)) {
												mine_clean(i, j, size, x, y);
											}
										}
									}
								}

								//重置被点击的方格
								setfillcolor(WHITE);
								solidrectangle(size * column, size * row, size * column + size, size * row + size);
								setbkmode(TRANSPARENT);
								settextcolor(color_list[boom]);
								settextstyle(size, 0, _T("黑体"));
								char repeater = boom + 48;
								outtextxy(size * column, size * row, repeater);

								map[column][row].footprint = true;
								map[column][row].boom_num = boom;
							}
							else {//任务失败
								//显示所有患者的位置
								setfillcolor(RED);
								for (short i = 0; i < y; i++) {
									for (short j = 0; j < x; j++) {
										if (map[j][i].boom_exist == true) {
											solidrectangle(size * j, size * i, size * j + size, size * i + size);
											Sleep(20);
										}
									}
								}
								//按下左键后退出
								while (1) {
									if (MouseHit()) {
										if (GetMouseMsg().mkLButton) {
											return;
										}
									}
									Sleep(20);
								}
							}
						}
					}
				}
				else if (mouse.mkMButton) {//快速询问
					bool flag;
					//同上，检测鼠标回弹位置
					while (1) {
						if (MouseHit()) {
							MOUSEMSG mouse1 = GetMouseMsg();
							if (not mouse1.mkMButton) {
								short row2 = round(mouse.y / size);
								short column2 = round(mouse.x / size);
								if (row != row2 or column != column2) {
									flag = false;
								}
								else {
									flag = true;
								}
								break;
							}
						}
						Sleep(5);
					}
					if (flag == true) {
						if (map[column][row].footprint == true) {
							short boom = 0;
							short flag = 0;
							for (short i = row - 1; i <= row + 1; i++) {
								for (short j = column - 1; j <= column + 1; j++) {
									if (i >= 0 and i < y and j >= 0 and j < x and (j != column or i != row)) {
										if (map[j][i].boom_exist == true) {
											boom++;
										}
										if (map[j][i].flag == true) {
											flag++;
										}
									}
								}
							}
							if (boom == flag) {
								for (short i = row - 1; i <= row + 1; i++) {
									for (short j = column - 1; j <= column + 1; j++) {
										if (i >= 0 and i < y and j >= 0 and j < x and (j != column or i != row)) {
											if (map[j][i].flag == false and map[j][i].footprint == false) {
												if (map[j][i].boom_exist == true) {
													setfillcolor(RED);
													for (short i = 0; i < y; i++) {
														for (short j = 0; j < x; j++) {
															if (map[j][i].boom_exist == true) {
																solidrectangle(size * j, size * i, size * j + size, size * i + size);
																Sleep(20);
															}
														}
													}
													while (1) {
														if (MouseHit()) {
															if (GetMouseMsg().mkLButton) {
																return;
															}
														}
														Sleep(20);
													}
												}
												else {
													mine_clean(i, j, size, x, y);
												}
											}
										}
									}
								}
							}
						}
					}
				}
				else if (mouse.mkRButton) {//隔离
					bool flag;
					while (1) {
						if (MouseHit()) {
							MOUSEMSG mouse1 = GetMouseMsg();
							if (not mouse1.mkRButton) {
								short row2 = round(mouse.y / size);
								short column2 = round(mouse.x / size);
								if (row != row2 or column != column2) {
									flag = false;
								}
								else {
									flag = true;
								}
								break;
							}
						}
						Sleep(5);
					}
					if (flag == true) {
						if (map[column][row].footprint == false) {
							if (map[column][row].flag == false) {
								boom_num--;
								map[column][row].flag = true;
								setfillcolor(GREEN);
								setlinecolor(WHITE);
								fillrectangle(size * column, size * row, size * column + size, size * row + size);

								if (boom_num == 0) {
									short boom = 0;
									bool flag_1 = true;
									for (short i = 0; i < x; i++) {
										for (short j = 0; j < y; j++) {
											if (map[i][j].boom_exist == true) {
												boom++;
												if (map[i][j].flag == false) {
													flag_1 = false;
												}
											}
										}
									}
									if (flag_1) {
										setlinecolor(GREEN);
										color_text(11);
										cout << "恭喜，您已成功查出所有患者" << endl;

										long int_time = time(0);
										int_time = int_time - ini_time;
										record_log rec;
										rec.boom = boom; rec.x = x; rec.y = y; rec.difficulty = difficulty; rec.time = int_time;
										color_text(11);
										cout << "本次记录：" << endl;
										color_text(14); cout << "所用时间:";
										color_text(7); cout << rec.time << "	";
										color_text(14); cout << "x/y轴长度：";
										color_text(7); cout << rec.x << "/" << rec.y << "	";
										color_text(14); cout << "患者数:";
										color_text(7); cout << rec.boom << endl << endl;

										color_text(11);
										cout << "任务历史记录:";
										for (unsigned short i = 0; i < record_list.size(); i++) {
											if (i < 4) {
												color_text(10);
												cout << endl << record_list[i].difficulty << ":" << endl;
											}
											if (record_list[i].time == 1000) {
												color_text(14);
												cout << "该难度目前还尚未有记录。" << endl;
												color_text(7);
											}
											else {
												color_text(14); cout << "所用时间:";
												color_text(7); cout << record_list[i].time << "	";
												color_text(14); cout << "x/y轴长度：";
												color_text(7); cout << record_list[i].x << "/" << record_list[i].y << "	";
												color_text(14); cout << "患者数:";
												color_text(7); cout << record_list[i].boom << endl;
											}
										}
										color_text(7);

										//用文件流形式输出存档数据
										ofstream output("record.sav");
										bool flag2 = true;
										for (unsigned short i = 0; i < record_list.size(); i++) {
											if (record_list[i].difficulty == rec.difficulty and record_list[i].x == rec.x and record_list[i].y == rec.y and record_list[i].boom == rec.boom) {
												if (record_list[i].time > rec.time) {
													record_list[i] = rec;
												}
												flag2 = false;
												break;
											}
										}
										if (flag2) {
											record_list.push_back(rec);
										}

										output << record_list.size() << endl;
										for (unsigned short i = 0; i < record_list.size(); i++) {
											output << record_list[i].time << "	" << record_list[i].difficulty << "	" << record_list[i].x << "	" << record_list[i].y << "	" << record_list[i].boom << endl;
										}

										output.close();

										for (short i = 0; i < y * size; i++) {
											line(0, i, x * size, i);
											Sleep(10);
											if (MouseHit()) {
												if (GetMouseMsg().mkLButton) {
													setfillcolor(GREEN);
													solidrectangle(0, i, x * size, y * size);
												}
											}
										}
										while (1) {
											if (MouseHit()) {
												if (GetMouseMsg().mkLButton) {
													return;
												}
											}
											Sleep(20);
										}
									}
								}
							}
							else {
								boom_num++;
								map[column][row].flag = false;
								setfillcolor(BLUE);
								setlinecolor(WHITE);
								fillrectangle(size * column, size * row, size * column + size, size * row + size);
							}
						}
					}
				}
			}
			else {
				if (mouselocation_button(mouse, button_exit) and mouse.mkLButton) {
					return;
				}
			}
		}
		//数据反馈
		BeginBatchDraw();
		//输出时间和患者数
		settextcolor(WHITE);
		setfillcolor(BLACK);
		setbkcolor(BLACK);
		settextstyle(size, 0, _T("黑体"));
		clearrectangle(2.5 * size, y * size, x * size - 150, (y + 2) * size);
		long int_time = time(0);
		int_time = int_time - ini_time;
		TCHAR t[4], b[4];
		_stprintf_s(t, _T("%d"), int_time);
		_stprintf_s(b, _T("%d"), boom_num);
		outtextxy(2.5 * size, y * size, t);
		outtextxy(2.5 * size, y * size + size, b);
		EndBatchDraw();
		Sleep(10);
	}
}

void page_begin() {
	system("cls");
	color_text(11);
	cout << "任务最佳记录:" << endl;
	for (unsigned short i = 0; i < record_list.size(); i++) {
		if (i < 4) {
			color_text(10);
			cout << record_list[i].difficulty << ":" << endl;
		}
		if (record_list[i].time == 1000) {
			color_text(14);
			cout << "该难度目前还尚未有记录。" << endl;
			color_text(7);
		}
		else {
			color_text(14); cout << "所用时间:";
			color_text(7); cout << record_list[i].time << "	";
			color_text(14); cout << "x/y轴长度：";
			color_text(7); cout << record_list[i].x << "/" << record_list[i].y << "	";
			color_text(14); cout << "患者数:";
			color_text(7); cout << record_list[i].boom << endl;
		}
	}
	color_text(7);

	BeginBatchDraw();
	refresh_page(refresh_MainInterface);

	button_log button_easy = { 50, 50, 400, 150, 80, 50,20,20, _T("EASY") };
	paint_button(button_easy);

	button_log button_normal = { 50, 200, 400, 300, 80, 200,20,20, _T("NORMAL") };
	paint_button(button_normal);

	button_log button_big = { 50, 350, 400, 450, 80, 350,20,20, _T("BIG") };
	paint_button(button_big);

	button_log button_custom = { 50, 500, 400, 650, 80, 500,20,20, _T("自定义") };
	paint_button(button_custom);

	button_log button_back = { 50,700,400,800,80,700,20,20,_T("BACK") };
	paint_button(button_back);
	settextstyle(20, 0, _T("楷体"));
	outtextxy(50, 600, _T("(点击后请于副窗口输入相关信息)"));

	EndBatchDraw();
	GetMouseMsg();
	while (1) {
		if (MouseHit()) {
			mouse = GetMouseMsg();
			if (mouse.mkLButton) {
				if (mouselocation_button(mouse, button_easy)) {
					game("easy", 9, 9, 10);
					return;
				}
				else if (mouselocation_button(mouse, button_normal)) {
					game("normal", 15, 10, 30);
					return;
				}
				else if (mouselocation_button(mouse, button_big)) {
					game("big", 20, 20, 90);
					return;
				}
				else if (mouselocation_button(mouse, button_custom)) {
					short x, y, boom;
					system("cls");
					color_text(6);
					while (1) {
						cout << "①请输入地图的横向格数:（数量不小于九个,输入0则返回）" << endl;
						cin >> x;
						if (x < 9 and x != 0) {
							color_text(12);
							cout << endl << "数据违规，请重新输入。";
							color_text(6);
							_getch();
							system("cls");
							continue;
						}
						else if (x == 0) {
							return;
						}
						else {
							cout << endl << "②请输入地图的纵向格数:（数量不小于九个,输入0则返回）" << endl;
							cin >> y;
							if (y < 9 and y != 0) {
								color_text(12);
								cout << endl << "数据违规，请重新输入。";
								color_text(6);
								_getch();
								system("cls");
								continue;
							}
							else if (x == 0) {
								return;
							}
							else {
								cout << endl << "②请输入炸弹数:（数量不小于10个,不大于" << x * y - 20 << "个，输入0则返回）" << endl;
								cin >> boom;
								if ((y > x * y - 20 or y < 10) and y != 0) {
									color_text(12);
									cout << endl << "数据违规，请重新输入。";
									color_text(6);
									_getch();
									system("cls");
									continue;
								}
								else if (x == 0) {
									return;
								}
								else {
									game("custom", x, y, boom);
									return;
								}
							}
						}
					}
				}
				else if (mouselocation_button(mouse, button_back)) {
					return;
				}
			}
		}
		Sleep(10);
	}
}

void page_Description(short page) {
	const short wait_time = 50;
	refresh_page(refresh_MainInterface);
	//首先绘制按钮
	button_log Button_Exit = { 50,600,400,700,80,600,20,20,_T("返 回") };
	paint_button(Button_Exit);

	if (page == 1) {
		settextstyle(50, 0, _T("楷体"));
		settextcolor(YELLOW);
		outtextxy(0, 0, _T("游戏背景:"));
		settextcolor(WHITE);
		settextstyle(20, 0, _T("楷体"));
		Sleep(wait_time);
		outtextxy(0, 50, _T("    故事发生在疫情爆发前期。就在不久前，有位患有新"));
		Sleep(wait_time);
		outtextxy(0, 70, _T("冠肺炎的患者在某个公共场所中出入。虽然相关人员已及"));
		Sleep(wait_time);
		outtextxy(0, 90, _T("时隔离此人并封锁该地，但令人头疼的是，他毋庸置疑已"));
		Sleep(wait_time);
		outtextxy(0, 110, _T("经接触了大量人群。"));
		Sleep(wait_time);
		outtextxy(0, 130, _T("    现在，你的任务便是将被传染的人调查出来。但因为"));
		Sleep(wait_time);
		outtextxy(0, 150, _T("医疗设备紧张，大数据、核酸检测等技术尚未普及开来，"));
		Sleep(wait_time);
		outtextxy(0, 170, _T("你只能通过询问未与患者接触的群众来得知他近日接触过"));
		Sleep(wait_time);
		outtextxy(0, 190, _T("多少人，最终通过重重线索锁定并隔离潜伏患者。"));
		Sleep(wait_time);
		settextcolor(RED);
		outtextxy(0, 220, _T("    切记："));
		settextcolor(WHITE);
		outtextxy(5 * 20, 220, _T("你不能直接接触潜伏患者，否则你自己将不得"));
		Sleep(wait_time);
		outtextxy(0, 240, _T("不被隔离并导致任务失败！"));

		settextstyle(50, 0, _T("楷体"));
		settextcolor(YELLOW);
		outtextxy(0, 300, _T("玩法请参照副窗口"));
		Sleep(wait_time);
		settextcolor(WHITE);

		system("cls");
		color_text(10);
		cout << "玩法：" << endl;
		color_text(13);
		cout << "一、操作" << endl;
		color_text(6);
		cout << "①鼠标左键：询问" << endl;
		color_text(7);
		cout << "	询问疑似患有新冠肺炎的人将直接导致任务失败。询问健康者则会在原位置返回一个数字，表示他周围八格中患病者的人数。" << endl;
		color_text(6);
		cout << "②右键：隔离（已被隔离的人则被解除隔离）" << endl;
		color_text(7);
		cout << "	任务的胜利条件便是隔离所有患病者。若隔离了健康者，则意味着仍有患病者在外，游戏仍会继续进行。已被隔离的人无法被询问。" << endl;
		color_text(6);
		cout << "③中键：快速询问" << endl;
		color_text(7);
		cout << "	询问周围所有未被隔离的人。" << endl << endl;
		color_text(13);
		cout << "二、图标" << endl;
		color_text(30);
		cout << "      " << endl;
		cout << "  "; color_text(112); cout << " 1"; cout << "  " << endl;
		color_text(160); cout << "  "; color_text(30); cout << "    " << endl << endl;
		color_text(30); cout << "  "; color_text(7); cout << ":情况不明的人。可能是患者，也可能是健康者。" << endl;
		color_text(34); cout << "  "; color_text(7); cout << ":被隔离者。左键、中键对其无效。" << endl;
		color_text(112); cout << " 1"; color_text(7); cout << ":健康者。其中的数字表示他周围八格中的患者人数。" << endl;
	}
	while (1) {
		if (MouseHit()) {
			mouse = GetMouseMsg();
			if (mouselocation_button(mouse, Button_Exit) and mouse.mkLButton) {
				system("cls");
				color_text(6);
				cout << "这里是副窗口,主要用来输出复杂信息、接收消息等，基本上除非在主窗口中有提示外，不需要特别关注这里的消息";
				GetMouseMsg();
				return;
			}
		}
		Sleep(20);
	}
}

void page_Initialization() {
	BeginBatchDraw();
	setbkmode(TRANSPARENT);
	//填充背景
	for (short i = 0; i < 800; i++) {
		setlinecolor(RGB(0, 0, i * 255 / 800));
		line(0, i, 500, i);
	}
	//标题
	settextstyle(100, 0, _T("楷体"));
	settextcolor(GREEN);
	outtextxy(100, 20, _T("溯源"));

	settextcolor(BLACK);
	setbkcolor(WHITE);
	//按钮1
	button_log Button_Begin = { 50,200,400,300,80,200,20,20,_T("开 始") };
	paint_button(Button_Begin);
	//按钮2
	button_log Button_describe = { 50, 400, 400, 500,80, 400,20,20, _T("说 明") };
	paint_button(Button_describe);
	//按钮3
	button_log Button_Exit = { 50,600,400,700,80,600,20,20,_T("退 出") };
	paint_button(Button_Exit);
	EndBatchDraw();
	while (1) {
		if (MouseHit()) {
			mouse = GetMouseMsg();
			if (mouse.mkLButton) {
				if (mouselocation_button(mouse, Button_Begin)) {
					page_begin();
					return;
				}
				else if (mouselocation_button(mouse, Button_describe)) {
					setfillcolor(WHITE);
					page_Description(1);
					return;
				}
				else if (mouselocation_button(mouse, Button_Exit)) {
					exit(0);
				}
			}
		}
		Sleep(10);
	}
}

int main(){
	while (1) {
		//用文件流形式读取记录
		ifstream input("record.sav");
		record_list.clear();
		short p;
		input >> p;
		for (short i = 0; i < p; i++) {
			record_log n;
			input >> n.time >> n.difficulty >> n.x >> n.y >> n.boom;
			record_list.push_back(n);
		}
		input.close();

		//注册窗口（并保留调试控制台作为副窗口）
		initgraph(500, 800, EW_SHOWCONSOLE);

		//用时间来给随机函数做种
		srand((unsigned)time(0));

		//设定输出文本的全部参数
		settextstyle(50, 0, _T("楷体"), 0, 0, 0, false, false, false, GB2312_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH);

		//开始绘制主界面
		for (short i = 0; i < 800; i++) {//开场渐变效果（按下鼠标任意键位直接跳过）
			setlinecolor(RGB(0, 0, i * 255 / 800));
			line(0, i, 500, i);
			if (MouseHit()) {
				mouse = GetMouseMsg();
				if (mouse.mkLButton or mouse.mkRButton or mouse.mkMButton) {
					for (short j = i; j < 800; j++) {
						setlinecolor(RGB(0, 0, j * 255 / 800));
						line(0, j, 500, j);
					}
					break;
				}
			}
			Sleep(2);
		}
		Sleep(100);
		system("cls");
		color_text(10);
		cout << "这里是副窗口,主要用来输出复杂信息、接收消息等，基本上除非在主窗口中有提示外，不需要特别关注这里的消息";
		page_Initialization();
	}
	return 0;
}