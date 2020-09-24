#include <stdio.h>

int main(){
	printf("hello world\n");
	for (int i = 0; i < 1425; i++){
		for (int j = 0; j < 80; j++){
			char a = 'A' + (random() % 26);
			fprintf(stdout, "%c", a);
		}
		fprintf(stdout, "\n");
	}
}