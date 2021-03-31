default:
	@echo Specify the target system please.
	@echo Targets: linux-64, windows-32, windows-64, all
	@echo Input: make [Your target]
linux-64:
	x86_64-linux-gnu-gcc Tab2Spaces.c -o ../Bin/x86_64-linux-gnu/Tab2Spaces
	x86_64-linux-gnu-gcc Tab2Spaces_4space.c -o ../Bin/x86_64-linux-gnu/Tab2Spaces_4space
windows-32:
	i686-w64-mingw32-gcc Tab2Spaces.c -o ../Bin/Win32/Tab2Spaces.exe
	i686-w64-mingw32-gcc Tab2Spaces_4space.c -o ../Bin/Win32/Tab2Spaces_4space.exe
windows-64:
	x86_64-w64-mingw32-gcc Tab2Spaces.c -o ../Bin/Win64/Tab2Spaces.exe
	x86_64-w64-mingw32-gcc Tab2Spaces_4space.c -o ../Bin/Win64/Tab2Spaces_4space.exe
all:
	make linux-64
	make windows-32
	make windows-64
