# XenoTech Library
by Tom Everaarts

Link with this library by adding -lxt when building your program!

This library is cross platform, however there are some problems.
All functions which exhibit problems on certain platforms have this noted in their documentation.

Dependencies :
When using this library, you need to link with other libraries as well... This is explained below.
Read this correctly! You do not need everything if certain library functions are not used.

The minimum versions listed below ensure that everything should work flawlessly.

Linux kernel 3.20+
#################################################################
Things that you may need to install (or should have) : 

libsdl2-* for all SDL2 linker options.
You should be able to link with -lpthread and -lGL, but your system should have these already!
-----------------------------------------------------------------
Linkage dependencies
####################
dlload.h: -rdynamic and -ldl
thread.h: -lpthread

Windows Vista+
#################################################################
Things that you may need to install (or should have) : 

You should have a ton of SDL2 DLL files bundled with your application so you can run them (Only for xtgl).
Keep in mind that there is a difference between x86 and x64.
-----------------------------------------------------------------
Linkage dependencies
####################
socket.h: -lws2_32
thread.h: -lntdll

Building the library: Just type (without quotes) "./configure && make" and everything should be done!

To compile and run the test, just run this command
Linux: 		gcc -g -std=c99 -L"lib" -I"include" test.c -o test -lxt -lpthread && ./test
Windows: 	gcc -g -std=c99 -L"lib" -I"include" test.c -o test.exe -lxt -lntdll -lws2_32 && ./test.exe
