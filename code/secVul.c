#include <stdio.h>
int f(int he);

int main(){
    int k;
    int j = 9;
    int reVal = 0;
    
    scanf("%d", &k);
    
    j = k; // 상관 o
    
    k = 1;
    reVal = f(k); // 일 경우 상관x
    
    return 0;
}

int f(int he){
    int i = 2;
    int k = he;
    int j = he+2;
    int h ;
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
