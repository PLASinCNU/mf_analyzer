#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int f(int he);

int main(){
    char buf[10];
    int readResult;
    int tmp = 0;
    int j = 0;

     scanf("%d\n", &readResult);

    if(j == 0){
        tmp = f(readResult);
        
        printf("%d\n", tmp);
    }

    return 0;
}

int f(int he){
    int i = 2;
    int k = he;
    int j = he+2;
    int h;
    //    int k = p;

    scanf("%d", &h);
    
    if( k > 2 ){
        printf("Hello User. k is : %d\n", h);
    }
    else if(i == 2){
        printf("Changed : %d\n", j);
    }
    else{
        printf("Changed : %d\n", i);
    }

    return 0;
}
