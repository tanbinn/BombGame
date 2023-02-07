#include<graphics.h>//�ڱ���ǰ�밲װeasyx������
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
//0:��ɫ��1����ɫ��2����ɫ��3����ɫ��4����ɫ��5����ɫ��6����ɫ��7����ɫ��8����ɫ

void color_text(short x)	//�Զ��庯���ݲ����ı���Կ���̨������ı�����ɫ
{
	if (x >= 0 && x <= 256)//������0-15�ķ�Χ��ɫ
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), x);	//ֻ��һ������ 
	else//Ĭ�ϵ���ɫ��ɫ
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}

struct cell_information {//��ͼ�е������ӵ���Ϣ
	bool boom_exist;//�ø������Ƿ��л���
	bool footprint;//�ø����Ƿ񱻵����
	bool flag;//�Ƿ񱻸���
	short boom_num;//��Χ��ը����
};
vector <vector <cell_information> > map;//ע���ά��̬������Ϊ��ͼ
//x->column->���ᣬy->row->����

struct record_log {//�����洢��¼����
	int time;
	string difficulty;
	short x, y, boom;
};
vector <record_log> record_list;

struct button_log {//��ťע�ắ��
	short left, top, right, bottom;//��ť��λ��
	short x, y;//���ֵ�λ��
	short ellipsewidth, ellipseheight;//�����У�Բ�ǿ�߶�
	TCHAR mes[];//��������
};

const short refresh_MainInterface = 1;//����ˢ���õĳ���
void refresh_page(short choose) {//����ˢ�º���
	if (choose == refresh_MainInterface) {
		for (short i = 0; i < 800; i++) {
			setlinecolor(RGB(0, 0, i * 255 / 800));
			line(0, i, 500, i);
		}
	}
}

bool mouselocation_button(MOUSEMSG& mse, button_log& button) {//���Լ�����Ƿ�����ĳ����ť��
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

void paint_button(button_log& button) {//���Ի��ư�ť�ĺ���
	fillroundrect(button.left, button.top, button.right, button.bottom, button.ellipsewidth, button.ellipseheight);
	outtextxy(button.x, button.y, button.mes);
}

void mine_log(vector <short>& ini_x, vector <short>& ini_y, short row, short column, short x, short y, short& ran) {
	//��ʼ����ͼʱ����Χ���Ʋ���
	short I = 1000;short J = 1000;
	for (short i = row - 1; i <= row + 1; i++) {
		for (short j = column - 1; j <= column + 1; j++) {
			if (i >= 0 and i < y and j >= 0 and j < x and (j != column or i != row)) {
				bool flag = true;
				for (short o = 0; o < ini_x.size(); o++) {
					if (ini_x[o] == j and ini_y[o] == i) {//����Ƿ����ظ�
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
	//��Ѷ�ʺ󣬽���Χ������Χû�л��ߵĸ�����ʾ����
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

		setfillcolor(WHITE);//���ñ�����ķ���
		setbkmode(TRANSPARENT);//����ı��ı���͸��
		solidrectangle(size * column, size * row, size * column + size, size * row + size);//ָ��x��yλ�ü����εĳ���Ȼ�����
		settextcolor(color_list[boom]);//�����ı���ɫ
		settextstyle(size, 0, _T("����"));//�����ı��ĸ�ʽ
		char repeater = boom + 48;//��ʣ�ಡ�����������м���
		outtextxy(size * column, size * row, repeater);//����ı�
		map[column][row].footprint = true;
		map[column][row].boom_num = boom;

		if (boom == 0) {//������ǰ������Χû�л��ߣ���ֱ�ӵݶѼ������Χ�ĸ���
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

	//�趨��׼ʱ��
	long ini_time = time(0);

	//���ݲ�������ע�ᴰ��
	initgraph(x * size, y * size + 2 * size, EW_SHOWCONSOLE);
	BeginBatchDraw();
	setlinecolor(BLUE);
	for (short i = 0; i < x * size; i++) {//���Ƶ�ɫ
		line(i, 0, i, y * size);
	}
	setlinecolor(WHITE);
	for (short i = 0; i < x; i++) {//���Ʒ���
		line(i * size, 0, i * size, y * size);

		vector <cell_information> repeater;
		for (short j = 0; j < y; j++) {//ͬʱ��ʼ��map��̬����
			line(0, j * size, x * size, j * size);
			cell_information n = { false,false,false,0 };
			repeater.push_back(n);
		}
		map.push_back(repeater);
	}
	system("cls");

	//����״̬��
	settextcolor(YELLOW);
	settextstyle(size, 0, _T("����"));
	outtextxy(0, y * size, _T("ʱ��:"));
	outtextxy(0, y * size + size, _T("����:"));

	//���Ʒ��ذ�ť
	setbkcolor(WHITE);
	settextstyle(2 * size, 0, _T("����"));
	settextcolor(RGB(148, 0, 211));
	button_log button_exit = { x * size - 150,y * size,x * size,(y + 2) * size,x * size - 150,y * size,30,30,_T("����") };
	paint_button(button_exit);
	EndBatchDraw();

	vector <short> initial_position_x, initial_position_y;
	while (1) {
		if (MouseHit()) {//��һ�µ��еļ�����Χ�ĸ����ϲ�����mine,�����ö�̬����洢��һ���������������
			mouse = GetMouseMsg();
			short row = round(mouse.y / size);
			short column = round(mouse.x / size);
			if (mouse.mkLButton and row < y and column < x) {
				initial_position_x.push_back(column);
				initial_position_y.push_back(row);
				short ran = rand() % 3 + 5;
				srand((unsigned)time(0));
				mine_log(initial_position_x, initial_position_y, row, column, x, y, ran);
				//��Ҫ���û��mine�ĳ�ʼ���
				break;
			}
			else if(mouse.mkLButton and mouselocation_button(mouse,button_exit)) {
				return;
			}
		}
		Sleep(20);
	}

	for (short i = 0; i < boom_num; i++) {//�������
		short ran_num = rand() % (x * y);
		short row = ran_num / x;
		short column = ran_num % x;
		if (map[column][row].boom_exist == false) {//�رܳ�ʼ���
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
		setfillcolor(RED);//����ʱֱ����ʾmine��λ��
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
				if (mouse.mkLButton) {//ѯ��
					bool flag;//����������ɿ�ʱ��λ���Ƿ���ϵ���ʱ��λ��
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

					if (flag == true) {//���ص�ʱ��������ʱ��λ����ͬ
						if (map[column][row].flag == false) {//���λ���ϵĲ���δ������
							if (map[column][row].boom_exist == false) {//��λ���ϵ���δ����
								short boom = 0;
								for (short i = row - 1; i <= row + 1; i++) {//ͳ������Χ�Ļ�����
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

								//���ñ�����ķ���
								setfillcolor(WHITE);
								solidrectangle(size * column, size * row, size * column + size, size * row + size);
								setbkmode(TRANSPARENT);
								settextcolor(color_list[boom]);
								settextstyle(size, 0, _T("����"));
								char repeater = boom + 48;
								outtextxy(size * column, size * row, repeater);

								map[column][row].footprint = true;
								map[column][row].boom_num = boom;
							}
							else {//����ʧ��
								//��ʾ���л��ߵ�λ��
								setfillcolor(RED);
								for (short i = 0; i < y; i++) {
									for (short j = 0; j < x; j++) {
										if (map[j][i].boom_exist == true) {
											solidrectangle(size * j, size * i, size * j + size, size * i + size);
											Sleep(20);
										}
									}
								}
								//����������˳�
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
				else if (mouse.mkMButton) {//����ѯ��
					bool flag;
					//ͬ�ϣ�������ص�λ��
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
				else if (mouse.mkRButton) {//����
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
										cout << "��ϲ�����ѳɹ�������л���" << endl;

										long int_time = time(0);
										int_time = int_time - ini_time;
										record_log rec;
										rec.boom = boom; rec.x = x; rec.y = y; rec.difficulty = difficulty; rec.time = int_time;
										color_text(11);
										cout << "���μ�¼��" << endl;
										color_text(14); cout << "����ʱ��:";
										color_text(7); cout << rec.time << "	";
										color_text(14); cout << "x/y�᳤�ȣ�";
										color_text(7); cout << rec.x << "/" << rec.y << "	";
										color_text(14); cout << "������:";
										color_text(7); cout << rec.boom << endl << endl;

										color_text(11);
										cout << "������ʷ��¼:";
										for (unsigned short i = 0; i < record_list.size(); i++) {
											if (i < 4) {
												color_text(10);
												cout << endl << record_list[i].difficulty << ":" << endl;
											}
											if (record_list[i].time == 1000) {
												color_text(14);
												cout << "���Ѷ�Ŀǰ����δ�м�¼��" << endl;
												color_text(7);
											}
											else {
												color_text(14); cout << "����ʱ��:";
												color_text(7); cout << record_list[i].time << "	";
												color_text(14); cout << "x/y�᳤�ȣ�";
												color_text(7); cout << record_list[i].x << "/" << record_list[i].y << "	";
												color_text(14); cout << "������:";
												color_text(7); cout << record_list[i].boom << endl;
											}
										}
										color_text(7);

										//���ļ�����ʽ����浵����
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
		//���ݷ���
		BeginBatchDraw();
		//���ʱ��ͻ�����
		settextcolor(WHITE);
		setfillcolor(BLACK);
		setbkcolor(BLACK);
		settextstyle(size, 0, _T("����"));
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
	cout << "������Ѽ�¼:" << endl;
	for (unsigned short i = 0; i < record_list.size(); i++) {
		if (i < 4) {
			color_text(10);
			cout << record_list[i].difficulty << ":" << endl;
		}
		if (record_list[i].time == 1000) {
			color_text(14);
			cout << "���Ѷ�Ŀǰ����δ�м�¼��" << endl;
			color_text(7);
		}
		else {
			color_text(14); cout << "����ʱ��:";
			color_text(7); cout << record_list[i].time << "	";
			color_text(14); cout << "x/y�᳤�ȣ�";
			color_text(7); cout << record_list[i].x << "/" << record_list[i].y << "	";
			color_text(14); cout << "������:";
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

	button_log button_custom = { 50, 500, 400, 650, 80, 500,20,20, _T("�Զ���") };
	paint_button(button_custom);

	button_log button_back = { 50,700,400,800,80,700,20,20,_T("BACK") };
	paint_button(button_back);
	settextstyle(20, 0, _T("����"));
	outtextxy(50, 600, _T("(��������ڸ��������������Ϣ)"));

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
						cout << "���������ͼ�ĺ������:��������С�ھŸ�,����0�򷵻أ�" << endl;
						cin >> x;
						if (x < 9 and x != 0) {
							color_text(12);
							cout << endl << "����Υ�棬���������롣";
							color_text(6);
							_getch();
							system("cls");
							continue;
						}
						else if (x == 0) {
							return;
						}
						else {
							cout << endl << "���������ͼ���������:��������С�ھŸ�,����0�򷵻أ�" << endl;
							cin >> y;
							if (y < 9 and y != 0) {
								color_text(12);
								cout << endl << "����Υ�棬���������롣";
								color_text(6);
								_getch();
								system("cls");
								continue;
							}
							else if (x == 0) {
								return;
							}
							else {
								cout << endl << "��������ը����:��������С��10��,������" << x * y - 20 << "��������0�򷵻أ�" << endl;
								cin >> boom;
								if ((y > x * y - 20 or y < 10) and y != 0) {
									color_text(12);
									cout << endl << "����Υ�棬���������롣";
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
	//���Ȼ��ư�ť
	button_log Button_Exit = { 50,600,400,700,80,600,20,20,_T("�� ��") };
	paint_button(Button_Exit);

	if (page == 1) {
		settextstyle(50, 0, _T("����"));
		settextcolor(YELLOW);
		outtextxy(0, 0, _T("��Ϸ����:"));
		settextcolor(WHITE);
		settextstyle(20, 0, _T("����"));
		Sleep(wait_time);
		outtextxy(0, 50, _T("    ���·��������鱬��ǰ�ڡ����ڲ���ǰ����λ������"));
		Sleep(wait_time);
		outtextxy(0, 70, _T("�ڷ��׵Ļ�����ĳ�����������г��롣��Ȼ�����Ա�Ѽ�"));
		Sleep(wait_time);
		outtextxy(0, 90, _T("ʱ������˲������õأ�������ͷ�۵��ǣ�����ӹ������"));
		Sleep(wait_time);
		outtextxy(0, 110, _T("���Ӵ��˴�����Ⱥ��"));
		Sleep(wait_time);
		outtextxy(0, 130, _T("    ���ڣ����������ǽ�����Ⱦ���˵������������Ϊ"));
		Sleep(wait_time);
		outtextxy(0, 150, _T("ҽ���豸���ţ������ݡ�������ȼ�����δ�ռ�������"));
		Sleep(wait_time);
		outtextxy(0, 170, _T("��ֻ��ͨ��ѯ��δ�뻼�߽Ӵ���Ⱥ������֪�����սӴ���"));
		Sleep(wait_time);
		outtextxy(0, 190, _T("�����ˣ�����ͨ��������������������Ǳ�����ߡ�"));
		Sleep(wait_time);
		settextcolor(RED);
		outtextxy(0, 220, _T("    �мǣ�"));
		settextcolor(WHITE);
		outtextxy(5 * 20, 220, _T("�㲻��ֱ�ӽӴ�Ǳ�����ߣ��������Լ�������"));
		Sleep(wait_time);
		outtextxy(0, 240, _T("�������벢��������ʧ�ܣ�"));

		settextstyle(50, 0, _T("����"));
		settextcolor(YELLOW);
		outtextxy(0, 300, _T("�淨����ո�����"));
		Sleep(wait_time);
		settextcolor(WHITE);

		system("cls");
		color_text(10);
		cout << "�淨��" << endl;
		color_text(13);
		cout << "һ������" << endl;
		color_text(6);
		cout << "����������ѯ��" << endl;
		color_text(7);
		cout << "	ѯ�����ƻ����¹ڷ��׵��˽�ֱ�ӵ�������ʧ�ܡ�ѯ�ʽ����������ԭλ�÷���һ�����֣���ʾ����Χ�˸��л����ߵ�������" << endl;
		color_text(6);
		cout << "���Ҽ������루�ѱ���������򱻽�����룩" << endl;
		color_text(7);
		cout << "	�����ʤ���������Ǹ������л����ߡ��������˽����ߣ�����ζ�����л��������⣬��Ϸ�Ի�������С��ѱ���������޷���ѯ�ʡ�" << endl;
		color_text(6);
		cout << "���м�������ѯ��" << endl;
		color_text(7);
		cout << "	ѯ����Χ����δ��������ˡ�" << endl << endl;
		color_text(13);
		cout << "����ͼ��" << endl;
		color_text(30);
		cout << "      " << endl;
		cout << "  "; color_text(112); cout << " 1"; cout << "  " << endl;
		color_text(160); cout << "  "; color_text(30); cout << "    " << endl << endl;
		color_text(30); cout << "  "; color_text(7); cout << ":����������ˡ������ǻ��ߣ�Ҳ�����ǽ����ߡ�" << endl;
		color_text(34); cout << "  "; color_text(7); cout << ":�������ߡ�������м�������Ч��" << endl;
		color_text(112); cout << " 1"; color_text(7); cout << ":�����ߡ����е����ֱ�ʾ����Χ�˸��еĻ���������" << endl;
	}
	while (1) {
		if (MouseHit()) {
			mouse = GetMouseMsg();
			if (mouselocation_button(mouse, Button_Exit) and mouse.mkLButton) {
				system("cls");
				color_text(6);
				cout << "�����Ǹ�����,��Ҫ�������������Ϣ��������Ϣ�ȣ������ϳ�����������������ʾ�⣬����Ҫ�ر��ע�������Ϣ";
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
	//��䱳��
	for (short i = 0; i < 800; i++) {
		setlinecolor(RGB(0, 0, i * 255 / 800));
		line(0, i, 500, i);
	}
	//����
	settextstyle(100, 0, _T("����"));
	settextcolor(GREEN);
	outtextxy(100, 20, _T("��Դ"));

	settextcolor(BLACK);
	setbkcolor(WHITE);
	//��ť1
	button_log Button_Begin = { 50,200,400,300,80,200,20,20,_T("�� ʼ") };
	paint_button(Button_Begin);
	//��ť2
	button_log Button_describe = { 50, 400, 400, 500,80, 400,20,20, _T("˵ ��") };
	paint_button(Button_describe);
	//��ť3
	button_log Button_Exit = { 50,600,400,700,80,600,20,20,_T("�� ��") };
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
		//���ļ�����ʽ��ȡ��¼
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

		//ע�ᴰ�ڣ����������Կ���̨��Ϊ�����ڣ�
		initgraph(500, 800, EW_SHOWCONSOLE);

		//��ʱ�����������������
		srand((unsigned)time(0));

		//�趨����ı���ȫ������
		settextstyle(50, 0, _T("����"), 0, 0, 0, false, false, false, GB2312_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH);

		//��ʼ����������
		for (short i = 0; i < 800; i++) {//��������Ч����������������λֱ��������
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
		cout << "�����Ǹ�����,��Ҫ�������������Ϣ��������Ϣ�ȣ������ϳ�����������������ʾ�⣬����Ҫ�ر��ע�������Ϣ";
		page_Initialization();
	}
	return 0;
}