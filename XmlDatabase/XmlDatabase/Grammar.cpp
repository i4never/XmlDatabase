#include "Grammar.h"
#include <iostream>
#include <iomanip>

using namespace std;

//Experssion*********************************
Experssion::Experssion()
{
	memset (expr,0,128);
	offset = 0;
	no = 0;
}

bool Experssion::operator==(Experssion t)
{
	if (offset != t.offset)
		return false;
	
	int i;
	for (i = 0 ; i < 128 ; i++)
	{
		if (expr[i] == 0 || t.expr[i] == 0)
			break;
		if (expr[i] != t.expr[i])
			return false;
	}
	if (expr[i] != 0 || t.expr[i] != 0)
		return false;
	return true;
}

void Experssion::Show()
{
	cout<<"no."<<no<<"expr:"<<expr<<" offset "<<offset<<endl;
}

//ItemSet************************************
ItemSet::ItemSet()
{
	no = 0;
	num = 0;
	expr = NULL;
}

void ItemSet::AddExpr(Experssion new_expr)
{
	expr = (Experssion*)realloc(expr,(++num)*sizeof(Experssion));
	memcpy(&expr[num-1],&new_expr,sizeof(Experssion));
}

void ItemSet::Show()
{
	cout<<"itemset num:"<<num<<endl;
	for (int i = 0 ; i < num ; i++)
		expr[i].Show();
}

bool ItemSet::operator==(ItemSet t)
{
	if (num != t.num)
		return false;

	int equal_num = 0;
	int i,j;
	for (i = 0 ; i < num ; i++)
	{
		for (j = 0 ; j < num ; j++)
		{
			if (expr[i] == t.expr[j])
				break;
		}
		//找到相等等式
		if (j != num)
			continue;
		else
			return false;
	}
	return true;
}

//ArcNode************************************
ArcNode::ArcNode()
{
	simbol = 0;
	next = NULL;
	index = -1;
}

//VNode*************************************
VNode::VNode()
{
	itemset = new ItemSet();
	firstarc = NULL;
}

void VNode::AddArcNode(char c,int in)
{
	ArcNode *q;
	q = new ArcNode;
	q->simbol = c;
	q->index = in;
	q->next = firstarc;
	firstarc = q;
}

//Graphis***********************************
Graphis::Graphis ()
{
	vexnum = 0;
	vertices = NULL;
}

VNode* Graphis::AddVNode()
{
	vertices = (VNode*)realloc(vertices,(++vexnum)*sizeof(VNode));
	vertices[vexnum-1].itemset = new ItemSet;
	vertices[vexnum-1].itemset->no = vexnum - 1;
	vertices[vexnum-1].firstarc = NULL;
	return &vertices[vexnum-1];
}

void Graphis::Show()
{
	cout<<"**********Show Graphis Start**********"<<endl;
	cout<<"vexnum : "<<vexnum<<endl;
	for (int i = 0 ; i < vexnum ; i++)
	{
		cout<<"node "<<i<<" ";
		vertices[i].itemset->Show();
		cout<<endl;
	}

	for (int i = 0 ; i < vexnum ; i++)
	{
		cout<<i;
		ArcNode *p;
		p = vertices[i].firstarc;
		while (p != NULL)
		{
			cout<<"->"<<p->simbol<<" "<<vertices[p->index].itemset->no;
			p = p->next;
		}
		cout<<endl;
	}
	cout<<"**********Show Graphis End**********"<<endl;
}

//Grammar***********************************
Grammar::Grammar()
{
	num = 0;
	simbol_num = 0;
	expr = NULL;
	state = NULL;
	simbol = NULL;
	form = NULL;
}

bool Grammar::Open(string filename)
{
	file.open(filename);
	return file.is_open();
}

void Grammar::Read()
{
	file>>num;
	//cout<<"num:"<<num<<endl;
	expr = new Experssion[num];
	memset(expr,0,num*sizeof(Experssion));
	string ul;
	//useless
	getline(file,ul);
	for (int i = 0 ; i < num ; i++)
	{
		file.getline(expr[i].expr,128);
		expr[i].offset = 0;
		expr[i].no = i;
	}

	for (int i = 0 ; i < num ; i++)
		for (int j = 1 ; ; j++)
		{
			expr[i].expr[j] = expr[i].expr[j+2];
			if (expr[i].expr[j] == 0)
			{
				expr[i].expr[j+1] = expr[i].expr[j+2] = 0;
				break;
			}
		}
}

void Grammar::InitGraphis()
{
	g.AddVNode();
	for (int i = 0 ; i < num ; i++)
	{
		if (expr[i].expr[0] == 'S')
			g.vertices[0].itemset->AddExpr(expr[i]);
	}
	ExpandItemSet(g.vertices[0].itemset);

}

void Grammar::GenerateDFA()
{
	for (int i = 0 ; i < g.vexnum ; i++)
	{
		NewItemSet(i);
	}
	g.Show();
}

void Grammar::NewItemSet(int index)
{
	VNode * vpt = &g.vertices[index];
	ItemSet *t;
	char * move;
	move = new char[vpt->itemset->num];
	memset (move,0,vpt->itemset->num);
	for (int i = 0 ; i < vpt->itemset->num ; i++)
	{
		move[i] = vpt->itemset->expr[i].expr[vpt->itemset->expr[i].offset+1];
		move[i] == 0 ? move[i] = ' ' : move[i] = move[i];
	}

	for (int i = 0 ; i < vpt->itemset->num-1 ; i++)
		for (int j = i+1 ; j < vpt->itemset->num-1 ; j++)
			if (move[i] == move[j])
				move[j] = ' ';

	for (int i = 0 ; i < vpt->itemset->num && move[i] != 0; i++)
	{
		int j;
		if (move[i] == ' ')
			continue;
		t = new ItemSet;
		
		for (j = 0 ; j < vpt->itemset->num ; j++)
		{
			if (vpt->itemset->expr[j].expr[vpt->itemset->expr[j].offset+1] == move[i])
			{
				t->AddExpr(vpt->itemset->expr[j]);
				t->expr[t->num-1].offset++;
			}
		}
		
		ExpandItemSet(t);

		for (j = 0 ; j < g.vexnum ; j++)
			if (*t == *(g.vertices[j].itemset))
				break;

		
		if (j == g.vexnum)	//该项目集规范簇不重复,加入DFA中
		{
			VNode * p;
			p = g.AddVNode();
			vpt = &g.vertices[index];
			vpt->AddArcNode(move[i],g.vexnum-1);
			p->itemset->num = t->num;
			p->itemset->expr = t->expr;
		}
		else
		{
			vpt->AddArcNode(move[i],j);
		}
	}
}

void Grammar::ExpandItemSet(ItemSet *t)
{
	bool is_change = true;
	//项目集有变化则扩展
	while (is_change)
	{
		is_change = false;
		//扫描项目集的每个候选式
		for (int i = 0 ; i < t->num ; i++)
		{
			//是非终结符
			if(!IsTerminal(t->expr[i].expr[t->expr[i].offset+1]))
			{
				//在候选式中找以该非终结符开头的式子
				for (int j = 0 ; j < num ; j++)
				{
					if (expr[j].expr[0] == t->expr[i].expr[t->expr[i].offset+1])
					{
						//判断该式子是否已经在项目集内
						int k = 0;
						for (k = 0 ; k < t->num ; k++)
							if (t->expr[k] == expr[j])
								break;
						if (k == t-> num)
						{
							is_change = true;
							t->AddExpr(expr[j]);
						}
					}
				}
			}
		}
	}
}

void Grammar::GenerateForm()
{
	//初始化预测分析表行/列
	state = new int[g.vexnum];
	int i,j,k;
	for (i = 0 ; i < g.vexnum ; i++)
		state[i] = i;

	for (i = 0 ; i < num ; i++)
	{
		for (j = 0 ; j < 128 && expr[i].expr[j] != 0 ; j++)
		{
			for (k = 0 ; k < simbol_num ; k++)
				if (simbol[k] == expr[i].expr[j])
					break;
			if (k == simbol_num)
			{
				simbol = (char*) realloc(simbol,(++simbol_num)*sizeof(char));
				simbol[simbol_num-1] = expr[i].expr[j];
			}
		}
	}
	simbol = (char*) realloc(simbol,(++simbol_num)*sizeof(char));
	simbol[simbol_num-1] = '#';

	//调整列，将非terminal放在最后
	for (i = 0 ; i < simbol_num-1 ; i++)
		if (!IsTerminal(simbol[i]))
		{
			for (j = simbol_num-1 ; j > i ; j--)
				if (IsTerminal(simbol[j]))
				{
					char c = simbol[j];
					simbol[j] = simbol[i];
					simbol[i] = c;
					break;
				}
			if (j == i)
				break;
		}

	//预测分析表初始化
	form = new char**[g.vexnum];
	for (i = 0 ; i < g.vexnum ; i++)
	{
		form[i] = new char*[simbol_num];
		for (j = 0 ; j < simbol_num ; j++)
		{
			form[i][j] = new char[5];
			memset(form[i][j],0,5);
		}
	}

	//初始化first，follow集
	//找到非终结符个数
	for (i = 0 ; i < simbol_num ; i++)
		if (! IsTerminal(simbol[i]))
			break;
	loc = i;
	first = new bool*[simbol_num-loc];
	follow = new bool*[simbol_num-loc];
	for (i = 0 ; i < simbol_num - loc ; i++)
	{
		first[i] = new bool[loc];
		follow[i] = new bool[loc];
	}
	//初始化first，follow集
	
	for (i = 0 ; i < simbol_num-loc ; i++)
		for (j = 0 ; j < loc ; j++)
		{
			first[i][j] = false;
			follow[i][j] = false;
		}

	//计算first，follow集
	First();
	Follow();

	//遍历图，生成预测分析表
	for (i = 0 ; i < g.vexnum ; i++)
	{
		//处理移进项目
		for (ArcNode *p = g.vertices[i].firstarc ; p != NULL ; p = p->next)
		{
			//定位
			for (j = 0 ; j < simbol_num ; j++)
				{
					if (simbol[j] == p->simbol)
						break;
				}
			if (IsTerminal(p->simbol))
			{
				//ACTION，移进项目
				if (form[i][j][0] != 0)
				{
					cout<<"文法有冲突,冲突位置："<<i<<" "<<j<<endl;
					goto L;
				}
				form[i][j][0] = 's';
				itoa(&form[i][j][1],g.vertices[p->index].itemset->no);		
			}
			else
			{
				//GOTO
				itoa(&form[i][j][0],g.vertices[p->index].itemset->no);
			}
		}
		//无后续符号的候选式（查看follow集，规约）
		/*
		for (j = 0 ; j < g.vertices[i].itemset->num ; j++)
		{
			if (g.vertices[i].itemset->expr[j].expr[g.vertices[i].itemset->expr[j].offset+1] != 0)
				break;
			if (g.vertices[i].itemset->expr[j].expr[0] == 'S')
			{
				for (k = 0 ; k < simbol_num && IsTerminal(simbol[k]); k++)
				{
					if (simbol[k] != '#')
						continue;
					//ACC
					form[i][k][0] = 'a';
					form[i][k][1] = 'c';
					form[i][k][2] = 'c';
				}
				continue;
			}
			for (k = 0 ; k < simbol_num && IsTerminal(simbol[k]); k++)
			{
				//ACTION，规约项目
				if (form[j][k][0] != 0)
				{
					cout<<"文法有规约冲突"<<endl;
					goto L;
				}
				form[i][k][0] = 'r';
				itoa(&form[i][k][1],g.vertices[i].itemset->expr[j].no);
			}
		}*/
		
		//处理规约项目（查看follow集，规约）
		for (j = 0 ; j < g.vertices[i].itemset->num ; j++)
		{
			if (g.vertices[i].itemset->expr[j].expr[g.vertices[i].itemset->expr[j].offset+1] != 0)
				continue;
			//文法开始符号，acc
			if (g.vertices[i].itemset->expr[j].expr[0] == 'S')
			{
				for (k = 0 ; k < loc; k++)
				{
					if (simbol[k] != '#')
						continue;
					//ACC
					form[i][k][0] = 'a';
					form[i][k][1] = 'c';
					form[i][k][2] = 'c';
				}
				continue;
			}

			int loc1;
			for (loc1 = loc ; loc1 < simbol_num ; loc1++)
				if (g.vertices[i].itemset->expr[j].expr[0] == simbol[loc1])
					break;

			//相应非终结符follow处填上规约项目
			for (k = 0 ; k < loc; k++)
			{
				//ACTION，规约项目，检查每个表达式的follow
				if (follow[loc1 - loc][k])
				{
					if (form[i][k][0] != 0)
					{
						cout<<"文法有规约冲突,冲突位置："<<i<<" "<<k<<endl;
					}
					form[i][k][0] = 'r';
					itoa(&form[i][k][1],g.vertices[i].itemset->expr[j].no);
				}
			}
		}
	}
L:
	//显示预测分析表
	cout<<"**********Show Form Start**********"<<endl;
	cout<<' '<<setw(5);
	for (i = 0 ; i < simbol_num ; i++)
		cout<<simbol[i]<<setw(5);

	cout<<endl;
	for (i = 0 ; i < g.vexnum ; i++)
	{
		cout << setiosflags(ios::left);             //输出左对齐
        cout <<i<< setw(5);
		for (j = 0 ; j < simbol_num ; j++)
			cout<<form[i][j]<<setw(5);
		cout<<endl;
	}
	cout<<"**********Show Form End**********"<<endl;
}

void Grammar::itoa(char* dst,int num)
{
	//cout<<"num "<<num;
	char temp[4];
	memset(temp,0,4);
	for (int i = 0 ; i < 3 ; i++)
	{
		if (num == 0)
			break;
		temp[i] = num%10 + '0';
		num/=10;
	}

	//cout<<"itoa******"<<temp<<endl;
	
	if (temp[1] == 0)
	{
		memcpy(dst,&temp[0],1);
		return ;
	}
	if (temp[2] == 0)
	{
		char c = temp[1];
		temp[1] = temp[0];
		temp[0] = c;
		memcpy(dst,&temp[0],2);
		return ;
	}
	char c = temp[2];
	temp[2] = temp[0];
	temp[0] = c;
	memcpy(dst,&temp[0],3);
	return ;

}

void Grammar::First()
{
	int loc1,loc2;
	//扫描一遍所有表达式，加入形如A->aB的first
	for (int i = 0 ; i < num ; i++)
	{
		if (IsTerminal(expr[i].expr[1]))	//将该终结符加入非终结符的first中
		{
			for (loc1 = loc ; loc1 < simbol_num ; loc1++)
				if (simbol[loc1] == expr[i].expr[0])
					break;
			for (loc2 = 0 ; loc2 < loc ; loc2++)
				if (simbol[loc2] == expr[i].expr[1])
					break;
			first[loc1-loc][loc2]=true;
		}
	}

	bool change = true;
	while (change)
	{
		change = false;
		//扫描所有表达式，形如A->B...的，把firstB加入firstA
		for  (int i = 0 ; i < num ; i++)
		{
			if (!IsTerminal(expr[i].expr[1]))	//非终结符开头
			{
				//找到位置
				for (loc1 = loc ; loc1 < simbol_num ; loc1++)
					if (simbol[loc1] == expr[i].expr[0])
						break;
				for (loc2 = loc ; loc2 < simbol_num ; loc2++)
					if (simbol[loc2] == expr[i].expr[1])
						break;
				//合并
				for (int j = 0 ; j < loc ; j++)
				{
					if (first[loc1 - loc][j])
						continue;
					if (first[loc2 - loc][j])
					{
						change = true;
						first[loc1 - loc][j] = true;
					}
				}
			}
		}
	}

	//显示first集合
	cout<<"  ";
	for (int i = 0 ; i < loc ; i++)
		cout<<simbol[i]<<" ";
	cout<<endl;
	for (int i = 0 ; i < simbol_num-loc ; i++)
	{
		cout<<simbol[i+loc]<<" ";
		for (int j = 0 ; j < loc ; j++)
			cout<<first[i][j]<<" ";
		cout<<endl;
	}
}

void Grammar::Follow()
{
	int loc1,loc2;
	//‘#’置入文法开始符号follow中
	for (loc1 = loc ; loc1 < simbol_num ; loc1++)
		if (simbol[loc1] == 'S')
			break;
	for (loc2 = 0 ; loc2 < loc ; loc2++)
		if (simbol[loc2] == '#')
			break;
	follow[loc1-loc][loc2]=true;


	//扫描一遍所有表达式，加入形如A->aBb的follow
	for (int i = 0 ; i < num ; i++)
	{
		int j;
		//每个表达式，检查是否有前一个是非终结符，后一个是终结符的情况
		for (j = 1 ; expr[i].expr[j+1] != 0 ; j++)
			if (!IsTerminal(expr[i].expr[j]) && IsTerminal(expr[i].expr[j+1]))
				break;
		if (expr[i].expr[j+1] == 0)
			continue;

		//将该终结符加入非终结符的follow中
		for (loc1 = loc ; loc1 < simbol_num ; loc1++)
			if (simbol[loc1] == expr[i].expr[j])
				break;
		for (loc2 = 0 ; loc2 < loc ; loc2++)
			if (simbol[loc2] == expr[i].expr[j+1])
				break;
		follow[loc1-loc][loc2]=true;
	}

	//形如A->asdfB,将followA加入followB
	//形如A->BC,将firstC加入followB
	bool change = true;
	while (change)
	{
		change = false;
		int j;
		for (int i = 0 ; i < num ; i++)
		{
			//第一种情况
			for (j = 1 ; expr[i].expr[j+1] != 0 ; j++)
				;
			if (IsTerminal(expr[i].expr[j]))
				continue;

			//找到位置
			for (loc1 = loc ; loc1 < simbol_num ; loc1++)
				if (simbol[loc1] == expr[i].expr[j])
					break;
			for (loc2 = loc ; loc2 < simbol_num ; loc2++)
				if (simbol[loc2] == expr[i].expr[0])
					break;
			//合并follow
			for (int k = 0 ; k < loc ; k++)
			{
				if (follow[loc1 - loc][k])
					continue;
				if (follow[loc2 - loc][k])
				{
					change = true;
					follow[loc1 - loc][k] = true;
				}
			}

			//第二种情况
			for (j = 1 ; expr[i].expr[j+1] != 0 ; j++)
			{
				if (!IsTerminal(expr[i].expr[j]) && !IsTerminal(expr[i].expr[j+1]))
				{
					//找到位置
					for (loc1 = loc ; loc1 < simbol_num ; loc1++)
						if (simbol[loc1] == expr[i].expr[j])
							break;
					for (loc2 = loc ; loc2 < simbol_num ; loc2++)
						if (simbol[loc2] == expr[i].expr[j+1])
							break;
					//first加入follow
					for (int k = 0 ; k < loc ; k++)
					{
						if (follow[loc1 - loc][k])
							continue;
						if (first[loc2 - loc][k])
						{
							change = true;
							follow[loc1 - loc][k] = true;
						}
					}	
				}
			}
		}
	}


	//显示follow集合
	cout<<"  ";
	for (int i = 0 ; i < loc ; i++)
		cout<<simbol[i]<<" ";
	cout<<endl;
	for (int i = 0 ; i < simbol_num-loc ; i++)
	{
		cout<<simbol[i+loc]<<" ";
		for (int j = 0 ; j < loc ; j++)
			cout<<follow[i][j]<<" ";
		cout<<endl;
	}
}