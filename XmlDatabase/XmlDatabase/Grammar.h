#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>

using namespace std;

class ItemSet;
class ArcNode;
class VNode;
class Graphis;
class Grammar;

class Experssion
{
public:
	char expr[128];
	int offset;
	int no;
public:
	Experssion ();
	bool operator==(Experssion t);
	void Show();
};

class ItemSet
{
public:
	int no;
	int num;
	Experssion *expr;
public:
	ItemSet ();
	void AddExpr(Experssion new_expr);
	bool operator==(ItemSet t);
	//testshow
	void Show();
};

class ArcNode
{
public:
	char simbol;
	ArcNode * next;
	int index;
public:
	ArcNode ();
};

class VNode
{
public:
	ItemSet *itemset;
	ArcNode * firstarc;
public:
	VNode ();
	void AddArcNode(char c,int in);
};

class Graphis
{
public:
	//�������ڵ���
	int vexnum;
	VNode * vertices;
public:
	Graphis ();
	VNode* AddVNode ();
	void Show();
};

class Grammar
{
public:
	ifstream file;
	int num;
	int loc;
	Experssion * expr;
	Graphis g;
	int* state;
	char* simbol;
	char ***form;
	bool **first;
	bool **follow;
	int simbol_num;
public:
	Grammar ();
	bool Open(string filename);
	void Read();
	void InitGraphis();					//��ʼ��DFA�ĵ�һ���ڵ㣨�ķ���ʼ���ţ�δ��չ��
	void ExpandItemSet(ItemSet *t);		//��չһ����Ŀ��
	void GenerateDFA();					//����DFA
	void NewItemSet(int i);				//��������Ŀ���淶��
	void GenerateForm();				//����Ԥ�������
	void First();						//����first��
	void Follow();						//����follow��
	void itoa(char *,int);
};

#define IsTerminal(ch)\
	((ch) < 'A' || (ch) >'Z')