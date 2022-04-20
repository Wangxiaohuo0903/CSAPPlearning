/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
//(s=5, E=1, b=5)
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;
    if (M == 32)
    {
        int k;
        for (i = 0; i < N; i += 8)
        {
            for (j = 0; j < M; j += 8)
            {
                for (k = i; k < 8 + i; k++)
                {
                        int v1, v2, v3, v4, v5, v6, v7, v8;
                        v1 = A[k][j];
                        v2 = A[k][j + 1];
                        v3 = A[k][j + 2];
                        v4 = A[k][j + 3];
                        v5 = A[k][j + 4];
                        v6 = A[k][j + 5];
                        v7 = A[k][j + 6];
                        v8 = A[k][j + 7];
                        B[j][k] = v1;
                        B[j + 1][k] = v2;
                        B[j + 2][k] = v3;
                        B[j + 3][k] = v4;
                        B[j + 4][k] = v5;
                        B[j + 5][k] = v6;
                        B[j + 6][k] = v7;
                        B[j + 7][k] = v8;
                    
                }
            }
        }
    }
    else if (M == 64)
    {
        
        for(i=0;i<64;i+=8)
        {
            for(j=0;j<64;j+=8)
            {
                //a b insert and reverse
                int v0,v1,v2,v3,v4,v5,v6,v7;
                for(int k=0;k<4;k++)
                {  
                    v0=A[k+i][j];v1=A[k+i][j+1];v2=A[k+i][j+2];v3=A[k+i][j+3];
                    v4=A[k+i][j+4];v5=A[k+i][j+5];v6=A[k+i][j+6];v7=A[k+i][j+7];
                    B[j][i+k]=v0;B[j+1][i+k]=v1;B[j+2][i+k]=v2;B[j+3][i+k]=v3;
                    B[j][i+k+4]=v4;B[j+1][i+4+k]=v5;B[j+2][i+4+k]=v6;B[j+3][i+4+k]=v7;

                }
                for(int k=0;k<4;k++)
                {
                    v4=B[j+k][i+4];v5=B[k+j][i+5];v6=B[k+j][i+6];v7=B[k+j][i+7];
                    v0=A[i+4][j+k];v1=A[i+5][j+k];v2=A[i+6][j+k];v3=A[i+7][j+k];
                    B[k+j][i+4]=v0;B[k+j][i+5]=v1;B[k+j][i+6]=v2;B[k+j][i+7]=v3;
                    B[k+j+4][i]=v4;B[k+j+4][i+1]=v5;B[k+j+4][i+2]=v6;B[k+j+4][i+3]=v7;
                    
                }
                for(int k=0;k<4;k++)
                {  
                    
                    v4=A[k+i+4][j+4];v5=A[k+i+4][j+5];v6=A[k+i+4][j+6];v7=A[k+i+4][j+7];
                    B[j+4][i+k+4]=v4;B[j+5][i+4+k]=v5;B[j+6][i+4+k]=v6;B[7+j][i+4+k]=v7;

                }
               
   

            }
        }
    }
        else 
    {
        //int b = 8; //M=61,N=67，b=8，
        //int tmp;
        //int en = b * (N / b), em = b * (M / b);
        int k;
        for (i = 0; i <= 64; i += 8)
        {
            for (j = 0; j <= 56; j += 8)
            {
                for (k = i; k < 8 + i && k < N; k++)
                {
                    //if(1==0)
                    if(i<64&&j<56)
                    {

                        int v1, v2, v3, v4, v5, v6, v7, v8;
                        v1 = A[k][j];
                        v2 = A[k][j + 1];
                        v3 = A[k][j + 2];
                        v4 = A[k][j + 3];
                        v5 = A[k][j + 4];
                        v6 = A[k][j + 5];
                        v7 = A[k][j + 6];
                        v8 = A[k][j + 7];
                        B[j][k] = v1;
                        B[j + 1][k] = v2;
                        B[j + 2][k] = v3;
                        B[j + 3][k] = v4;
                        B[j + 4][k] = v5;
                        B[j + 5][k] = v6;
                        B[j + 6][k] = v7;
                        B[j + 7][k] = v8;
                    }
                    else
                    for (int jj = j; jj < 8 + j && jj < M; jj++)
                    {
                   
                        B[jj][k] = A[k][jj];
                    }
                }
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
//(s=5, E=1, b=5)
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                return 0;
            }
        }
    }
    return 1;
}
