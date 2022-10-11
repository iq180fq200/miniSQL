/***
 *
 **/

#include<assert.h>
#include<cstdio>
using namespace std;

template<class T>
void Merge(T* a[],T* tmp[],unsigned int l,unsigned int r,bool (*cmp)(T,T)) // Merge sort of pointers.
{
	if(l>=r) return;
	unsigned int mid=(l+r)>>1;
	Merge(a,tmp,l,mid,cmp);
	Merge(a,tmp,mid+1,r,cmp);
	unsigned int i=l,j=mid+1,cnt=0;
//	while(i<=mid && j<=r) if(cmp(**(a+i),**(a+j))) tmp[cnt++]=a[i++]; else tmp[cnt++]=a[j++]; // 相等先放右边的数。 
	while(i<=mid && j<=r) if(cmp(**(a+j),**(a+i))) tmp[cnt++]=a[j++]; else tmp[cnt++]=a[i++]; // 相等先放左边的数。 
	while(i<=mid) tmp[cnt++]=a[i++];
	while(j<=r) tmp[cnt++]=a[j++];
	for(i=0;i<cnt;++i) a[l+i]=tmp[i];
}
template<class T>
void MergeSort(T* const l,T* const r,bool (*cmp)(T,T)) // Sort [l,r) with cmp() function.
{
	T *a[r-l];
	T **tmp=new T*[(unsigned int)(r-l)];
	for(int i=r-l-1;i>=0;--i) a[i]=l+i;
	Merge(a,tmp,0,r-l-1,cmp);
	delete tmp; // 寸土寸金.jpg。 
	T t;
	unsigned int pos;
	unsigned int lastpos;
	bool vis[r-l]={0};
	for(int i=r-l-1;i>=0;--i) 
		if(!vis[i]) {
			t=*(l+i),pos=i;
			do{*(l+pos)=**(a+pos),vis[pos]=true,lastpos=pos,pos=*(a+pos)-l;}while(!vis[pos]);
			*(l+lastpos)=t;
		}
}

//
//void BufferRead(int ibuf[],char filename[],unsigned int l,unsigned int r) // l 从零编号，读入 [l,r) 这些记录。 
//{
//	FILE *fp=fopen(filename,"rb");
//	assert(fp!=NULL);
//	unsigned int elementsize=sizeof(int);
//	unsigned int filehead=0;
////	unsigned int blocksize=3*elementsize;
//	fseek(fp,filehead+elementsize*l,SEEK_SET);
//	fread(ibuf,elementsize,r-l,fp);
//	printf("ibuf=");for(unsigned int i=0;i<r-l;++i) printf("%d ",ibuf[i]);printf("\n");
//	fclose(fp);
//}
//void BufferWrite(int obuf[],char filename[],unsigned int l,unsigned int r)
//{
//	FILE *fp=fopen(filename,"rb+");
//	assert(fp!=NULL);
//	unsigned int elementsize=sizeof(int);
//	unsigned int filehead=0;
//	fseek(fp,filehead+elementsize*l,SEEK_SET);
//	fwrite(obuf,elementsize,r-l,fp);
//	printf("obuf=");for(unsigned int i=0;i<r-l;++i) printf("%d ",obuf[i]);printf("\n");
//	fclose(fp);
//}
//
//template<class T>
//void ExternalSort(char filename[],unsigned int l,unsigned int r,unsigned int k,bool (*cmp)(T,T))
//{
//	if(l>r) return;
//	unsigned int elementsize=sizeof(T)+sizeof(T*)*2; // 一个元素在排序中需要过程的空间。 
//	unsigned int memorysize=100; // 内存大小。 
//	unsigned int blocksize=24;
//	unsigned int seglen=memorysize/elementsize; // 一个排序段的元素数量。 
//	unsigned int segnum=(elementsize*-1)/memorysize+1; // 归并次数。 
//	T *a=new T[seglen];
//	for(unsigned int i=1,tmp1=l,tmp2=l+seglen;i<segnum;++i,tmp1+=seglen,tmp2+=seglen) {
//		BufferRead(a,filename,tmp1,tmp2);
//		MergeSort(a+0,a+seglen,cmp);
//		BufferWrite(filename,tmp1,tmp2,a);
//	}
//	BufferRead(a,filename,l+seglen*(segnum-2),r);
//	MergeSort(a+0,a+seglen,cmp);
//	BufferWrite(filename,l+seglen*(segnum-2),r,a);
//	delete []a;
//	
//	
//	delete []a;
//}
//bool cmp(int a,int b) {return a<b;}
//
//int main()
//{
//	int a[10]={2,4,0,1,3,9,7,8,6,3};
//	MergeSort(a,a+10,cmp);
//	printf("a=");for(int i=0;i<10;++i) printf("%d ",a[i]);printf("\n");
//	int b[10]={10,7,4,1,9,6,3,8,5,2};
//	FILE *fp=fopen("text.dat","rb+");
//	fseek(fp,0,SEEK_SET);
//	fwrite(b,sizeof(int),10,fp);
//	BufferRead(a,"text.dat",0,3);
//	BufferWrite(a+3,"text.dat",0,3);
//	BufferRead(a,"text.dat",1,8);
//	fclose(fp);
//	return 0;
//}
