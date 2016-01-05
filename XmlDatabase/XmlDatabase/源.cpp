#include "head.h"
#include "Grammar.h"
#include <iostream>
#include <iomanip>

using namespace std;

Database db;
Grammar G;
Table* current;
char vt[6][4];
int priority[6][6] = {{1,-1,-1,1,-1,1},
						{1,1,-1,1,-1,1},
						{-1,-1,-1,0,-1,-2},
						{1,1,-2,1,-2,1},
						{1,1,-2,1,-2,1},
						{-1,-1,-1,-1,-1,0}};

int main()
{
	//预处理，sql语句的算符优先表，-1小于，0无效，1大于
	vt[0][0] = 'o';	vt[0][1] = 'r'; vt[0][2] = 0;
	vt[1][0] = 'a';	vt[1][1] = 'n'; vt[1][2] = 'd'; vt[1][3] = 0;
	vt[2][0] = '(';	vt[2][1] = 0;
	vt[3][0] = ')';	vt[3][1] = 0;
	vt[4][0] = 'e';	vt[4][1] = 0;
	vt[5][0] = '#';	vt[5][1] = 0;

	cout<<"anaylizing grammar file..."<<endl;
	if (!G.Open("H:\\test.txt"))
	{
		cout<<"fail to open file"<<endl;
		exit(-1);
	}
	G.Read();
	G.InitGraphis();
	G.GenerateDFA();
	G.GenerateForm();
	cout<<"anaylizing grammar file success"<<endl<<endl;

	//打开文件
	if (!db.Open("H:\\xml.txt"))
	{
		cout<<"fail to open file"<<endl;
		exit(-1);
	}

	db.Grammar_Anaylize();

	char lineInput[512];
	while(1)
	{
		cout<<">";
		memset(lineInput,0,512);
		cin.get(lineInput,512);
		fflush(stdin);
		argsCnt = SpiltCommand(lineInput);
		if (args[0][0] == 'q')
		{
			cout<<"Bye"<<endl;
			db.Save();
			break;
		}
		Execute();
	}
	
	return 0;
}

void Execute()
{
	int i;
	if (strcmp("select",args[0]) == 0)
	{
		//定位到from，找到table
		for (i = 0 ; i < argsCnt ; i++)
			if (strcmp("from",args[i]) == 0)
				break;
		if (i == argsCnt)
		{
			cout<<"ERR,select without from"<<endl;
			return;
		}
		current = db.t;
		while (current != NULL && strcmp(current->name,args[i+1])!=0)
			current = current->next_table;
		if (current == NULL)
		{
			cout<<"ERR,table doesn't exsist"<<endl;
			return;
		}
		if (args[1][0] == '*')
			current->Show();
		else
			current->ShowWithCondition(i);
	}
	if (strcmp("insert",args[0]) == 0)
	{
		current = db.t;
		while (current != NULL && strcmp(current->name,args[2])!=0)
			current = current->next_table;
		if (current == NULL)
		{
			cout<<"ERR,table doesn't exsist"<<endl;
			return;
		}
		if (args[4][0] != '(' || args[6][0] != ')')
		{
			cout<<"ERR,can't find value"<<endl;
			return;
		}
		current->Insert();
	}
	if (strcmp("delete",args[0]) == 0)
	{
		//定位到from，找到table
		for (i = 0 ; i < argsCnt ; i++)
			if (strcmp("from",args[i]) == 0)
				break;
		if (i == argsCnt)
		{
			cout<<"ERR,select without from"<<endl;
			return;
		}
		current = db.t;
		while (current != NULL && strcmp(current->name,args[i+1])!=0)
			current = current->next_table;
		if (current == NULL)
		{
			cout<<"ERR,table doesn't exsist"<<endl;
			return;
		}
		current->Delete();
	}
}

//预处理
char* NextKeyToken(char* input)
{
	char* p = input;
	while ( *p )
	{
		if ( IsKeyToken(*p) )
		{
			if ( *(p-1) == '\\' ) 
			{
				p += 2;
				continue;
			}
			else break;
		}
		else if ( IsSpace(*p) ) break;
		p++;
	}
	return p;
}

char* TrimLeft( char* tp )
{
	char* p = tp;
	while ( IsSpace(*p) && *p ) ++p;	/* 去掉多余的空格 */
	return p;
}

int SpiltCommand( char* input )
{
	char* pline = line;
	int nargs = 0;
	char* pinput = input;
	char* pKeyToken = input;

	while ( *pKeyToken )
	{
		pinput = TrimLeft( pinput );				//左边去掉多余空格

		pKeyToken = NextKeyToken(pinput);			//锁定命令位置从pinput至pKeyToken-1
		if ( pKeyToken > pinput )					//读入参数
		{	
			args[ nargs++ ] = pline;
			while ( pinput < pKeyToken )
			{
				if ( *pinput == '\\' && ( IsKeyToken( *(pinput+1) ) || *(pinput+1) == '\\' ) )
					++pinput;
				*pline++ = *pinput++;
			}
			*pline++ = 0;
		}
		
		if ( IsKeyToken( *pKeyToken ) )				//排除pKeyToken指向\\的情况
		{
			args[ nargs++ ] = pline;
			*pline++ = *pKeyToken;
			*pline++ = 0;
		}
		pinput = pKeyToken + 1;
	}
	return nargs;
}

//Database****************************************
Database::Database ()
{
	t=NULL;
}

bool Database::Open(string filename)
{
	file.open(filename);	
	return file.is_open();
}

Word Database::Get_next_word()
{
	Word a;
	char c=' ';
	//去除前导
	while (c==' ' || c=='\t' || c=='\n')
		c = file.get();

	//读到文件末尾
	if (c == EOF)
	{
		a.Insert('#');
		a.type = 'a';
		return a;
	}
	if (c == '<' || c == '>' || c=='\"' || c=='=' || c=='/')
	{
		a.Insert(c);
		a.type = 'a';
		//cout<<"type: "<<a.type<<" "<<a.word;
		return a;
	}
	else
	{
		a.Insert(c);
		c = file.get();
		//c不是界符
		while (c!=' ' && c!='\t' && c!='\n' && c!='\"' && c!='<' && c!='>' && c!='/')
		{
			a.Insert(c);
			c = file.get();
		}
		file.seekg(-1,ios::cur);
	}
	a.IS_TAG() ? a.type='t' : a.type='c';

	//cout<<"type: "<<a.type<<" "<<a.word;
	return a;
}

bool Database::Grammar_Anaylize()
{
	int i;
	Word temp;
	char action[5];
	Stack s1,s2,s3;		//s1:符号栈,s2:状态栈
	temp.size = 2;
	temp.word = new char[2];
	temp.word[0] = '#';
	temp.word[1] = 0;
	s1.Push(temp,-1);
	s2.Push(temp,0);
	
	temp = Get_next_word();
	while (!s1.IsEmpty())
	{
		//找到下一个终结符在simbol中的位置
		for (i = 0 ; i < G.simbol_num ; i++)
		{
			if ((temp.type == 'a' && G.simbol[i] == temp.word[0]) || (temp.type == G.simbol[i]))
				break;
		}
		memcpy(action,G.form[s2.top->state][i],5);
		//cout<<"location: "<<s2.top->state<<" "<<i<<"action:"<<action<<endl;
		if (action[0] == 'a' && action[1] == 'c' && action[2] == 'c')	//分析成功
		{
			cout<<"Grammar anaylize success"<<endl<<endl;
			return true;
		}
		if (action[0] == 0)		//若表项为空，有语法错误
		{
			cout<<"anaylize error!"<<endl;
			return false;
		}
		if (action[0] == 's')	//移进项目
		{
			s1.Push(temp,-1);
			s2.Push(temp,atoi(&action[1]));
			if (temp.word[0]!='#')
				temp = Get_next_word();
		}
		else if (action[0] == 'r')	//规约项目
		{
			//cout<<"use :"<<G.expr[atoi(&action[1])].expr<<" 规约"<<endl;
			//***********xml特有，使用堆栈s3来判断Head与Tail是否匹配，规约成H与T时需要判断是否标签符合
			if (G.expr[atoi(&action[1])].expr[0] == 'H')	//Head Push
			{
				s3.Push(s1.top->last->last->c,-1);
			}
			if (G.expr[atoi(&action[1])].expr[0] == 'T')	//Tail ，符合则 Pop
			{
				char* a;
				char* b;
				a = s3.top->c.word;
				b = s1.top->last->c.word;
				while (*a == *b && *a!=0 && *b!=0)
				{
					a++;b++;
				}
				if (*a!=0 || *b!=0)
				{
					cout<<"tag dosen't match"<<endl;
					return false;
				}
				else
					s3.Pop();
			}
			if (G.expr[atoi(&action[1])].expr[0] == 'E' && G.expr[atoi(&action[1])].expr[2] == 'c')		//规约等式E->HcT，c为内容
				AddRecord(current,s1.top->last->c);

			if (G.expr[atoi(&action[1])].expr[0] == 'F')	//规约等式F->c="c"/Fc="c"，判断是否是表名，属性
			{
				Word a = s1.top->last->last->last->last->c;
				if (a.word[0] == 'd')	//数据库名字
				{
					db.name = new char[s1.top->last->c.size];
					memcpy(db.name,s1.top->last->c.word,s1.top->last->c.size);
					//cout<<"database name:"<<db.name<<endl;
				}
				if (a.word[0] == 't')	//表名
				{
					current = db.InsertTable();
					current->name = new char[s1.top->last->c.size];
					memcpy(current->name,s1.top->last->c.word,s1.top->last->c.size);
				}
				if (a.word[0] == 'p')	//属性
				{
					current->pro_num++;
					current->pro = (char**) realloc(current->pro,sizeof(char**)*current->pro_num);
					current->type = (int*) realloc(current->type,sizeof(int*)*current->pro_num);

					//获取名字
					a = s1.top->last->c;
					int l;
					for (l = 0 ; l < a.size ; l++)
						if (a.word[l] == '|')
							break;
					current->pro[current->pro_num-1] = new char[l+1];
					current->pro[current->pro_num-1][l] = 0;
					memcpy(current->pro[current->pro_num-1],a.word,l);
					//cout<<"property name:"<<db.t->pro[db.t->pro_num-1]<<endl;
					//获取类型
					if (a.word[l+1] == 'i')		//int
						current->type[current->pro_num-1] = 0;
					else						//char
					{
						//判断char大小
						for (; l < a.size ; l++)
							if (a.word[l] == '(')
								break;
						if (a.word[l] != '(')
							current->type[current->pro_num-1] = 1;
						else
							current->type[current->pro_num-1] = atoi(&a.word[l+1]);
					}
				}
			}

			//pop
			for(i = 1 ; G.expr[atoi(&action[1])].expr[i]!=0 ; i++)
			{
				s1.Pop();
				s2.Pop();
			}

			//非终结符进栈
			Word t;
			t.size = 2;
			t.word = new char[2];
			t.word[0] = G.expr[atoi(&action[1])].expr[0];
			t.word[1] = 0;
			s1.Push(t,-1);
			//锁定非终结符位置
			for (i = 0 ; i < G.simbol_num ; i++)
			{
				if (G.simbol[i] == t.word[0])
					break;
			}
			//转换状态
			s2.Push(t,atoi(G.form[s2.top->state][i]));
		}
		/*
		cout<<"**********stack 1:"<<endl;
		s1.Show(true);
		cout<<endl;
		cout<<"**********stack 2:"<<endl;
		s2.Show(false);
		cout<<endl;		*/
		//system("pause");
	}
	return false;
}

Table* Database::InsertTable()
{
	Table * a;
	a = new Table;
	a->next_table = t;
	a->db = this;
	t = a;
	return a;
}

void Database::AddRecord(Table * t,Word record)
{
	//cout<<record.word<<endl;
	int r_size = 0;
	int cur_size;
	for (int i = 0 ; i < t->pro_num ; i++)
		if(t->type[i] == 0)				//int
			r_size += sizeof(int);
		else							//char
			r_size += sizeof(char)*t->type[i];

	int pt = 0,nt;
	while (1)
	{
		cur_size = 0;
		t->record_num++;
		t->record = (char**) realloc(t->record,t->record_num*sizeof(char**));
		t->record[t->record_num-1] = new char[r_size];
		memset(t->record[t->record_num-1],0,r_size);
		for (int i = 0 ; i < t->pro_num ; i++)
		{
			//找到分隔符
			for (nt = pt ; record.word[nt] != '|' && record.word[nt] != 0 ; nt++)
				;

			if (t->type[i] == 0)		//int
			{
				int a = atoi(&record.word[pt]);
				memcpy(t->record[t->record_num-1]+cur_size,&a,sizeof(int));
				cur_size += sizeof(int);
				//cout<<"int ,"<<pt<<" "<<nt<<endl;
			}
			else
			{
				memcpy(t->record[t->record_num-1]+cur_size,&record.word[pt],nt-pt);
				cur_size += t->type[i];
				//cout<<"char() ,"<<pt<<" "<<nt<<endl;
			}
			pt = nt+1;
		}
		if (record.word[pt-1] == 0)
			break;
	}
	//cout<<"add record over********"<<endl<<endl;
}

void Database::Save()
{
	return;
}

//Table*********************************************
Table::Table()
{
	pro=NULL;
	type=NULL;
	record=NULL;
	next_table=NULL;
	db = NULL;
	record_num=0;
	pro_num = 0;
}

void Table::Show()
{
	//确定有没有where条件
	int loc_of_where;
	for (loc_of_where = 0 ; loc_of_where < argsCnt ; loc_of_where++)
		if (strcmp("where",args[loc_of_where]) == 0)
			break;
	//计算各个属性的列宽
	int *size;
	int width = 0;
	size = new int[pro_num+1];
	size[0] = 2;			//no列
	for (int i = 0 ; i < pro_num ; i++)
	{
		if (type[i] == 0)
			size[i+1] = 8;
		else
			size[i+1] = type[i];
	}

	//打印上边框
	cout<<' ';
	for (int i = 0 ; i < pro_num+1 ; i++)
	{
		for (int j = 0 ; j < size[i] ; j++)
			cout<<"-";
		if (i == pro_num)
			break;
		cout<<'-';
	}
	cout<<endl;
	//打印属性名字
	for (int i = 0 ; i < pro_num+1 ; i++)
	{
		cout<<'|';
		if (!i)
			cout<<"no";
		else
		{
			int len = 0;
			for (len = 0 ; pro[i-1][len] !=0 ; len++);
			cout<<pro[i-1];
			for (;len < size[i] ; len++)
				cout<<' ';
		}
	}
	cout<<'|'<<endl;
	//打印属性下边框
	cout<<'|';
	for (int i = 0 ; i < pro_num+1 ; i++)
	{
		for (int j = 0 ; j < size[i] ; j++)
			cout<<"-";
		if (i == pro_num)
			break;
		cout<<'-';
	}
	cout<<'|'<<endl;

	//打印每条记录
	int print_no = 0;
	char * fm;
	fm = NULL;
	if (loc_of_where < argsCnt)
		fm=WhereAnaylize(loc_of_where);

	int loc;
	for (int i = 0 ; i < record_num ; i++)
	{
		if (loc_of_where < argsCnt && !Match(record[i],fm,loc_of_where))
			continue;
		loc = 0;
		//打印序号
		cout<<'|';
		if (print_no < 10)
			cout<<print_no<<' ';
		else
			cout<<print_no;
		print_no++;
		//打印每个属性
		for (int j = 0 ; j < pro_num ; j++)
		{
			cout<<'|';
			if (type[j] == 0)
			{
				char *a;
				a = new char[8];
				itoa((int)record[i][loc],a,10);
				int len;
				for (len = 0 ; a[len] != 0 ; len++)
					;
				cout<<a;
				for (;len < size[j+1];len++)
					cout<<' ';
				
				loc += sizeof(int);
			}
			else
			{
				int len;
				for (len = 0 ; record[i][loc+len] != 0 ; len++)
					;

				cout<<&record[i][loc];
				loc += type[j];
				for (;len < size[j+1];len++)
					cout<<' ';
			}
		}
		cout<<'|'<<endl;
	}

	//打印表格下边框
	cout<<' ';
	for (int i = 0 ; i < pro_num+1 ; i++)
	{
		for (int j = 0 ; j < size[i] ; j++)
			cout<<"-";
		if (i == pro_num)
			break;
		cout<<'-';
	}
	cout<<endl;
	return;
}

void Table::ShowWithCondition(int loc_of_from)
{
	//确定有没有where条件
	int loc_of_where;
	for (loc_of_where = 0 ; loc_of_where < argsCnt ; loc_of_where++)
		if (strcmp("where",args[loc_of_where]) == 0)
			break;
	bool *choosen;
	int loc_of_last_true;
	choosen = new bool[pro_num+1];
	//初始化
	for (int i = 0 ; i < pro_num+1 ; i++)
		choosen[i+1] = false;
	choosen[0] = true;		//no列
	loc_of_last_true = 0;

	for (int i = 0 ; i < pro_num ; i++)
		for (int j = 1 ; j < loc_of_from ; j++)
			if (strcmp(args[j],pro[i]) == 0)
			{
				choosen[i+1] = true;
				loc_of_last_true = i+1;
			}

	//计算各个属性的列宽
	int *size;
	size = new int[pro_num+1];
	size[0] = 2;			//no列
	for (int i = 0 ; i < pro_num ; i++)
	{
		if (type[i] == 0)
			size[i+1] = 8;
		else
			size[i+1] = type[i];
	}

	//打印上边框
	cout<<' ';
	for (int i = 0 ; i < pro_num+1 ; i++)
	{
		if (!choosen[i])
			continue;
		for (int j = 0 ; j < size[i] ; j++)
			cout<<"-";
		if (i == loc_of_last_true)
			break;
		cout<<'-';
	}
	cout<<endl;
	//打印属性名字
	for (int i = 0 ; i < pro_num+1 ; i++)
	{
		if (!choosen[i])
			continue;
		cout<<'|';
		if (!i)
			cout<<"no";
		else
		{
			int len = 0;
			for (len = 0 ; pro[i-1][len] !=0 ; len++);
			cout<<pro[i-1];
			for (;len < size[i] ; len++)
				cout<<' ';
		}
	}
	cout<<'|'<<endl;

	//属性下边框
	cout<<'|';
	for (int i = 0 ; i < pro_num+1 ; i++)
	{
		if (!choosen[i])
			continue;
		for (int j = 0 ; j < size[i] ; j++)
			cout<<"-";
		if (i == loc_of_last_true)
			break;
		cout<<'-';
	}
	cout<<'|'<<endl;

	//打印每条记录
	char * fm;
	fm = NULL;
	if (loc_of_where < argsCnt)
		fm=WhereAnaylize(loc_of_where);
	int print_no = 0;
	int loc;
	for (int i = 0 ; i < record_num ; i++)
	{
		if (loc_of_where < argsCnt && !Match(record[i],fm,loc_of_where))
			continue;
		loc = 0;
		//打印序号
		cout<<'|';
		if (print_no < 10)
			cout<<print_no<<' ';
		else
			cout<<print_no;
		print_no++;
		//打印每个属性
		for (int j = 0 ; j < pro_num ; j++)
		{
			if (choosen[j+1])
				cout<<'|';
			if (type[j] == 0)
			{
				char *a;
				a = new char[8];
				itoa((int)record[i][loc],a,10);
				int len;
				for (len = 0 ; a[len] != 0 ; len++)
					;
				if (choosen[j+1])
				{
					cout<<a;
					for (;len < size[j+1];len++)
						cout<<' ';	
				}
				loc += sizeof(int);
			}
			else
			{
				int len;
				for (len = 0 ; record[i][loc+len] != 0 ; len++)
					;
				if (choosen[j+1])
				{
					cout<<&record[i][loc];
					for (;len < size[j+1];len++)
						cout<<' ';
				}
				loc += type[j];
			}
		}
		cout<<'|'<<endl;
	}

	//表格属性下边框
	cout<<' ';
	for (int i = 0 ; i < pro_num+1 ; i++)
	{
		if (!choosen[i])
			continue;
		for (int j = 0 ; j < size[i] ; j++)
			cout<<"-";
		if (i == loc_of_last_true)
			break;
		cout<<'-';
	}
	cout<<endl;
}

void Table::Insert()
{
	Word t;
	for (int i = 0 ; args[5][i] != 0 ;i++)
	{
		if (args[5][i] != ',')
			t.Insert(args[5][i]);
		else
			t.Insert('|');
	}
	db->AddRecord(this,t);
}

void Table::Delete()
{
	//获取波兰式
	int loc;
	int size = 0;
	int delete_num = 0;
	for (loc = 0 ; loc < argsCnt ; loc++)
		if (strcmp(args[loc],"where") == 0)
			break;
	if (loc == argsCnt)
	{
		cout<<"ERR,delete without where"<<endl;
		return;
	}

	//计算每条记录所占内存大小
	for (int i = 0 ; i < pro_num ; i++)
		if (type[i] == 0)
			size += sizeof(int);
		else
			size += type[i];

	char *fm;
	fm = WhereAnaylize(loc);
	for (int i = 0 ; i < record_num ; i++)
	{
		if (Match(record[i],fm,loc))
		{
			delete_num++;
			memcpy(record[i],record[record_num-1],size);
			free(record[record_num-1]);
			record_num--;
			i--;
		}
	}
	cout<<delete_num<<" rows effected"<<endl;
}

bool Table::Match(char * rcd,char* polan,int loc_of_where)
{
	//先计算表达式 property=value的值
	bool *result;	//表达式计算结果
	result = NULL;
	int num_of_exp = 0;	//表达式数量
	int loc_of_eq = 0;
	int type = -1;
	int offset = 0;
	//计算表达式
	for (int i = loc_of_where+1 ; i < argsCnt ; i++)
	{
		//是表达式
		if (strcmp(args[i],"and") == 0 || strcmp(args[i],"or") == 0 || args[i][0] == '(' || args[i][0] == ')')
			continue;
		//找到等号位置
		for (loc_of_eq = 0 ; args[i][loc_of_eq] != '=' && args[i][loc_of_eq] != 0 ; loc_of_eq++)
			;
		if (args[i][loc_of_eq]== 0)
		{
			cout<<"ERR,no '='"<<endl;
			return false;
		}
		args[i][loc_of_eq] = 0;
		//找到属性
		for (type = 0 ; type < pro_num ; type++)
			if (strcmp(args[i],pro[type]) == 0)
				break;
		if (type == pro_num)
		{
			cout<<"ERR,don't have property "<<args[i]<<endl;
			args[i][loc_of_eq] = '=';
			return false;
		}
		//找到属性，寻找属性偏移量
		offset = 0;
		for (int j = 0 ; j < type ; j++)
		{
			if (this->type[j] == 0)
				offset += sizeof(int);
			else
				offset += this->type[j];
		}
		//cout<<"offset:"<<offset<<endl;
		//找到该记录该属性，判断是相等
		num_of_exp++;
		result = (bool*) realloc(result,sizeof(bool)*num_of_exp);
		if (this->type[type] == 0)
			result[num_of_exp-1] = ((int)rcd[offset] == atoi(&args[i][loc_of_eq+1]));
		else
			result[num_of_exp-1] = !strcmp(&args[i][loc_of_eq+1],&rcd[offset]);
		args[i][loc_of_eq] = '=';
	}

	//根据波兰式，用堆栈计算
	Stack s;
	Word useless;
	bool res = result[0];
	for (int i = 0 ; polan[i] != 0 ; i++)
	{
		if (polan[i]>='0' && polan[i]<='9')
			s.Push(useless,result[polan[i]-'0']);
		else
		{
			if (polan[i] == 'a')
			{
				res = s.top->state && s.top->last->state;
				s.Pop();s.Pop();
				s.Push(useless,res);
			}
			else
			{
				res = s.top->state || s.top->last->state;
				s.Pop();s.Pop();
				s.Push(useless,res);
			}
		}
	}
	return res;
}

char * Table::WhereAnaylize(int loc_of_where)
{
	Stack s;
	Word useless;
	s.Push(useless,'#');
	int loc1,loc2,loc = loc_of_where+1;
	loc1 = 5;				//'#'号在优先表中的位置
	int no = 0;				//栈顶表达式编号
	char *result;
	result = new char;
	*result = 0;
	int res_len = 1;
	int i;

	while(1)
	{
		//确下一个待进栈符在优先表中的位置
		if (loc < argsCnt)
		{
			for (i = 0 ; i < 6 ; i++)
				if (strcmp(vt[i],args[loc]) == 0)
					break;
			i >= 6 ? loc2 = 4 : loc2 = i;
		}
		else
			loc2 = 5;
		
		//cout<<"loc1: "<<loc1<<" loc2: "<<loc2<<endl;

		if (priority[loc1][loc2] <= 0)		//进栈
		{
			//cout<<"进栈"<<endl;
			//进栈，表达式进e，否则进符号
			if (loc2 == 4)
				s.Push(useless,'e');
			else
				s.Push(useless,vt[loc2][0]);
			loc++;
		}
		else								//规约
		{
			//cout<<"规约"<<endl;
			//规约e
			if (s.top->state == 'e')
			{
				//cout<<"e 规约"<<endl;
				res_len++;
				result = (char*) realloc(result,sizeof(char)*res_len);
				result[res_len-2] = '0'+no++;
				result[res_len-1] = 0;
				s.Pop();
			}
			//规约（E）或e or e或e and e
			else if (s.top->state == ')')
			{
				//cout<<"()规约"<<endl;
				s.Pop();s.Pop();
			}
			else if (s.top->state == 'o' || s.top->state == 'a')
			{
				//cout<<"and or规约"<<endl;
				res_len++;
				result = (char*) realloc(result,sizeof(char)*res_len);
				result[res_len-2] = s.top->state;
				result[res_len-1] = 0;
				s.Pop();
			}
		}
		//栈顶终结符在优先表中的位置
		for (i = 0 ; i < 6 ; i++)
			if (vt[i][0] == s.top->state)
				break;
		loc1 = i;
		//s.Show(false);
		if (s.top->state == '#' && s.top->last != NULL && s.top->last->state == '#')
			break;
	}
	return result;
}

//Word
Word::Word()
{
	size = 1;
	type = -1;
	word = (char*)malloc(sizeof(char));
	word[0] = 0;
}

void Word::Insert(char c)
{
	word = (char*) realloc(word,sizeof(char)*++size);
	word[size-2] = c;
	word[size-1] = 0;
}

bool Word::IS_TAG()
{
	if(size == 6)	//Table
	{
		if (word[0] == 'T' && word[1] == 'a' && word[2] == 'b' && word[3] == 'l' && word[4] == 'e')
			return true;
		return false;
	}
	if(size == 7)	//Record
	{
		if (word[0] == 'R' && word[1] == 'e' && word[2] == 'c' && word[3] == 'o' && word[4] == 'r' && word[5]=='d')
			return true;
		return false;
	}
	if (size == 9)	//Database,Property
	{
		if (word[0] == 'D' && word[1] == 'a' && word[2] == 't' && word[3] == 'a' && word[4] == 'b' && word[5]=='a' && word[6]=='s' && word[7]=='e')
			return true;
		if (word[0] == 'P' && word[1] == 'r' && word[2] == 'o' && word[3] == 'p' && word[4] == 'e' && word[5]=='r' && word[6]=='t' && word[7]=='y')
			return true;
		return false;
	}
	return false;
}

//StackNode*****************************************
StackNode::StackNode()
{
	last = NULL;
}

//Stack*********************************************
Stack::Stack()
{
	size = 0;
	top = NULL;
}

Word Stack::Pop()
{
	Word c;
	c.size = -1;
	if (top == NULL)
		return c;
	c = top->c;
	top = top->last;
	size--;
	return c;
}

void Stack::Push(Word c,int s)
{
	StackNode* p;
	p = new StackNode;
	p->c = c;
	p->last = top;
	p->state = s;
	top = p;
	size++;
}

bool Stack::IsEmpty()
{
	if (top)
		return false;
	return true;
}

void Stack::Show(bool tag)
{
	cout<<"Stack size: "<<size<<"	Element:"<<endl;
	StackNode *p;
	p = top;
	while (p)
	{
		if (tag)
			cout<<p->c.word<<" ";
		else
			cout<<(char)p->state<<" ";
		p = p->last;
	}
	cout<<endl;
}