#!/bin/bash

echo "[🌐] 1. جاري تنزيل وتثبيت أدوات PS4 SDK سحابياً..."
wget -q https://github.com
tar -xzf OpenOrbis-PS4-SDK.tar.gz
rm OpenOrbis-PS4-SDK.tar.gz

# تعريف مسارات المترجم تلقائياً
export OO_PS4_TOOLCHAIN=$(pwd)/OpenOrbis-PS4-SDK
export PATH=$PATH:$OO_PS4_TOOLCHAIN/bin

echo "[📂] 2. جاري إنشاء مجلدات النظام وتنزيل الخط السحابي التلقائي..."
mkdir -p bin/data
# تنزيل خط مفتوح المصدر ومجاني وتسميته بالشكل الصحيح للكود
wget -q -O bin/data/font.ttf "https://github.com"

echo "[⚙️] 3. جاري بدء تجميع الكود المدمج وصناعة التطبيق..."
make

echo "[🎉] 4. اكتمل العمل بنجاح! ستجد ملف الـ PKG النهائي الآن داخل مجلد bin باسم PS34CloudLibrary.pkg"
