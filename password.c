#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void delay(int secs_delay){
    //This function creates a delay for file system use after the password is incorrect a certain number of times

    int milli_secs = 1000 * secs_delay; //converts seconds to milli seconds
    clock_t start_time = clock(); //current time since program was launched
    clock_t delay = start_time + milli_secs; //current time since program was launched + delay time

    while (clock() < delay){ //empty for loop that runs until delay is complete
    }

}
char* create_pass(){	
	char pass[50];
	char pass_hold[50];

	printf("Choose a password greater than 5 characters long: " );
	fgets(pass, 50, stdin);
	//printf("%s", pass);
	int len_pass = strlen(pass);

	if(strlen(pass) <=5){ //checks if pass is greater than 5
		printf("The password entered does not meet the required length");
		while(strlen(pass)<=5){ //loops until password meets requirements
			//printf("%s and %ld", pass, strlen(pass));
			printf("\nEnter password that meets the requirement: ");
			fgets(pass, 50, stdin);
		}
	}

	printf("Enter password again for verification: "); //verification of password
	fgets(pass_hold, 50, stdin);

	if(strcmp(pass, pass_hold)!=0){ //checks if passwords are equivalent
		while(strcmp(pass, pass_hold) != 0){ //loops until password is the same as first entered
			printf("the passwords do not match, try again: ");
			fgets(pass_hold, 50, stdin);
		}
	}
	char* dyn_hold = (char*)malloc(50); //creates a string stored in dynamic memory
	strcpy(dyn_hold, pass); //copies password string to dynamic memory
	return dyn_hold; //returns data that is not local
}

void check_pass(char pass[], int num){
	int error = 0;
	char pass_hold[50];
	if(num == 0){ //read file
		printf("Must Enter Password to Read File: ");
	}
	else if(num==1){ //num is 1, write to file
		printf("Must Enter Password to Write to File: ");
	}
	else{
		printf("Must Enter Password to Remove File: ");
	}

    fgets(pass_hold, 50, stdin);
	error++; //increase number of guesses

	while(strcmp(pass, pass_hold) != 0){ //compares stored password with entered password
		if(error == 3){
			printf("\n3 Incorrect Guesses. File System is Disabled for 3 Minutes.");
			delay(9000); //if four wrong guesses delay 3 minutes
		}
		if(error == 6){ 
			printf("\n6 Incorrect Guesses. File System is Disabled for 5 Minutes.");
			delay(300000); //five minute disabled
		}
		else{
			printf("\nIncorrect. Enter again: ");
			fgets(pass_hold, 50, stdin);
		}
		error++;	
	}
}

//int main(){
	//create_pass();
//	check_pass("holder");
    //char password[] = "1234";
    //char password_hold[] = "";
    //printf("Enter password: ");
    //scanf("%s",password_hold);
    //printf("\n%s", password_hold);
    
    //if(strcmp(password, password_hold) != 0){
      //  delay(3000); //I times this and 300,000 delays for 5 minutes 
	//printf("yeah");
//    }
//}
