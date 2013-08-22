contains(QMAKE_TARGET.arch, x86_64):{
  INCLUDEPATH += "C:\OpenSSL-Win64\include"
  LIBPATH += "C:\OpenSSL-Win64\lib"
}

contains(QMAKE_TARGET.arch, x86):{
  INCLUDEPATH += "C:\OpenSSL-Win32\include"
  LIBPATH += "C:\OpenSSL-Win32\lib"
}

LIBS      += -llibeay32

CONFIG(debug, debug|release) {
    DESTDIR = debug
} else {
    DESTDIR = release
}
