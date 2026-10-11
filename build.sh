#!/bin/bash
echo "Starting Time:" `date`


BUILD_NAME="installer"
ADDITIONAL_LIBS="/usr/lib/libcrypto++.a /usr/lib/gcc/x86_64-linux-gnu/4.9/libstdc++.a"

rm -rf $BUILD_NAME

g++ -std=c++14 \
-g \
main.cpp \
Ini/ini.c \
plog/Log.h \
Ini/inireader.cpp \
tsl/ordered_map.h \
-o $BUILD_NAME \
-L/usr/lib \
-pthread \
-I/usr/local/src/sources/fpanel/installer/ \
-I/usr/include/ \
-I/usr/local/include/ \
/usr/local/lib/libboost_system.a \
/usr/local/lib/libboost_program_options.a \
/usr/local/lib/libboost_filesystem.a \
/usr/local/lib/libboost_thread.a \
/usr/local/lib/libboost_regex.a \
/usr/local/lib/libboost_iostreams.a \
/usr/local/lib/libboost_date_time.a \
$ADDITIONAL_LIBS \
-lcrypto \
-Wl,--whole-archive -lpthread -Wl,--no-whole-archive \
-g2 \
-w \
-ggdb \
-O3 \
-Wall \
-ldl \
-lm \
-Wunused-parameter \
-Wunused-variable \
-Wparentheses \
-Wno-sign-compare \
-fpermissive \
-pedantic \
-Wextra \
-Wno-missing-field-initializers \
-Wno-return-type \
-Wno-unused-parameter \
-Wno-unused-variable \
-Wno-maybe-uninitialized \
-Wno-unused \
-fext-numeric-literals
#-static \
RETVAL=$?
if [ "$RETVAL" != "0" ]
  then
      echo "Error"
  else
#      sshpass -p "a" scp installer root@192.168.139.42:/root/
#      echo "Uploaded to 42"
#      scp $BUILD_NAME root@repo.fpanel.net:/var/www/repo.fpanel.net/
fi
echo "Finished Time:" `date`


# ./installer --fast --hostname=debian.test.fpanel.net --apache --perl --python --php72 --mysql --bind --postfix --proftpd --dovecot --spamassassin --roundcube --phpmyadmin --awstats --webalizer --verbose
# --postgresql --pgadmin
