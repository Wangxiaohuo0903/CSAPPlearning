#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
//csim.c by huo
typedef struct
{
    int valid;
    int tag;
    int wait_time;
} cache_line,*cache_set,**cache;
    int s_num,E_num,b_num;
    int S_num;
    int v_flag=0;
    char *file_txt;
    cache test_cache=NULL;
    int hit=0,miss=0,evi=0;
    int h_flag=0,m_flag=0,e_flag=0;
void init_cache()
{
test_cache=(cache)malloc(sizeof(cache_set) * S_num);
	for(int i = 0; i < S_num; ++i)
	{
		test_cache[i] = (cache_set)malloc(sizeof(cache_line) * E_num);
		for(int j = 0; j < E_num; ++j)
		{
			test_cache[i][j].valid = 0;
			test_cache[i][j].tag = -1;
			test_cache[i][j].wait_time = -1;
		}
	}
    return;
}
void update(unsigned int address)
{
int tag=address>>(b_num+s_num);
int set= (address <<(64-b_num-s_num))>>(64-s_num);
//printf("\nAAA%d,%d\n",tag,set);
int find_tag=0;
//try to find,if tag=-1,insert one
for(int i=0;i<E_num;i++)
{
    if(test_cache[set][i].tag==-1) //empty
    {
        test_cache[set][i].tag=tag;
        test_cache[set][i].wait_time=-1;
        find_tag=1;
        miss++;
        if(v_flag)
        printf("miss ");
        //m_flag=1;
        break;
    }
    else if(test_cache[set][i].tag==tag)
    {
        test_cache[set][i].wait_time=-1;
        find_tag=1;
        hit++;
        if(v_flag)
        printf("hit ");
        //h_flag=1;
        break;
    }

}
//010000
//100000
//110000
//all wait_time add 1

    for(int j=0;j<E_num;j++)
    {
        if(test_cache[set][j].tag!=-1)
        test_cache[set][j].wait_time++;
    }

//if not find,exchange one
if(!find_tag){
int tmp=test_cache[set][0].wait_time;
int t_mun=0;
for(int i=1;i<E_num;i++)
{
if(tmp<test_cache[set][i].wait_time)
{tmp=test_cache[set][i].wait_time;
t_mun=i;
}
}
//printf("EVICTION from %x to %x",test_cache[set][t_mun].tag,tag);
test_cache[set][t_mun].tag=tag;
test_cache[set][t_mun].wait_time=0;
evi++;
miss++;
// m_flag=1;
// e_flag=1;
if(v_flag)
printf("miss eviction ");
}

return;

}


int main(int argc, char *argv[])
{
	char ch;
//read seq

	while((ch = getopt(argc, argv, "hvs:E:b:t:")) != EOF)
	{
		switch(ch)
		{
			case 'h':
printf("\
Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n\
Options:\n\
  -h         Print this help message.\n\
  -v         Optional verbose flag.\n\
  -s <num>   Number of set index bits.\n\
  -E <num>   Number of lines per set.\n\
  -b <num>   Number of block offset bits.\n\
  -t <file>  Trace file.\n\
\n\
Examples:\n\
  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n\
  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
				break;

            case 's':
                s_num=atoi(optarg);
                S_num=1<<s_num;
               // printf("s_num:%d\n",s_num)
                break;
            case 'E':
                E_num=atoi(optarg);
                break;
            case 'b':
                b_num=atoi(optarg);
                break;
            case 't':
                file_txt=optarg;
                //strcpy(file_txt,optarg);
                break;
            case 'v':
                v_flag=1;
                break;
			default:
				return 1;
		}
	} 

//init cache
init_cache(test_cache);

//read trace
    FILE* fp = fopen(file_txt, "r"); // 读取文件名
	if(fp == NULL)
	{
		printf("open error");
		exit(-1);
	}
	
	char operation;         // 命令开头的 I L M S
	unsigned int address;   // 地址参数
	int size;               // 大小
	while(fscanf(fp, " %c %x,%d\n", &operation, &address, &size) > 0)
	{
        if(v_flag)
        printf(" %c %x,%d\t", operation, address, size);
        switch (operation)
        {
        case 'I':
            /* code */
            break;
        case 'L':
            update(address);
            break;
        case 'M':
            update(address);
        case 'S':
            update(address);
            break;            
        default:
            break;
        }
        if(v_flag)
        printf("\n");
        
    }
	fclose(fp);
	for(int i = 0; i < S_num; ++i)
		free(test_cache[i]);
	free(test_cache); 
    printSummary(hit, miss, evi);
	return 0;
}
