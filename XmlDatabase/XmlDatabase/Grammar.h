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
	//边数，节点数
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
	void InitGraphis();					//初始化DFA的第一个节点（文法开始符号，未扩展）
	void ExpandItemSet(ItemSet *t);		//扩展一个项目集
	void GenerateDFA();					//生成DFA
	void NewItemSet(int i);				//生成新项目集规范簇
	void GenerateForm();				//生成预测分析表
	void First();						//计算first集
	void Follow();						//计算follow集
	void itoa(char *,int);
};

#define IsTerminal(ch)\
	((ch) < 'A' || (ch) >'Z')