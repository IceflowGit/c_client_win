Target=Client
Sources=client.c
L_Linux=gcc  -o $(Target)  $(Sources) -Wall
L_Windows=i686-w64-mingw32-gcc  -o $(Target).exe $(Sources) -lws2_32 -D_MINGW_ -mwin32 -Wall

client:
	$(L_Linux) 
	$(L_Windows)
clean:
	rm -rf Client Client.exe
