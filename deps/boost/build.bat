.\bootstrap.bat --with-libraries=system,context,coroutine,thread,regex,random,atomic,date_time,asio,beast
.\b2 --toolset=clang-win stage debug release link=static runtime-link=static -j2
.\b2  stage debug release link=static runtime-link=static -j2