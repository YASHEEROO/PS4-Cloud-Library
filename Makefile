# إعدادات مسارات أداة OpenOrbis للـ PS4
Target          := PS34CloudLibrary
OutDir          := $(CURDIR)/bin

# لغات البرمجة والمترجمات المطلوبة
Compiler        := clang++
IncludeDir      := -I$(OO_PS4_TOOLCHAIN)/include -I$(OO_PS4_TOOLCHAIN)/include/SDL2

# ربط المكتبات المطلوبة (الرسوميات، الخطوط، الأمان، الشبكة)
LinkLibraries   := -lScePad -lSDL2 -lSDL2_image -lSDL2_ttf -lcurl -lssl -lcrypto

# أعلام التجميع للنظام
CompilerFlags   := -O2 -std=c++17 $(IncludeDir)
LinkerFlags     := -L$(OO_PS4_TOOLCHAIN)/lib $(LinkLibraries)

# ملفات المشروع
SourceFiles     := main.cpp
ObjectFiles     := $(SourceFiles:.cpp=.o)

all: $(Target).pkg

$(Target).elf: $(ObjectFiles)
	@mkdir -p $(OutDir)
	$(Compiler) $(ObjectFiles) -o $(OutDir)/$(Target).elf $(LinkerFlags)

# تحويل الكود المجمع إلى حزمة .pkg وهمية التوقيع (Fake Signed) جاهزة للـ PS4
$(Target).pkg: $(Target).elf
	@echo "[📦] جاري إنشاء حزمة الـ PKG النهائية للـ PS4..."
	create-pkg --input $(OutDir)/$(Target).elf --output $(OutDir)/$(Target).pkg --title "PS3/PS4 Cloud Library" --content-id UP0001-CLD123456_00-MAINAPPLICATION0

clean:
	rm -rf *.o $(OutDir)
