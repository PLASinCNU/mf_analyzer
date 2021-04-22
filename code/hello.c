#include <stdio.h>

int main(){
	int k,m;
	int jni = 2;
	int fd = 5;

	scanf("%d", &k);

	if(k>4){
		m = jni+5;
		jni = k;
		printf("%d", jni);
	} 
	else{
		m = k+3;
		jni = 9;
		fd = 5+m;
	}

	m = jni+m;
	printf("%d\n", m);

	return 0;
}
