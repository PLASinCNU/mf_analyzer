int foo(){
	int i, j, k;
	i = 1;
	j = i+1;

	if (i == j)
	{ 
		k = i*j;
		i = 0;
	}
	else 
	{
		j = k+1;
	}

	k = i-j;	

	return 1;
}
/*
BASIC BLOCK[0]
  IN  :   k  
  OUT : i  j  k  
BASIC BLOCK[1]
  IN  : i  j  
  OUT : i  j  
BASIC BLOCK[2]
  IN  : i  k  
  OUT : i  j  
BASIC BLOCK[3]
  IN  : i  j  
  OUT : 
*/
