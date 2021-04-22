#include <stdio.h>
#include <iostream>

string writeURL(string u);

int main(){
	string url;
	int a = 2;

	url = writeURL();

	std::cout << url << "\n";

	return 0;
}


string writeURL(string u){
	string temp;

	std::cout << "input : ";
	std::cin >>temp;

	return temp;
	
}
