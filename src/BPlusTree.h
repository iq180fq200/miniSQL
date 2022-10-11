
#ifndef _MINISQL_BPLUSTREE_H_
#define _MINISQL_BPLUSTREE_H_

#include "global.h"
using namespace std;

std::string GenIndexName(std::string table_name,std::string index_name);

#ifndef SplitLeafNode
#define SplitLeafNode(_Node) SplitNode(_Node,true)
#endif

#ifndef To_Leftmost
#define To_Leftmost(cur) while(cur&&!cur->son[0]) cur=cur->son[0];
#define To_Rightmost(cur) while(cur&&cur->son[cur->n-1]) cur=cur->son[cur->n-1];
#endif

#define NodeCopy(_to,__offsetto__,_from,__offsetfrom__,_num,_leftmost) \
	memcpy((_to)->values+(__offsetto__), \
		(_from)->values+(__offsetfrom__),   sizeof(T)       * (_num) ); \
	memcpy((_to)->element+(__offsetto__), \
		(_from)->element+(__offsetfrom__),  sizeof(Pointer) * (_num) ); \
	memcpy((_to)->son+(__offsetto__)+1, \
		(_from)->son+(__offsetfrom__)+1,    sizeof(Node<T>*)*((_num)+(_leftmost)));


template <class T>
class Node {
public:
    uint m;
    uint n;
    T* values;
    Pointer* element;
    Node** son;
    Node* fa;
    Node* last;
    Node* next;

public:
    Node() {};
    Node(int _MAXChildrens = defaultMaxChildrens);
    void Save(FILE* fp);
    void Load(FILE* fp);
    virtual ~Node();
};


template <class T>
class BPlusTree {
private:
    Node<T>* root;
    string TableName;
    string AttribName;
    int m;

    Node<T>* FindPosition(const T& data);
    Node<T>* Find(const T& data, int& index);
    int Find(const T& data, Node<T>* node);
    int GetRecordsNum();
    bool Borrow(Node<T>* node, T& data);
    bool Merge(Node<T>* node, T& data);
    void InsertKey(const T& data, const Pointer& pointer, Node<T>* node, Node<T>* fa);
    void InsertKey(const T& data, const Pointer& pointer, Node<T>* node);
    void DeleteKey(Node<T>* node, T& data);

public:
    BPlusTree() {};
    BPlusTree(std::string& FileName);
    BPlusTree(std::string& _TableName, std::string& _AttribName, int _MaxBranches);
    bool Find(const T& data, Pointer& pointer);
    Pointer FindPointer(const T& data);
    int FindLess(const T& data, bool CanEqual, vector<Pointer>& pointers);
    int FindLarger(const T& data, bool CanEqual, vector<Pointer>& pointers);
    int FindNonEqual(const T& data, vector<Pointer>& pointers);
    int FindBetween(const T& data1, bool CanEqual1, T& data2, bool CanEqual2, std::vector<Pointer>& pointers);
    int CalNodeNum(Node<T>* node);
    string GetSaveFileName();
    void Insert(const T& data, const Pointer& pointer);
    void SplitNode(Node<T>* node,bool isleaf=false);
    void Delete(T& data);
    void DoSave(Node<T>* node, FILE* fp);
    void Save(const char* filename);
    void Load(const char* filename);
    bool DeletePointers(vector<Pointer>& pointers);
    void Update(T& data_old, T& data_new, Pointer pointer_new);
    int PrintAll();
    int PrintAllreverse();
    int PrintAll(std::ofstream& fout);
    void setAttribute(string&, string&);
    virtual ~BPlusTree() {};
};

// [][][][][][][][]

template<class T>
void Node<T>::Save(FILE* fp)
{
	fwrite(&n, sizeof(int), 1, fp);
	fwrite(values, sizeof(T), m, fp);
	fwrite(element, sizeof(Pointer), m, fp);
	fwrite(&fa, sizeof(uint*), 1, fp);
	fwrite(son, sizeof(uint*), m + 1, fp);
	fwrite(&last, sizeof(uint*), 1, fp);
	fwrite(&next, sizeof(uint*), 1, fp);
}

template<class T>
void Node<T>::Load(FILE* fp)
{
	fread(&n, sizeof(int), 1, fp);
	fread(values, sizeof(T), m, fp);
	fread(element, sizeof(Pointer), m, fp);
	fread(&fa, sizeof(uint*), 1, fp);
	fread(son, sizeof(uint*), m + 1, fp);
	fread(&last, sizeof(uint*), 1, fp);
	fread(&next, sizeof(uint*), 1, fp);
}

template<class T>
Node<T>::~Node()
{
	delete[]element;
	delete[]son;
}


template<class T>
Node<T>::Node(int _MaxBranches):m(_MaxBranches),n(0),fa(NULL),next(NULL),last(NULL) {
	values = new T[m];
	element = new Pointer[m];
	memset(element,0x00,sizeof(Pointer)*m);
	son = new Node<T>*[m+1];
	memset(son,0x00,sizeof(uint*)*(m+1));
}


template<class T>
int BPlusTree<T>::GetRecordsNum() {
	int count = 0;
	Node<T>* cur=root;
	To_leftmost(cur);
	while(cur) count+=cur->n,cur=cur->next;
	cout << "total records: " << count << endl;
	return count;
}

template<class T>
void BPlusTree<T>::InsertKey(const T& data,const Pointer& pointer,Node<T>* node,Node<T>* fa) {
	for(int i=0;i<fa->n;++i) {
		if(fa->values[i]>data) {
			for(int j=fa->n;j>i;--j) {
				fa->values[j]=fa->values[j-1];
				fa->element[j]=fa->element[j-1];
				fa->son[j+1]=fa->son[j];
			}
			fa->values[i]=data;
			fa->element[i]=pointer;
			fa->son[i+1]=node;
			++(fa->n);
			return;
		}
	}
	fa->values[fa->n]=data;
	fa->element[fa->n]=pointer;
	++(fa->n);
	fa->son[fa->n]=node;
	return;
}

template<class T>
void BPlusTree<T>::InsertKey(const T& data,const Pointer& pointer,Node<T>* node) {
	int i;
	for(i=0;i<node->n;++i) {
		if(node->values[i]>data) {
			for(int j=node->n;j>i;--j) {
				node->values[j]=node->values[j-1];
				node->element[j]=node->element[j-1];
			}
			node->values[i]= data;
			node->element[i]= pointer;
			node->n++;
			return;
		}
	}
	node->n++;
	node->element[i]= pointer;
	node->values[i]= data;
	return;
}

template<class T>
void BPlusTree<T>::Insert(const T& data, const Pointer& pointer) {
	Node<T>* node=NULL;
	if(!this->root) {
		this->root = new Node<T>(this->m);
		node = this->root;
	}
	else node = FindPosition(data);
	InsertKey(data,pointer,node);
	if(node->n==this->m) SplitLeafNode(node);
	return;
}


template<class T>
void BPlusTree<T>::SplitNode(Node<T>* node,bool isleaf) {
	Node<T>* fa = node->fa;
	if(!fa) { // root.
		fa = new Node<T>(m);
		fa->son[0]=node;
		node->fa=fa;
		this->root=fa;
	}
	Node<T>* newNode = new Node<T>(m);
	uint lm=((m-(!isleaf))>>1);
	uint rm=((m-(!isleaf))-lm);
	newNode->n=rm;
	memcpy(newNode->values,node->values+lm+(!isleaf),sizeof(T)*rm);
	memcpy(newNode->element,node->element+lm+(!isleaf),sizeof(Pointer)*rm);
	if(!isleaf) {
		memcpy(newNode->son,node->son+lm+1,sizeof(Node<T>*)*(rm+1));
		for(int i=0;i<=newNode->n;++i) newNode->son[i]->fa=newNode;
	}

	
	newNode->fa=fa;
	newNode->last=node;
	newNode->next=node->next;
	node->next=newNode;
	if(newNode->next) newNode->next->last=newNode;
	node->n=lm;
	InsertKey(node->values[lm],node->element[lm],newNode,fa);
	if(fa->n==m) SplitNode(fa);
	return;
}

template<class T>
Node<T>* BPlusTree<T>::FindPosition(const T& data) {
	Node<T>* cur=this->root;
	MyAssert(cur==NULL,Exist,"Root is not exists.");
	while(cur)
		if(cur->son[0]==NULL) return cur;
		else for(int i=0;i<=cur->n;++i)
			if(i==cur->n||cur->values[i]>data) {cur=cur->son[i];break;}
	return NULL;
}

template<class T>
Node<T>* BPlusTree<T>::Find(const T& data,int& index) {
	Node<T>* cur=this->root;
	MyAssert(cur==NULL,Exist,"Root is not exists.");
	while(cur) {
		if(cur->son[0]==NULL) return Find(data,cur)!=-1?cur:NULL;
		for(int i=0;i<=cur->n;++i) if(i==cur->n||cur->values[i]>data) {cur=cur->son[i];break;}
	}
	return NULL;
}

template<class T>
int BPlusTree<T>::Find(const T& data,Node<T>* cur) {
	for(int i=0;i<cur->n;++i) 
		if(cur->values[i]==data) return i;
	return -1;
}

template<class T>
bool BPlusTree<T>::Borrow(Node<T>* node, T& data) {
	Node<T>* fa;
	Node<T>* ls;
	Node<T>* rs;
	int indexP;
	fa=node->fa;
	if(node->last && node->last->fa==node->fa) ls=node->last; else ls=NULL;
	if(node->next && node->next->fa==node->fa) rs=node->next; else rs=NULL;
	if(fa)
		if(Find(data,fa)==-1) indexP=Find(node->values[0],fa);
		else indexP=Find(data,fa)==-1;
	if(ls && ((!ls->son[0] && ls->n>(m>>1)) || (ls->son[0] && ls->n>(m-1)>>1))) {
		NodeCopy(node,1,node,0,m-1,1); // cur node to right shift one position.
		// borrow one from ls
		node->values[0]=ls->values[ls->n-1];
		node->element[0]=ls->element[ls->n-1];
		node->son[1]=ls->son[ls->n];
		// shift the last son
		if(ls->son[ls->n]) ls->son[ls->n]->fa = node;
		ls->n--;
		if(fa) {
			fa->values[indexP]= node->values[0];
			fa->element[indexP]= node->element[0];
			fa->son[indexP + 1]= node;
		}
		node->n++;
		return true;
	}
	else if(rs && ((!rs->son[0] && rs->n > (m >>1)) ||
		(rs->son[0] && rs->n > (m - 1) >>1))) {
		if(fa) {
			indexP = Find(rs->values[0],fa);
			fa->values[indexP]=rs->values[1];
			fa->element[indexP]=rs->element[1];
			fa->son[indexP+1]=rs;
		}
		// borrow one from right.
		node->values[node->n]=rs->values[0];
		node->element[node->n]=rs->element[0];
		node->son[node->n]=rs->son[0];
		if(rs->son[1]) rs->son[1]->fa=node;
		// rightSibing to left shift one position.	
		NodeCopy(rs,0,rs,1,m-1,1);
		node->n++;
		rs->n--;
		return true;
	}
	else
		return false;
}

template<class T>
bool BPlusTree<T>::Merge(Node<T>* node,T& data) {
	Node<T>* fa;
	Node<T>* ls;
	Node<T>* rs;
	int indexP;
	fa=node->fa;
	if(node->last && node->last->fa==node->fa) ls=node->last; else ls=NULL;
	if(node->next && node->next->fa==node->fa) rs=node->next; else rs=NULL;
	if(fa) 
		if(Find(data,fa)==-1) indexP=Find(node->values[0],fa);
		else indexP=Find(data,fa)==-1;
	if(ls && ((!ls->son[0] && ls->n+node->n<m-1) || (ls->son[0] && ls->n+node->n<m-2))) {
		// merge to ls
		NodeCopy(ls,ls->n,node,0,node->n,0);
		if(fa) {
			int i=indexP;
			NodeCopy(fa,i,fa,i+1,m-i-1,1);
			fa->n--;
		}
		if(node->son[0]) for(int i=0;i<node->n;++i) node->son[i]->fa=ls;
		ls->n+=node->n;
		ls->next=node->next;
		node->next->last=ls;
		return true;
	}
	else if(rs && ((!rs->son[0] && rs->n+node->n<m-1) || (rs->son[0] && rs->n>+node->n<m-2))) {
		// merge to rs.
		NodeCopy(node,node->n,rs,0,rs->n,0);
		if(fa) {
			int i=Find(rs->values[0],fa);
			NodeCopy(fa,i,fa,i+1,m-i-1,1);
			fa->n--;
		}
		if(rs->son[0])
			for(int i=0;i<rs->n;++i) rs->son[i]->fa=ls;
		node->n+=rs->n;
		node->next=rs->next;
		rs->next->last=node;
		return true;
	}
	return false;
}


template<class T>
void BPlusTree<T>::DeleteKey(Node<T>* node, T& data) {
	int pos=Find(data, node);
	int indexP=-1;
	Node<T>* fa=node->fa;
	if(pos !=-1) {
		NodeCopy(node,pos,node,pos+1,m-pos-1,0)
		node->n--;
		if(node == this->root && node->n == 0) {
			this->root = node->son[0];
			if(this->root != NULL) this->root->fa = NULL;
			delete node;
			return;
		}
		if(node != root && ((node->son[0] && node->n < (m>>1)) ||
			(!node->son[0] && node->n < (m - 1)>>1))) {
			if(!Borrow(node, data)) Merge(node, data);
		}
	}
	// recursion to delete the fa.
	if(fa) DeleteKey(fa, data);

	return;
}

template<class T>
void BPlusTree<T>::Delete(T& data) {
	int i;
	if(!root) return;
	Node<T>* node = NULL;
	node=FindPosition(data);
	if(!node) return;
	DeleteKey(node, data);
}

template<class T>
bool BPlusTree<T>::Find(const T& data, Pointer& pointer) {
	Node<T>* node = this->root;
	while(node) {
		if(node->son[0]== NULL) {
			for(int i = 0; i < node->n; ++i) {
				if(node->values[i]== data) {
					pointer = node->element[i];
					return true;
				}
			}
			pointer = 0;
			return false;
		}
		if(node->values[node->n - 1] <= data) {
			node = node->son[node->n];
			continue;
		}
		for(int i = 0; i < node->n; ++i) {
			if(node->values[i] > data) {
				node = node->son[i];
				break;
			}
		}
	}
	pointer = 0;
	return false;
}


template<class T>
Pointer BPlusTree<T>::FindPointer(const T& data) {
	Node<T>* node = this->root;
	int i;
	while(node) {
		for(i = 0; i < node->n; ++i)
			if(node->values[i] > data)
				if(node->son[0]== NULL) 
					if(i > 1) return node->element[i - 1];
					else if(node->last) return node->last->element[node->last->n - 1];
					else return node->element[i];
				else node = node->son[i];
		if(node->son[i]) node = node->son[i];
		else break;
	}
	return 0;
}


template<class T>
int BPlusTree<T>::FindLess(const T& data, bool CanEqual, vector<Pointer>& pointers) {
	Node<T>* node = root;
	while(node != NULL && node->son[0])
	{
		int i;
		for(i = 0; i < node->n; ++i)
			if(node->values[i] >= data)
				break;
		if(i == m - 1)
			node = node->son[m - 1];
		else
			if(data == node->values[i])
				node = node->son[i + 1];
			else
				node = node->son[i];
	}
	pointers.clear();
	int count = 0;
	if(node != NULL) {
		int k;
		for(k = node->n - 1; k >= 0; k--) {
			if(CanEqual) {
				if(node->values[k] <= data)
					break;
			}
			else {
				if(node->values[k] < data)
					break;
			}
		}
		for(int i = k; i >= 0; i--) {
			pointers.push_back(node->element[i]);
			count++;
		}
		node = node->last;
		bool out = false;
		while(node != NULL && !out) {
			for(int k = node->n - 1; k >= 0; k--) {
				pointers.push_back(node->element[k]);
				count++;
			}
			node = node->last;
		}
		std::reverse(pointers.begin(), pointers.end());
	}
	return count;
}

template<class T>
int BPlusTree<T>::FindLarger(const T& data, bool CanEqual, vector<Pointer>& pointers) {
	Node<T>* node = root;
	while(node != NULL && node->son[0])
	{
		int i;
		for(i = 0; i < node->n; ++i)
			if(node->values[i] >= data)
				break;
		if(i == m - 1)
			node = node->son[m - 1];
		else {
			if(data == node->values[i])
				node = node->son[i + 1];
			else
				node = node->son[i];
		}
	}
	pointers.clear();
	int count = 0;
	if(node != NULL) {
		int i;
		for(i = 0; i < node->n; ++i)
			if(CanEqual) {
				if(node->values[i] >= data)
					break;
			}
			else {
				if(node->values[i] > data)
					break;
			}
		for(int k = i; k < node->n; k++) {
			pointers.push_back(node->element[k]);
			count++;
		}
		node = node->next;
		bool out = false;
		while(node != NULL && !out) {
			for(int k = 0; k < node->n; k++) {
				pointers.push_back(node->element[k]);
				count++;
			}
			node = node->next;
		}
	}
	return count;
}


template<class T>
int BPlusTree<T>::FindNonEqual(const T& data, std::vector<Pointer>& pointers) {
	int count = 0;
	pointers.clear();
	Node<T>* node = root;
	while(node != NULL && node->son[0])
		node = node->son[0];
	bool out = false;
	while(node != NULL && !out) {
		for(int i = 0; i < node->n; ++i)
		{
			if(node->values[i]== data)
				continue;
			pointers.push_back(node->element[i]);
			count++;
		}
		node = node->next;
	}
	return count;
}

template<class T>
int BPlusTree<T>::FindBetween(const T& data1, bool CanEqual1, T& data2, bool CanEqual2, std::vector<Pointer>& pointers) {
	Node<T>* node = root;
	while(node != NULL && node->son[0]) {
		int i;
		for(i = 0; i < node->n; ++i)
			if(node->values[i] >= data1)
				break;
		if(i == m - 1)
			node = node->son[m - 1];
		else {
			if(data1 == node->values[i])
				node = node->son[i + 1];
			else
				node = node->son[i];
		}
	}
	pointers.clear();
	int count = 0;
	if(node != NULL) {
		int i;
		for(i = 0; i < node->n; ++i)
			if(CanEqual1) {
				if(node->values[i] >= data1)
					break;
			}
			else {
				if(node->values[i] > data1)
					break;
			}
		for(int k = i; k < node->n; k++) {
			if(CanEqual2) {
				if(node->values[k] > data2)
					break;
			}
			else {
				if(node->values[k] >= data2)
					break;
			}
			pointers.push_back(node->element[k]);
			count++;
		}
		node = node->next;
		bool out = false;
		while(node != NULL && !out) {
			for(int k = 0; k < node->n; k++) {
				if(CanEqual2) {
					if(node->values[k] > data2) {
						out = true;
						break;
					}
				}
				else {
					if(node->values[k] >= data2) {
						out = true;
						break;
					}
				}
				pointers.push_back(node->element[k]);
				count++;
			}
			node = node->next;
		}
	}
	return count;
}

template<class T>
string BPlusTree<T>::GetSaveFileName() {
	return GenIndexName(TableName,AttribName);
}

template<class T>
void BPlusTree<T>::Save(const char* filename) {
	FILE* fp = fopen(filename, "wb");
	fwrite(&AttribName, sizeof(std::string), 0, fp);
	fwrite(&m, sizeof(int), 1, fp);
	int tot = CalNodeNum(root);
	fwrite(&tot, sizeof(int), 1, fp);
	DoSave(root, fp);
	fclose(fp);
}

template<class T>
int BPlusTree<T>::CalNodeNum(Node<T>* node) {
	if(node == NULL) return 0;
	int ans = 1;
	for(int i = 0; i <= node->n; ++i)
		ans += CalNodeNum(node->son[i]);
	return ans;
}

template<class T>
void BPlusTree<T>::DoSave(Node<T>* node, FILE* fp) {
	if(node == NULL) return;
	fwrite(&node, sizeof(Node<T>*), 1, fp);
	node->Save(fp);
	for(int i = 0; i <= node->n; ++i)
		DoSave(node->son[i], fp);
}

template<class T>
void BPlusTree<T>::Load(const char* filename) {
	FILE* fp = fopen(filename, "rb");
	fread(&AttribName, sizeof(std::string), 0, fp);
	fread(&m, sizeof(int), 1, fp);
	int tot;
	fread(&tot, sizeof(int), 1, fp);
	if(tot == 0) {
		root = NULL;
		return;
	}
	Node<T>** tmp;
	Node<T>** old;
	tmp = new Node<T> * [tot];
	old = new Node<T> * [tot];
	for(int i = 0; i < tot; ++i) {
		tmp[i]= new Node<T>(m);
		fread(&old[i], sizeof(Node<T>*), 1, fp);
		tmp[i]->Load(fp);
	}
	for(int i = 0; i < tot; ++i) {
		for(int k = 0; k < tot; k++)
			if(tmp[i]->fa == old[k]) {
				tmp[i]->fa = tmp[k];
				break;
			}
		for(int j = 0; j <= tmp[i]->n; j++)
			for(int k = 0; k < tot; k++)
				if(tmp[i]->son[j]== old[k]) {
					tmp[i]->son[j]= tmp[k];
					break;
				}
		for(int k = 0; k < tot; k++)
			if(tmp[i]->last == old[k]) {
				tmp[i]->last = tmp[k];
				break;
			}
		for(int k = 0; k < tot; k++)
			if(tmp[i]->next == old[k]) {
				tmp[i]->next = tmp[k];
				break;
			}
	}
	root = tmp[0];
	delete[]tmp;
	delete[]old;
}

template<class T>
bool BPlusTree<T>::DeletePointers(vector<Pointer>& pointers) {
	int num = pointers.size();
	for(int i = 0; i < num; ++i) {
		Node<T>* node = root;
		while(node != NULL && node->son[0])
			node = node->son[0];
		bool found = false;
		int k;
		while(node != NULL && !found) {
			for(k = 0; k < node->n; k++) {
				if(node->element[k]== pointers[i]) {
					found = true;
					break;
				}
			}
			if(found)
				break;
			node = node->next;
		}
		if(found)
			DeleteKey(node, node->values[k]);
	}
	return true;
}

template<class T>
void BPlusTree<T>::Update(T& data_old, T& data_new, Pointer pointer_new) {
	Delete(data_old);
	Insert(data_new, pointer_new);
}

template<class T>
int BPlusTree<T>::PrintAll() {
	int count = 0;
	Node<T>* node = root;
	while(node != NULL && node->son[0])
		node = node->son[0];
	while(node != NULL) {
		for(int i = 0; i < node->n; ++i) {
			count++;
			std::cout << node->values[i] << " ";
		}
		std::cout << "    ";
		node = node->next;
	}
	std::cout << std::endl;
	std::cout << "count=" << count << std::endl;
	return count;
}

template<class T>
int BPlusTree<T>::PrintAllreverse() {
	int count = 0;
	Node<T>* node = root;
	while(node != NULL && node->son[0])
		node = node->son[node->n];
	while(node != NULL) {
		for(int i = 0; i < node->n; ++i) {
			count++;
			std::cout << node->values[i] << " ";
		}
		node = node->last;
	}
	std::cout << std::endl;
	std::cout << "count=" << count << std::endl;
	return count;
}

template<class T>
int BPlusTree<T>::PrintAll(std::ofstream& fout) {
	int count = 0;
	Node<T>* node = root;
	while(node != NULL && node->son[0])
		node = node->son[0];
	while(node != NULL) {
		for(int i = 0; i < node->n; ++i) {
			count++;
			fout << node->values[i] << " ";
		}
		node = node->next;
	}
	fout << endl;
	fout << "count=" << count << endl;
	return count;
}

template<class T>
BPlusTree<T>::BPlusTree(std::string& FileName) {
	Load(FileName.c_str());
}

template<class T>
BPlusTree<T>::BPlusTree(std::string& _TableName, std::string& _AttribName, int _MaxBranches) {
	TableName = _TableName;
	AttribName = _AttribName;
	m = _MaxBranches;
	root = NULL;
}
template<class T>
void BPlusTree<T>::setAttribute(string& table, string& attribute) {
	this->TableName = table;
	this->AttribName = attribute;
}

#endif
