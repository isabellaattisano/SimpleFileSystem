CC = gcc
CFLAGS = -c

xshell: fs.o disk.o fs_shell.o password.o
	$(CC) fs.o disk.o fs_shell.o password.o -o xshell

fs_shell.o: fs_shell.c
	$(CC) $(CFLAGS) fs_shell.c

fs.o: fs.c fs.h
	$(CC) $(CFLAGS) fs.c

disk.o: disk.c disk.h
	$(CC) $(CFLAGS) disk.c	

password.o: password.c
	$(CC) $(CFLAGS) password.c	

clean:
	rm *.o xshell

# target:dependencies 
# 	action 
# top target is primary target
# recompile target everytime the dependecies change which is the action 
