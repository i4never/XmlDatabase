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
	//Ԥ����sql����������ȱ�-1С�ڣ�0��Ч��1����
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

	//���ļ�
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
		//��λ��from���ҵ�table
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
		//��λ��from���ҵ�table
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

//Ԥ����
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
	while ( IsSpace(*p) && *p ) ++p;	/* ȥ������Ŀո� */
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
		pinput = TrimLeft( pinput );				//���ȥ������ո�

		pKeyToken = NextKeyToken(pinput);			//��������λ�ô�pinput��pKeyToken-1
		if ( pKeyToken > pinput )					//�������
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
		
		if ( IsKeyToken( *pKeyToken ) )				//�ų�pKeyTokenָ��\\�����
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
	//ȥ��ǰ��
	while (c==' ' || c=='\t' || c=='\n')
		c = file.get();

	//�����ļ�ĩβ
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
		//c���ǽ��
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
	Stack s1,s2,s3;		//s1:����ջ,s2:״̬ջ
	temp.size = 2;
	temp.word = new char[2];
	temp.word[0] = '#';
	temp.word[1] = 0;
	s1.Push(temp,-1);
	s2.Push(temp,0);
	
	temp = Get_next_word();
	while (!s1.IsEmpty())
	{
		//�ҵ���һ���ս����simbol�е�λ��
		for (i = 0 ; i < G.simbol_num ; i++)
		{
			if ((temp.type == 'a' && G.simbol[i] == temp.word[0]) || (temp.type == G.simbol[i]))
				break;
		}
		memcpy(action,G.form[s2.top->state][i],5);
		//cout<<"location: "<<s2.top->state<<" "<<i<<"action:"<<action<<endl;
		if (action[0] == 'a' && action[1] == 'c' && action[2] == 'c')	//�����ɹ�
		{
			cout<<"Grammar anaylize success"<<endl<<endl;
			return true;
		}
		if (action[0] == 0)		//������Ϊ�գ����﷨����
		{
			cout<<"anaylize error!"<<endl;
			return false;
		}
		if (action[0] == 's')	//�ƽ���Ŀ
		{
			s1.Push(temp,-1);
			s2.Push(temp,atoi(&action[1]));
			if (temp.word[0]!='#')
				temp = Get_next_word();
		}
		else if (action[0] == 'r')	//��Լ��Ŀ
		{
			//cout<<"use :"<<G.expr[atoi(&action[1])].expr<<" ��Լ"<<endl;
			//***********xml���У�ʹ�ö�ջs3���ж�Head��Tail�Ƿ�ƥ�䣬��Լ��H��Tʱ��Ҫ�ж��Ƿ��ǩ����
			if (G.expr[atoi(&action[1])].expr[0] == 'H')	//Head Push
			{
				s3.Push(s1.top->last->last->c,-1);
			}
			if (G.expr[atoi(&action[1])].expr[0] == 'T')	//Tail �������� Pop
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
			if (G.expr[atoi(&action[1])].expr[0] == 'E' && G.expr[atoi(&action[1])].expr[2] == 'c')		//��Լ��ʽE->HcT��cΪ����
				AddRecord(current,s1.top->last->c);

			if (G.expr[atoi(&action[1])].expr[0] == 'F')	//��Լ��ʽF->c="c"/Fc="c"���ж��Ƿ��Ǳ���������
			{
				Word a = s1.top->last->last->last->last->c;
				if (a.word[0] == 'd')	//���ݿ�����
				{
					db.name = new char[s1.top->last->c.size];
					memcpy(db.name,s1.top->last->c.word,s1.top->last->c.size);
					//cout<<"database name:"<<db.name<<endl;
				}
				if (a.word[0] == 't')	//����
				{
					current = db.InsertTable();
					current->name = new char[s1.top->last->c.size];
					memcpy(current->name,s1.top->last->c.word,s1.top->last->c.size);
				}
				if (a.word[0] == 'p')	//����
				{
					current->pro_num++;
					current->pro = (char**) realloc(current->pro,sizeof(char**)*current->pro_num);
					current->type = (int*) realloc(current->type,sizeof(int*)*current->pro_num);

					//��ȡ����
					a = s1.top->last->c;
					int l;
					for (l = 0 ; l < a.size ; l++)
						if (a.word[l] == '|')
							break;
					current->pro[current->pro_num-1] = new char[l+1];
					current->pro[current->pro_num-1][l] = 0;
					memcpy(current->pro[current->pro_num-1],a.word,l);
					//cout<<"property name:"<<db.t->pro[db.t->pro_num-1]<<endl;
					//��ȡ����
					if (a.word[l+1] == 'i')		//int
						current->type[current->pro_num-1] = 0;
					else						//char
					{
						//�ж�char��С
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

			//���ս����ջ
			Word t;
			t.size = 2;
			t.word = new char[2];
			t.word[0] = G.expr[atoi(&action[1])].expr[0];
			t.word[1] = 0;
			s1.Push(t,-1);
			//�������ս��λ��
			for (i = 0 ; i < G.simbol_num ; i++)
			{
				if (G.simbol[i] == t.word[0])
					break;
			}
			//ת��״̬
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
			//�ҵ��ָ���
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
	//ȷ����û��where����
	int loc_of_where;
	for (loc_of_where = 0 ; loc_of_where < argsCnt ; loc_of_where++)
		if (strcmp("where",args[loc_of_where]) == 0)
			break;
	//����������Ե��п�
	int *size;
	int width = 0;
	size = new int[pro_num+1];
	size[0] = 2;			//no��
	for (int i = 0 ; i < pro_num ; i++)
	{
		if (type[i] == 0)
			size[i+1] = 8;
		else
			size[i+1] = type[i];
	}

	//��ӡ�ϱ߿�
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
	//��ӡ��������
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
	//��ӡ�����±߿�
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

	//��ӡÿ����¼
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
		//��ӡ���
		cout<<'|';
		if (print_no < 10)
			cout<<print_no<<' ';
		else
			cout<<print_no;
		print_no++;
		//��ӡÿ������
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

	//��ӡ����±߿�
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
	//ȷ����û��where����
	int loc_of_where;
	for (loc_of_where = 0 ; loc_of_where < argsCnt ; loc_of_where++)
		if (strcmp("where",args[loc_of_where]) == 0)
			break;
	bool *choosen;
	int loc_of_last_true;
	choosen = new bool[pro_num+1];
	//��ʼ��
	for (int i = 0 ; i < pro_num+1 ; i++)
		choosen[i+1] = false;
	choosen[0] = true;		//no��
	loc_of_last_true = 0;

	for (int i = 0 ; i < pro_num ; i++)
		for (int j = 1 ; j < loc_of_from ; j++)
			if (strcmp(args[j],pro[i]) == 0)
			{
				choosen[i+1] = true;
				loc_of_last_true = i+1;
			}

	//����������Ե��п�
	int *size;
	size = new int[pro_num+1];
	size[0] = 2;			//no��
	for (int i = 0 ; i < pro_num ; i++)
	{
		if (type[i] == 0)
			size[i+1] = 8;
		else
			size[i+1] = type[i];
	}

	//��ӡ�ϱ߿�
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
	//��ӡ��������
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

	//�����±߿�
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

	//��ӡÿ����¼
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
		//��ӡ���
		cout<<'|';
		if (print_no < 10)
			cout<<print_no<<' ';
		else
			cout<<print_no;
		print_no++;
		//��ӡÿ������
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

	//��������±߿�
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
	//��ȡ����ʽ
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

	//����ÿ����¼��ռ�ڴ��С
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
	//�ȼ�����ʽ property=value��ֵ
	bool *result;	//���ʽ������
	result = NULL;
	int num_of_exp = 0;	//���ʽ����
	int loc_of_eq = 0;
	int type = -1;
	int offset = 0;
	//������ʽ
	for (int i = loc_of_where+1 ; i < argsCnt ; i++)
	{
		//�Ǳ��ʽ
		if (strcmp(args[i],"and") == 0 || strcmp(args[i],"or") == 0 || args[i][0] == '(' || args[i][0] == ')')
			continue;
		//�ҵ��Ⱥ�λ��
		for (loc_of_eq = 0 ; args[i][loc_of_eq] != '=' && args[i][loc_of_eq] != 0 ; loc_of_eq++)
			;
		if (args[i][loc_of_eq]== 0)
		{
			cout<<"ERR,no '='"<<endl;
			return false;
		}
		args[i][loc_of_eq] = 0;
		//�ҵ�����
		for (type = 0 ; type < pro_num ; type++)
			if (strcmp(args[i],pro[type]) == 0)
				break;
		if (type == pro_num)
		{
			cout<<"ERR,don't have property "<<args[i]<<endl;
			args[i][loc_of_eq] = '=';
			return false;
		}
		//�ҵ����ԣ�Ѱ������ƫ����
		offset = 0;
		for (int j = 0 ; j < type ; j++)
		{
			if (this->type[j] == 0)
				offset += sizeof(int);
			else
				offset += this->type[j];
		}
		//cout<<"offset:"<<offset<<endl;
		//�ҵ��ü�¼�����ԣ��ж������
		num_of_exp++;
		result = (bool*) realloc(result,sizeof(bool)*num_of_exp);
		if (this->type[type] == 0)
			result[num_of_exp-1] = ((int)rcd[offset] == atoi(&args[i][loc_of_eq+1]));
		else
			result[num_of_exp-1] = !strcmp(&args[i][loc_of_eq+1],&rcd[offset]);
		args[i][loc_of_eq] = '=';
	}

	//���ݲ���ʽ���ö�ջ����
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
	loc1 = 5;				//'#'�������ȱ��е�λ��
	int no = 0;				//ջ�����ʽ���
	char *result;
	result = new char;
	*result = 0;
	int res_len = 1;
	int i;

	while(1)
	{
		//ȷ��һ������ջ�������ȱ��е�λ��
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

		if (priority[loc1][loc2] <= 0)		//��ջ
		{
			//cout<<"��ջ"<<endl;
			//��ջ�����ʽ��e�����������
			if (loc2 == 4)
				s.Push(useless,'e');
			else
				s.Push(useless,vt[loc2][0]);
			loc++;
		}
		else								//��Լ
		{
			//cout<<"��Լ"<<endl;
			//��Լe
			if (s.top->state == 'e')
			{
				//cout<<"e ��Լ"<<endl;
				res_len++;
				result = (char*) realloc(result,sizeof(char)*res_len);
				result[res_len-2] = '0'+no++;
				result[res_len-1] = 0;
				s.Pop();
			}
			//��Լ��E����e or e��e and e
			else if (s.top->state == ')')
			{
				//cout<<"()��Լ"<<endl;
				s.Pop();s.Pop();
			}
			else if (s.top->state == 'o' || s.top->state == 'a')
			{
				//cout<<"and or��Լ"<<endl;
				res_len++;
				result = (char*) realloc(result,sizeof(char)*res_len);
				result[res_len-2] = s.top->state;
				result[res_len-1] = 0;
				s.Pop();
			}
		}
		//ջ���ս�������ȱ��е�λ��
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