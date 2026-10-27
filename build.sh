#!/bin/bash

echo "[🌐] 1. جاري تنزيل وتثبيت أدوات PS4 SDK المستقرة..."
# استخدام رابط مباشر ومضمون ومستقر من السيرفر الرسمي
rm -rf OpenOrbis-PS4-SDK OpenOrbis-PS4-SDK.tar.gz
wget -q --no-check-certificate https://github.com

if [ ! -f OpenOrbis-PS4-SDK.tar.gz ]; then
    echo "[❌] فشل التنزيل من السيرفر الرئيسي، جاري المحاولة من السيرفر الاحتياطي..."
    wget -q --no-check-certificate https://github.com
fi

tar -xzf OpenOrbis-PS4-SDK.tar.gz
rm OpenOrbis-PS4-SDK.tar.gz

# تعريف المسارات والبيئة إجبارياً
export OO_PS4_TOOLCHAIN=$(pwd)/OpenOrbis-PS4-SDK
export PATH=$PATH:$OO_PS4_TOOLCHAIN/bin

echo "[📂] 2. جاري إنشاء مجلدات النظام وتنزيل الخط السحابي..."
mkdir -p bin/data
wget -q -O bin/data/font.ttf "https://github.com"

echo "[⚙️] 3. جاري بدء التجميع النظيف عبر محرك سوني..."
# حذف الملفات القديمة وبدء تجميع جديد وصارم باستخدام المترجم الصحيح
rm -f main.o bin/PS34CloudLibrary.elf bin/PS34CloudLibrary.pkg
make CC="clang++"

if [ -f bin/PS34CloudLibrary.pkg ]; then
    echo "[🎉] 4. كفو! اكتمل العمل بنجاح! ستجد ملف الـ PKG النهائي الآن داخل مجلد bin باسم PS34CloudLibrary.pkg"
else
    echo "[❌] حدثت مشكلة أثناء ربط المكتبات، يرجى مراجعة المطور."
fi
