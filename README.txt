
name : yisrael bar

explaintion:
its a simultion of disk memory mengment , how the disk create files open them reads form them, delete and write...
for each file we have inode (a structure) that points to blocks on disk and there will be the file data.


to compile:
press ctrl +shift+b

to run the program: 
press ctrl+f5
or throw the terminal and write ./exf

the files of program exf:
makefile,folder(.vscode),DISK_SIM_FILE.txt, exf, main.cpp


input: you eneter the following numbers:
0) delete and exit
1)print all the disk
2)format - gat a blcok size and how meny direct entries  
3)create-file - get a file name
4)open-file - get a file name
5)close-file - get a number of (fd)
6) write-file- get fd and string to write to the disk 
7) read from file - get fd and length to read
8)delete file


output: 
accordingly to the data you enter , if we enter 7 0 4 and the data at the start of fd 0 is abcdefgh it will print abcd.



