# تعريف المترجم والمسارات إجبارياً
CC              := clang++
Target          := PS34CloudLibrary
OutDir          := $(CURDIR)/bin

# مسارات التضمين والمكتبات من الـ SDK المفتوح
IncludeDir      := -I$(OO_PS4_TOOLCHAIN)/include -I$(OO_PS4_TOOLCHAIN)/include/SDL2
LinkerFlags     := -L$(OO_PS4_TOOLCHAIN)/lib -lScePad -lSDL2 -lSDL2_image -lSDL2_ttf -lcurl -lssl -lcrypto

# قواعد التجميع الصارمة لمنع المترجم الافتراضي من التدخل
all: $(Target).pkg

main.o: main.cpp
	$(CC) -c main.cpp -o main.o -O2 -std=c++17 $(IncludeDir)

$(Target).elf: main.o
	@mkdir -p $(OutDir)
	$(CC) main.o -o $(OutDir)/$(Target).elf $(LinkerFlags)

$(Target).pkg: $(Target).elf
	@echo "[📦] جاري إنشاء حزمة الـ PKG النهائية للـ PS4..."
	create-pkg --input $(OutDir)/$(Target).elf --output $(OutDir)/$(Target).pkg --title "PS3/PS4 Cloud Library" --content-id UP0001-CLD123456_00-MAINAPPLICATION0

clean:
	rm -rf *.o $(OutDir)
