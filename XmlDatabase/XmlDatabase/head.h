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
	int type;			//���࣬tΪ��ǩ���֣�aΪ"=<>\#�е�һ�֣�cΪ����,fΪ���ս��
public:
	Word ();
	void Insert(char c);
	bool IS_TAG();		//�Ƚϸõ����Ƿ��Ǳ�ǩ
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
	char **pro;			//��������
	int  *type;			//�������࣬0:int,1:char,>1,char[]
	char **record;		//�����¼
	int record_num;		//��¼����
	int pro_num;		//��������
	Table* next_table;		//�����ݿ��е���һ�ű�
	Database *db;
public:
	Table();
	void Show();		//��ʾTable
	void ShowWithCondition(int loc_of_from);	//select���
	void Insert();
	void Delete();
	char *WhereAnaylize(int loc_of_where);	//����where�����䣬�����沨��ʽ
	bool Match(char* rcd,char* polan,int loc_of_where);		//���ݼ�¼���㲨��ʽ
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

//�������
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