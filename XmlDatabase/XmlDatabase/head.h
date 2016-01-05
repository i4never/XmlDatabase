#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>

using namespace std;

class Word;
class StackNode;
class Stack;
class Table;
class Database;

class Word{
public:
	int size;
	char *word;
	int type;			//种类，t为标签名字，a为"=<>\#中的一种，c为内容,f为非终结符
public:
	Word ();
	void Insert(char c);
	bool IS_TAG();		//比较该单词是否是标签
};

class StackNode{
public:
	Word c;
	int state;
	StackNode * last;
public:
	StackNode();
};

class Stack{
public:
	int size;
	StackNode * top;
public:
	Stack();
	Word Pop();
	void Push(Word c,int s);
	void Show(bool tag);
	bool IsEmpty();
};

class Table{
public:
	char *name;
	char **pro;			//属性名字
	int  *type;			//属性种类，0:int,1:char,>1,char[]
	char **record;		//具体记录
	int record_num;		//记录数量
	int pro_num;		//属性数量
	Table* next_table;		//该数据库中的下一张表
	Database *db;
public:
	Table();
	void Show();		//显示Table
	void ShowWithCondition(int loc_of_from);	//select语句
	void Insert();
	void Delete();
	char *WhereAnaylize(int loc_of_where);	//分析where后的语句，返回逆波兰式
	bool Match(char* rcd,char* polan,int loc_of_where);		//根据记录计算波兰式
};

class Database{
public:
	char * name;
	ifstream file;
	Table *t;
public:
	Database();
	bool Open(string filename);
	Word Get_next_word();
	bool Grammar_Anaylize();
	Table* InsertTable();
	void AddRecord(Table * t,Word record);
	void Save();
};

//命令解析
char line[1000];
char* args[50];
int argsCnt;

#define IsKeyToken( ch ) \
		( (ch) == ';' || \
		 (ch) == '|'  || \
		 (ch) == '&'  || \
		 (ch) == '>'  || \
		 (ch) == '<'  || \
		 (ch) == '('  || \
		 (ch) == ')' )

#define IsSpace( ch ) (ch) == ' '

#define IsVt( ch ) \
		( (ch) == 'o' || \
		 (ch) == 'a'  || \
		 (ch) == '('  || \
		 (ch) == ')'  || \
		 (ch) == '#' )

char* NextKeyToken(char* input);
char* TrimLeft(char* tp);
int SpiltCommand(char* input);
void Execute();