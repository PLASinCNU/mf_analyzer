int goo(){
	int i, j, k;
//[0]
	i = 1;
	j = i+1;

	/* 
	Lloop:
//[1]
	if (i>= 10) goto Lexit;
//[2]
	k = i*j;
	i++;
	goto Lloop;
	Lexit:
	*/
		
	while (i < 10)
	{ 
		k = i*j;
		i++;
	}

//[3]
	k = i-k;	
	i = 0;

	return 1;
}

/*
BASIC BLOCK[0]
  IN  :   k  	
  OUT : i  j  k  
BASIC BLOCK[1]
  IN  : i  j  k 
  OUT : i  j  k 
BASIC BLOCK[2]
  IN  : i  j   
  OUT : i  j  k 
BASIC BLOCK[3]
  IN  : i  k  	
  OUT : 
*/
