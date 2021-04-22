int goo1(){
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

int goo2(){
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
