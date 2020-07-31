// 判断两个正则表达式的关系
#include<set>
#include<stack>
#include<queue>
#include<string.h>
#include<string>
#include<stdio.h>
#include<iostream>
#include<string>
using namespace std;

#define MAX 128  // 最多的状态数量

// =====================1. 中缀表达式转为后缀表达式=====================
// 在字符串s的第n位前插入字符ch
void insert(string &s, int n, char ch)
{
    if(n < s.size())
    {
        s += '0';
        for(int i = s.size() - 1; i > n; i--)
        {
            s[i] = s[i - 1];
        }
        s[n] = ch;
    }	
}

// 预处理 对字符串s进行预处理，在第一位是操作数,'*','+','?','E'或')'且第二位是操作数或'('之间加入连接符'.'
void preprocess(string &s)
{	
    int i = 0 , length = s.size();	
    while(i < length)
    {
        if((s[i] >= 'a' && s[i] <= 'z') || s[i] == '*' || s[i] == '+' || s[i] == '?' || s[i] == 'E' || s[i] == ')')
        {
            if((s[i + 1] >= 'a' && s[i + 1] <= 'z') || s[i+1] == 'E' || s[i + 1] == '(')
            {
                insert(s, i+1 , '.');
                length ++;
            }
        }
        i++;
    }
}

// 优先级 中缀转后缀时用到的优先级比较，即为每个操作符赋一个权重，通过权重大小比较优先级
int priority(char ch)
{
    if(ch == '*' || ch == '+' || ch == '?') { return 3; }	
    if(ch == '.') { return 2; } // 连接	
    if(ch == '|') { return 1; }
    if(ch == '(') { return 0; }
}

// 正则表达式从中缀表达式转后缀表达式
string infixToSuffix(string s)
{
    preprocess(s);			// 预处理
    string str;				// 后缀字符串
    stack<char> opstack;	// 运算符栈

    for(int i = 0; i < s.size(); i++)   // 遍历中缀表达式的字符
    {	
        if((s[i] >= 'a' && s[i] <= 'z') || s[i] == 'E')	// 操作数:直接输出
        {
            str += s[i];
        } 
        else							// 运算符
        {
            if(s[i] == '(')			    // 左括号 压栈
            {
                opstack.push(s[i]);
            } 
            else if(s[i] == ')')	    // 右括号
            {
                char ch = opstack.top();
                while(ch != '(')		// 栈中元素出栈，直到栈顶为左括号
                {
                    str += ch;
                    opstack.pop();
                    ch = opstack.top();
                }
                opstack.pop();		    // 左括号出栈 
            }
            else					    // 其他操作符
            {
                if(!opstack.empty())	// 栈非空
                {
                    char ch = opstack.top();
                    while(priority(ch) >= priority(s[i]))	// 弹出栈中优先级大于等于当前运算符的运算符
                    {
                        str += ch;
                        opstack.pop();
                        if(opstack.empty())	// 如果栈空结束循环 
                        {
                            break;
                        }
                        else ch = opstack.top();
                    } 
                    opstack.push(s[i]);		// 再将当前运算符入栈
                }
                else				// 栈为空，直接将运算符入栈
                {
                    opstack.push(s[i]);
                }
            }
        }
    }
    // 最后栈非空，出栈并输出到字符串
    while(!opstack.empty())
    {
        char ch = opstack.top();
        opstack.pop();
        str += ch;
    }
    return str;
} 


// =====================2.正则表达式转为NFA=====================
/*
这种方式构造的NFA，NFA状态只有一个在非ε的输入符号上的迁移，
且后继状态也是唯一的,但是在ε上的迁移函数结果可能有多个后继状态
NFA接受状态只有一个
*/
struct NFAState // NFA状态
{
    int index;  // 状态编号
    char input; // 输入符号（除ε外有且只有一个）
    int nextIndex;  // 后继状态编号(输入为input时有且只有一个后继状态)
    set<int> epNextIndex;   // ε上迁移的后继状态集合
};

struct NFA
{
    NFAState * head;    // 指向初始状态
    NFAState * tail;    // 指向接受状态（唯一）
};

NFAState NFAStates[MAX];    // NFA状态数组
int curNFAStateNum = 0;     // NFA状态总数

// 向状态n1添加输入符号为ch时迁移到状态n2的转换
void add(NFAState * n1, NFAState * n2, char ch)
{
    n1 -> input = ch;
    n1 -> nextIndex = n2 -> index;
}

// 向状态n1添加输入符号为ε时迁移到状态n2
void add(NFAState * n1, NFAState * n2)
{
    n1 -> epNextIndex.insert(n2 -> index);
}

// 新建一个NFA，添加起始状态和接受状态
NFA createNFA(int sum)
{
    NFA nfa;
    nfa.head = &NFAStates[sum];
    nfa.tail = &NFAStates[sum+1];
    return nfa;
}

// 正则表达式的后缀表达式转为NFA
NFA strToNFA(string str)
{
    stack<NFA> nfastack;    // 保存中间NFA的栈
    for(int i = 0; i < str.size(); ++i)     // 遍历后缀表达式的字符
    {
        if(str[i] >= 'a' && str[i] <= 'z')  // 操作数
        {
            NFA nfa = createNFA(curNFAStateNum);    // 新建一个NFA
            curNFAStateNum += 2;                    // NFA状态总数加2
            add(nfa.head, nfa.tail, str[i]);        // NFA的初始状态指向结束状态，输入符号为str[i]
            nfastack.push(nfa);                     // 新产生NFA压栈
        }
        else if(str[i] == 'E')              // ε
        {
            NFA nfa = createNFA(curNFAStateNum);    // 新建一个NFA
            curNFAStateNum += 2;                    // NFA状态总数加2
            add(nfa.head, nfa.tail);                // NFA的初始状态指向接受状态，输入符号为ε
            nfastack.push(nfa);                     // 新产生NFA压栈
        }
        else if(str[i] == '.')              // 连接运算符
        {
            NFA nfa1, nfa2, nfa;
            nfa2 = nfastack.top();  // 出栈
            nfastack.pop();
            nfa1 = nfastack.top();
            nfastack.pop();
            add(nfa1.tail, nfa2.head);      // NFA1的接受状态指向NFA2的开始状态，输入符号为ε
            nfa.head = nfa1.head;
            nfa.tail = nfa2.tail;
            nfastack.push(nfa);             // 新产生NFA压栈
        }
        else if(str[i] == '|')              // 或运算符
        {
            NFA nfa1, nfa2;
            nfa2 = nfastack.top();  // 出栈
            nfastack.pop();
            nfa1 = nfastack.top();
            nfastack.pop();

            NFA nfa = createNFA(curNFAStateNum);    // 新建一个NFA
            curNFAStateNum += 2;                    // NFA状态总数加2
            add(nfa.head, nfa1.head);   // nfa开始通过ε指向nfa1开始
            add(nfa.head, nfa2.head);   // nfa开始通过ε指向nfa2开始
            add(nfa1.tail, nfa.tail);   // nfa1结束通过ε指向nfa结束
            add(nfa2.tail, nfa.tail);   // nfa2结束通过ε指向nfa结束
            nfastack.push(nfa);         // 新产生NFA压栈
        }
        else if(str[i] == '*')              // 闭包运算符
        {
            NFA nfa1;
            nfa1 = nfastack.top();  // 出栈
            nfastack.pop();
            
            NFA nfa = createNFA(curNFAStateNum);    // 新建一个NFA
            curNFAStateNum += 2;                    // NFA状态总数加2

            add(nfa1.tail, nfa.head);   // nfa1结束通过ε指向nfa开始
            add(nfa1.tail, nfa.tail);   // nfa1结束通过ε指向nfa结束
            add(nfa.head, nfa1.head);   // nfa开始通过ε指向nfa1开始
            add(nfa.head, nfa.tail);    // nfa开始通过ε指向nfa结束
            nfastack.push(nfa);         // 新产生NFA压栈
        }
        else if(str[i] == '?')          // ?运算符
        {
            NFA nfa1;
            nfa1 = nfastack.top();  // 出栈
            nfastack.pop();
            NFA nfa = createNFA(curNFAStateNum);// 新建一个NFA
            curNFAStateNum += 2;        // NFA状态总数加2
            add(nfa.head, nfa1.head);   // nfa开始通过ε指向nfa1开始
            add(nfa1.tail, nfa.tail);   // nfa1结束通过ε指向nfa结束
            add(nfa.head, nfa.tail);    // nfa开始通过ε指向nfa结束
            nfastack.push(nfa);         // 新产生NFA压栈
        }
        else if(str[i] == '+')          // +运算符
        {
            NFA nfa1;
            nfa1 = nfastack.top();  // 出栈
            nfastack.pop();
            NFA nfa = createNFA(curNFAStateNum);    // 新建一个NFA
            curNFAStateNum += 2;        // NFA状态总数加2
            add(nfa1.tail, nfa.head);   // nfa1结束通过ε指向nfa开始
            add(nfa1.tail, nfa.tail);   // nfa1结束通过ε指向nfa结束
            add(nfa.head, nfa1.head);   // nfa开始通过ε指向nfa1开始
            nfastack.push(nfa);         // 新产生NFA压栈
        }
    }
    NFA nfa = nfastack.top();   // 栈顶为最终得到的NFA
    nfastack.pop();
    return nfa;
}


// 输出NFA
void printNFA(NFA nfa)
{
    cout<<"***************     NFA     ***************"<<endl<<endl; 
    cout<<"NFA has "<<curNFAStateNum<<"State"<<endl;
    cout<<"Start State:"<<nfa.head->index<<",End State:" <<nfa.tail->index<<", "<<endl<<endl<<"move function:"<<endl;	
    for(int i = 0; i < curNFAStateNum; i++)		/*遍历NFA状态数组*/
    {	
        if(NFAStates[i].input >= 'a' && NFAStates[i].input <= 'z')			
        {
            cout<<NFAStates[i].index<<"-->'"<<NFAStates[i].input<<"'-->"<<NFAStates[i].nextIndex<<'\t';
        }	
        set<int>::iterator it;					/*输出该状态经过ε到达的状态*/
        for(it = NFAStates[i].epNextIndex.begin(); it != NFAStates[i].epNextIndex.end(); it++)
        {
            cout<<NFAStates[i].index<<"-->'"<<' '<<"'-->"<<*it<<'\t';
        }
        cout<<endl;
    }
}





// =====================3.NFA转为DFA=====================
struct DFAState // DFA状态
{
    int index;          // 状态编号
    set<int> closure;   // 包含的NFA状态集合,NFA的ε-move()闭包
    bool isEnd;         // 是否为接受状态
};

DFAState DFAStates[MAX];    // DFA状态数组
int curDFAStateNum = 0;     // DFA状态总数

struct DFA  // DFA
{
    int startState;         // DFA初始状态编号
    set<int> endStates;     // DFA接受状态编号
    set<char> terminator;   // DFA输入符号('a'-'z')
    int trans[MAX][26];     // 转移矩阵(状态编号，输入符号)
    int stateNums;          // DFA状态总数
};

// 求NFA状态集合T的ε闭包
set<int> epsilonClosure(set<int> T)
{
    stack<int> intstack;
    set<int>::iterator it;
    for(it = T.begin(); it != T.end(); ++it)
    {
        intstack.push(*it); // 将T的所有状态压入stack中
    }
    while(!intstack.empty())
    {
        int tmp = intstack.top();   // 弹出栈顶元素(T中某状态)
        intstack.pop();
        set<int>::iterator iter;
        // 遍历某状态通过ε迁移的后续状态集合
        for(iter = NFAStates[tmp].epNextIndex.begin(); iter != NFAStates[tmp].epNextIndex.end(); ++iter)
        {
            if(!T.count(*iter)) // 后续状态不在T的ε闭包中
            {
                T.insert(*iter);        // 加入到T的ε闭包中
                intstack.push(*iter);   // 压栈
            }
        }
    }
    return T;
}

// 求NFA状态集合T的ε-closure(move(T, ch))
set<int> moveEpsilonClosure(set<int> T, char ch)
{
    set<int> moveEp;
    set<int>::iterator it;
    for(it = T.begin(); it != T.end(); ++it)// 遍历T的所有状态
    {
        if(NFAStates[*it].input == ch)// 当前状态的输入符号为ch
        {
            moveEp.insert(NFAStates[*it].nextIndex);   //将当前状态的后继状态加入到move集合中
        }
    }
    moveEp = epsilonClosure(moveEp);   // 求move集合的ε闭包
    return moveEp;
}

// 判断一个DFA状态是否为接受状态
// 如果包含nfa的一个接受状态则返回真
bool isEnd(NFA nfa, set<int> s)
{
    set<int>::iterator it;
    for(it = s.begin(); it != s.end(); ++it)// 遍历s包含的所有nfa状态
    {
        if(*it == nfa.tail->index)  //包含nfa的一个接受状态
        {
            return true;
        }
    }
    return false;
}

// NFA转为DFA
// 参数：nfa, 后缀表达式
DFA nfaToDfa(NFA nfa, string str)
{
    DFA dfa;
    set<set<int> > dfastates;   // 保存所有已构造的DFA状态，每个DFA状态为一个NFA状态集合
    memset(dfa.trans, -1, sizeof(dfa.trans));   // 初始化dfa的转移矩阵
    for(int i = 0; i < str.size(); ++i) // 遍历后缀表达式
    {
        if(str[i] >= 'a' && str[i] <= 'z')  // 操作数，加入dfa的终结符号集
        {
            dfa.terminator.insert(str[i]);
        }
    }

    // 构造DFA初始状态，初始化dfa的初始状态为ε-closure(s0)
    dfa.startState = 0; // DFA的初始状态编号为0
    set<int> tmpset;
    tmpset.insert(nfa.head->index); // s0为nfa的初始状态
    DFAStates[0].closure = epsilonClosure(tmpset);  // DFA的初始状态为ε-closure(s0)
    DFAStates[0].isEnd = isEnd(nfa, DFAStates[0].closure);  // 判断初始状态是否为终结状态
    ++curDFAStateNum;   // DFA状态数量加1

    queue<int> intq;            // 保存DFA状态编号
    intq.push(dfa.startState);  // 将DFA的初始状态加入队列中
    while(!intq.empty())
    {
        int curindex = intq.front(); // 队首元素为某dfa状态编号
        intq.pop(); // 出队
        set<char>::iterator it;
        for(it = dfa.terminator.begin(); it != dfa.terminator.end(); ++it)    // 遍历每个可能的输入符号
        {
            // 求当前DFA状态的ε-cloure(move(ch))
            set<int> temp = moveEpsilonClosure(DFAStates[curindex].closure, *it);
            if(!temp.empty() && !dfastates.count(temp)) // 新的状态集合非空且不在已有的状态集合中，新建一个DFA状态
            {
                dfastates.insert(temp); // 将新的状态集合加入到已有的状态集合中
                DFAStates[curDFAStateNum].closure = temp;
                DFAStates[curDFAStateNum].isEnd = isEnd(nfa, DFAStates[curDFAStateNum].closure);// 是否为接受态
                dfa.trans[curindex][*it - 'a'] = curDFAStateNum; // 更新DFA转移矩阵
                intq.push(curDFAStateNum);  // 将新建的DFA状态编号加入到队列中
                ++curDFAStateNum;   // DFA状态总数加1
            }   
            else    // 当前DFA状态的ε-cloure(move(ch))已在状态集合中
            {
                for(int i = 0; i < curDFAStateNum; ++i)
                {
                    if(temp == DFAStates[i].closure)    // 找到与该集合相同的DFA状态
                    {
                        dfa.trans[curindex][*it - 'a'] = i; // 更新DFA转移矩阵
                        break;
                    }
                }
            }
        }
    }
    // 计算dfa的接受状态集合
    for(int i = 0; i < curDFAStateNum; ++i) // 遍历DFA的所有状态
    {
        if(DFAStates[i].isEnd)              // 该状态是接受状态
        {
            dfa.endStates.insert(i);        // 添加该状态编号到dfa接受状态集合中
        }
    }
    dfa.stateNums = curDFAStateNum;         // 该DFA的状态总数

    return dfa;
}

void printDFA(DFA dfa)
{
    cout<<"***************     DFA     ***************"<<endl<<endl; 
    cout<<"DFA has total "<<dfa.stateNums<<"States , "<<"Start State:"<<dfa.startState<<endl<<endl;
    cout<<"input chars:{";
    set<char>::iterator it;
    for(it = dfa.terminator.begin(); it != dfa.terminator.end(); it++)
    {
        cout<<*it<<' ';
    }
    cout<<'}'<<endl<<endl;
    cout<<"Final States Set:{"; 
    set<int>::iterator iter;
    for(iter = dfa.endStates.begin(); iter != dfa.endStates.end(); iter++)
    {
        cout<<*iter<<' ';
    }
    cout<<'}'<<endl<<endl;
    cout<<endl<<"Trans Matrix:"<<endl<<"      ";
	set<char>::iterator t;    
    for(t = dfa.terminator.begin(); t != dfa.terminator.end(); t++)
    {
        cout<<*t<<"\t";
    }
    cout<<endl;
    for(int i = 0; i < dfa.stateNums; ++i)
    {
        if(dfa.endStates.count(i))
        {
            cout<<'('<<i<<")   ";
        }
        else
        {
            cout<<i<<"     ";
        }
        for(int j = 0; j < 26; ++j)
        {
            if(dfa.terminator.count(j+'a'))
            {
                if(dfa.trans[i][j] != -1)
                {
                    cout<<dfa.trans[i][j]<<"  ";
                }
                else
                {
                    cout<<"   ";
                }
                
            }
        }
        cout<<endl;
    }
}

// =====================4.判断2个DFA的关系=====================
// 将接受状态变为非接受状态，非接受状态变为接受状态，返回新的接受态
set<int> getReverse(set<int> & endStates, int stateNums)
{
    set<int> reverse;
    reverse.insert(-1);     // -1原来也是非接受态
    for(int i = 0; i < stateNums; ++i)
    {
        if(!endStates.count(i))
            reverse.insert(i);
    }
    return reverse;
}

// 判断两个状态是否都是接受状态
bool isBothEndState(int state1, int state2, set<int> & endStates1, set<int> & endStates2)
{
    if(endStates1.count(state1) && endStates2.count(state2))    // state1为接受状态且state2为接受状态
        return true;
    else
        return false;
}

// 判断两个DFA表示的语言集合是否相交
// 参数：dfa1 第1个DFA， dfa2 第2个DFA
// state1 第1个DFA的某个状态 state2 第2个DFA的某个状态
// endStates1 接受状态集合1(dfa1的接受态或非接受态)   
// endStates2 接受状态集合2(dfa2的接受态或非接受态)
// visited 二维数组，保存两个状态是否被访问过
bool intersect(DFA &dfa1, DFA & dfa2, int state1, int state2, 
set<int> & endStates1, set<int> & endStates2, vector<vector<bool> > & visited)
{
    visited[state1+1][state2+1] = true; // 访问state1和state2
     // state1和state2均为接受状态，则两个语言集合相交，
     // 即从dfa1和dfa2的初始状态出发，存在输入串能够分别到达endStates1和endStates2中的状态
    if(isBothEndState(state1, state2, endStates1, endStates2)) 
        return true;
    for(int i = 0; i < 26; ++i) // 遍历每一个输入字符构造输入串
    {
        int nextState1 = (state1 == -1 ? -1 : dfa1.trans[state1][i]); // -1的后继状态仍为-1
        int nextState2 = (state2 == -1 ? -1 : dfa2.trans[state2][i]);
        if(!visited[nextState1+1][nextState2+1])    // dfa1和dfa2的后继状态没有被访问过
        {
            // 递归调用判断后继状态是否为接受态
            if(intersect(dfa1, dfa2, nextState1, nextState2, endStates1, endStates2, visited))
                return true;
            visited[nextState1+1][nextState2+1] = false;    // 回溯
        }
    }
    return false;   // 遍历所有的输入字符都不能同时到达接受态，则返回假
}


int main()
{
    int T;  // 测试数据的组数
    cin>>T;
    while(T--)
    {
        string str1, str2;      
        cin>>str1>>str2;            // 输入正则表达式
        if(str1 == str2)            // 两个正则表达式相同则一定等价
        {
            cout<<"="<<endl;
            continue;
        }
        
        // 将第1个正则表达式str1转为DFA
        string suffix1 = infixToSuffix(str1);   // 正则表达式从中缀表达式转为后缀表达式
        // cout<<suffix1<<endl;

        for(int i = 0; i < MAX; i++)        // 初始化NFAStates
        {
            NFAStates[i].index = i;
            NFAStates[i].input = '#';
            NFAStates[i].nextIndex = -1;
            NFAStates[i].epNextIndex.clear();
        }
        curNFAStateNum  = 0;        // 初始化当前NFA状态总数为0

        NFA nfa1 = strToNFA(suffix1); // 后缀表达式转为NFA
        // printNFA(nfa1);

        for(int i = 0; i < MAX; i++)
        {
            DFAStates[i].index = i;
            DFAStates[i].isEnd = false;
            DFAStates[i].closure.clear();
        }
        curDFAStateNum = 0;        // 初始化当前DFA状态总数为0
        DFA dfa1 = nfaToDfa(nfa1, suffix1);   // NFA转为DFA
        
        // 将第2个正则表达式str2转为DFA
        string suffix2 = infixToSuffix(str2);   // 正则表达式从中缀表达式转为后缀表达式
        // cout<<suffix2<<endl;

        for(int i = 0; i < MAX; i++)        // 初始化NFAStates
        {
            NFAStates[i].index = i;
            NFAStates[i].input = '#';
            NFAStates[i].nextIndex = -1;
            NFAStates[i].epNextIndex.clear();
        }
        curNFAStateNum  = 0;        // 初始化当前NFA状态总数为0

        NFA nfa2 = strToNFA(suffix2); // 后缀表达式转为NFA
        // printNFA(nfa2);

        for(int i = 0; i < MAX; i++)
        {
            DFAStates[i].index = i;
            DFAStates[i].isEnd = false;
            DFAStates[i].closure.clear();
        }
        curDFAStateNum = 0;        // 初始化当前DFA状态总数为0
        DFA dfa2 = nfaToDfa(nfa2, suffix2);   // NFA转为DFA

        /*cout<<"================1===================="<<endl;
        printDFA(dfa1);
        cout<<endl<<"================2===================="<<endl;
        printDFA(dfa2);*/

        // 判断2个DFA的关系
        // 二维数组，大小为dfa1.stateNums* dfa2.stateNums
        // 访问数组，保存2个dfa状态访问记录，防止出现死循环（如闭包）
        vector<vector<bool> > visited;   
        for(int i = 0; i <= dfa1.stateNums; ++i)    // 初始化
        {
            vector<bool> tmp;
            for(int j = 0; j <= dfa2.stateNums; ++j)
            {
                tmp.push_back(false);
            }
            visited.push_back(tmp);
        }

        set<int> dfa1EndState = dfa1.endStates;     // dfa1的接受状态集合
        set<int> dfa2EndState = dfa2.endStates;     // dfa2的接受状态集合
        set<int> reverse_dfa1EndState = getReverse(dfa1EndState, dfa1.stateNums);   // dfa1的接受状态集合的补集
        set<int> reverse_dfa2EndState = getReverse(dfa2EndState, dfa2.stateNums);   // dfa2的接受状态集合的补集

        bool result1 = intersect(dfa1, dfa2, dfa1.startState, dfa2.startState, reverse_dfa1EndState, dfa2EndState, visited);
        for(int i = 0; i <= dfa1.stateNums; ++i)
        {
            for(int j = 0; j <= dfa2.stateNums; ++j)
            {
                visited[i][j] = false;
            }
        }
        bool result2 = intersect(dfa1, dfa2, dfa1.startState, dfa2.startState, dfa1EndState, reverse_dfa2EndState, visited);
        
        if(result1 && result2)
        {
            cout<<"!"<<endl;
        }
        else if(result1)
        {
            cout<<"<"<<endl;
        }
        else if(result2)
        {
            cout<<">"<<endl;
        }
        else
        {
            cout<<"="<<endl;
        }
    }   
    
    return 0;
}
