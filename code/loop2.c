int goo(){
	int i, j, k;
//[0]
	i = 1;
	j = i+1;

/* 
//[1]	
	Lloop: 
	if (i>= 10)  goto Lexit;
//[2]
	i++;
	k= i*j;
	goto Lloop;

	Lexit:
*/


	while (i < 10) {
		i++;
		k = i*j;
	} 
// [3]
	i = 0;
	k = i-k;	

	return 1;
}
/*
BASIC BLOCK[0]
  USE :   
  DEF :   add  i  j  
  IN  :   k  
  OUT : i  j  k  
BASIC BLOCK[1]
  USE  : i  
  DEF :   cmp  
  IN  : i  j  k  
  OUT :   i  j  k    
BASIC BLOCK[2]
  USE  : i  j  
  DEF :   i  inc  k  mul  
  IN  : i  j 
  OUT : i  j  k  
BASIC BLOCK[3]
  USE :   k  
  DEF :   i  k  sub  
  IN  :   k  
  OUT : 
*/
