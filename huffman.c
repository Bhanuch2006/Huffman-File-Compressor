#include<stdio.h>
#include<malloc.h>
#include<string.h>
#include "huffman.h"
#define INTERNAL 1
#define LEAF 0

typedef struct node
{
	char x;
	int freq;
	char *code;
	int type;
	struct node *next;
	struct node *left;
	struct node *right;
}node;

node *HEAD,*ROOT;

void printll();
void makeTree();
void genCode(node *p,char* code);
void insert(node *p,node *m);
void addSymbol(char c);
void writeHeader(FILE *f);
void writeBit(int b,FILE *f);
void writeCode(char ch,FILE *f);
char *getCode(char ch);

node* newNode(char c)
{
	node *q;
	q=(node *)malloc(sizeof(node));
	q->x=c;
	q->type=LEAF;	//leafnode
	q->freq=1;
	q->next=NULL;
	q->left=NULL;
	q->right=NULL;
	return q;
}


void printll()
{
node *p;
p=HEAD;

 while(p!=NULL)
 {
   printf("[%c|%d]=>",p->x,p->freq);
   p=p->next;
 }
}

int main(int argc, char *argv[])
{
FILE *fp,*fp2;
char ch;
int t;
HEAD=NULL;
ROOT=NULL;
if(argc<=2)
{
	printf("\n========================================\n");
	printf("  Huffman File Compressor\n");
	printf("  Author: Nilesh Akhade\n");
	printf("========================================\n");
	printf("\n  Usage: %s <input-file> [output-file]\n\n",argv[0]);
	if(argc==2)
	{
		argv[2]=(char *)malloc(sizeof(char)*(strlen(argv[1])+strlen(ext)+1));
		strcpy(argv[2],argv[1]);
		strcat(argv[2],ext);
		argc++;
	}
	else
		return 0;
}
fp=fopen(argv[1],"rb");
if(fp==NULL)
{
	printf("\n  [ERROR] Cannot open input file: %s\n\n",argv[1]);
	return -1;
}

printf("\n========================================\n");
printf("  Huffman File Compressor\n");
printf("========================================\n");
printf("\n  [Step 1/4] Analyzing input file...\n");
printf("             Source: %s\n",argv[1]);
	while(fread(&ch,sizeof(char),1,fp)!=0)
		addSymbol(ch);
	fclose(fp);

printf("  [Step 2/4] Building Huffman tree...\n");
makeTree();
printf("             Assigning codewords...\n");
genCode(ROOT,"\0");	

fp=fopen(argv[1],"r");
if(fp==NULL)
{
	printf("\n  [ERROR] Cannot reopen input file: %s\n\n",argv[1]);
	return -1;
}
fp2=fopen(argv[2],"wb");
if(fp2==NULL)
{
	printf("\n  [ERROR] Cannot create output file: %s\n\n",argv[2]);
	return -2;
}

printf("  [Step 3/4] Writing file header...\n");
printf("             Output: %s\n",argv[2]);
	writeHeader(fp2);
printf("  [Step 4/4] Writing compressed data...\n");
	while(fread(&ch,sizeof(char),1,fp)!=0)
	{	
		writeCode(ch,fp2);	
	}
fclose(fp);
fclose(fp2);

printf("\n  Compression complete!\n");
printf("========================================\n\n");
return 0;
}

void writeHeader(FILE *f)
{
symCode record;
node *p;
int temp=0,i=0;
p=HEAD;
while(p!=NULL)	
{
	temp+=(strlen(p->code)) * (p->freq);		
	if(strlen(p->code) > MAX) printf("\n  [WARNING] Codeword length exceeds MAX (%d).\n", MAX);	
	temp%=8;
	i++;
	p=p->next;
}

if(i==256)
	N=0;	
else 
	N=i;

fwrite(&N,sizeof(unsigned char),1,f);	
printf("             Unique symbols: %u\n",i);

p=HEAD;
while(p!=NULL)	
{
	record.x=p->x;
	strcpy(record.code,p->code);
	fwrite(&record,sizeof(symCode),1,f);
	p=p->next;
}
padding=8-(char)temp;	
fwrite(&padding,sizeof(char),1,f);
printf("             Padding bits:   %d\n",padding);
for(i=0;i<padding;i++)
	writeBit(0,f);

}

void writeCode(char ch,FILE *f)
{
char *code;
code=getCode(ch);
	while(*code!='\0')
	{
		if(*code=='1')
			writeBit(1,f); 
		else
			writeBit(0,f);
	code++;
	}
	return;
}

void writeBit(int b,FILE *f)
{
	static char byte;
	static int cnt;
	char temp;
	if(b==1)
	{	temp=1;
		temp=temp<<(7-cnt);		
		byte=byte | temp;
	}
	cnt++;
	
	if(cnt==8)	
	{
		fwrite(&byte,sizeof(char),1,f);
		cnt=0; byte=0;	
		return;
	}
	return;
}

char *getCode(char ch)
{
node *p=HEAD;
	while(p!=NULL)
	{
	    if(p->x==ch)
		  return p->code;
	    p=p->next;
	}
	return NULL; 
}

void insert(node *p,node *m)
{ 
if(m->next==NULL)
{  m->next=p; return;}
	while(m->next->freq < p->freq)
	{  m=m->next;
	  if(m->next==NULL)
	    { m->next=p; return; }
	}
  p->next=m->next;
  m->next=p;
}

void addSymbol(char c)
{
node *p,*q,*m;
int t;

if(HEAD==NULL)
{	HEAD=newNode(c);
	return;
}
	p=HEAD; q=NULL;
if(p->x==c)
{
	p->freq+=1;
	if(p->next==NULL)
		return;
	if(p->freq > p->next->freq)
	{
		HEAD=p->next;
		p->next=NULL;
		insert(p,HEAD);
	}
	return;
}

while(p->next!=NULL && p->x!=c)
{
	q=p; p=p->next;
}

if(p->x==c)
{
	p->freq+=1;
        if(p->next==NULL)
		return;	
	if(p->freq > p->next->freq)
	{
		m=p->next;
		q->next=p->next;
		p->next=NULL;
		insert(p,HEAD);
	}
}
else 
{	
	q=newNode(c);
	q->next=HEAD;  
	HEAD=q;
}
}

void makeTree()
{
node  *p,*q;
p=HEAD;
	while(p!=NULL)
	{
		q=newNode('@');
		q->type=INTERNAL;	
		q->left=p;		
		q->freq=p->freq;
		if(p->next!=NULL)
		{
			p=p->next;
			q->right=p;
			q->freq+=p->freq;
		}
		p=p->next;
		if(p==NULL)	
			break;
		if(q->freq <= p->freq)
		{
			q->next=p;
			p=q;
		}
		else
			insert(q,p);	
	}
	ROOT=q; 
}

void genCode(node *p,char* code)
{
char *lcode,*rcode;
static node *s;
static int flag;
if(p!=NULL)
{
	if(p->type==LEAF)  
	{	if(flag==0) 
		{flag=1; HEAD=p;}
		else	
		{ s->next=p;}		
		p->next=NULL;
		s=p;
	}

	p->code=code;	
	lcode=(char *)malloc(strlen(code)+2);
	rcode=(char *)malloc(strlen(code)+2);
	sprintf(lcode,"%s0",code);
	sprintf(rcode,"%s1",code);
	genCode(p->left,lcode);		
	genCode(p->right,rcode);
}
}

