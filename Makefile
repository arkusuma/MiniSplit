# You need Visual C++ 6.0 to use this Makefile
# And also UPX (http://upx.tsx.org) to compress the EXE

CC = cl
CFLAGS = /nologo /O1 /MD
LIBS = kernel32.lib user32.lib comdlg32.lib shell32.lib ole32.lib comctl32.lib advapi32.lib
OBJS = main.res crc32.obj utils.obj main.obj big_file.obj
LINK = /link /machine:ix86 /opt:nowin98

MiniSplit.exe: $(OBJS)
	$(CC) $(CFLAGS) /FeMiniSplit.exe $(OBJS) $(LIBS) $(LINK)
	strip -s MiniSplit.exe
	upx --best MiniSplit.exe
		
big_file.obj: big_file.c
	$(CC) $(CFLAGS) /c big_file.c

main.obj: main.c
	$(CC) $(CFLAGS) /c main.c

utils.obj: utils.c
	$(CC) $(CFLAGS) /c utils.c

crc32.obj: crc32.c
	$(CC) $(CFLAGS) /c crc32.c

main.res: main.rc
	rc main.rc

clean:
	del MiniSplit.exe *.o* main.res
