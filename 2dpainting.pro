QT += opengl
QT += multimedia
HEADERS = glwidget.h \
    helper.h \
    window.h \
    soundmaker.h \
    generator.h
SOURCES = glwidget.cpp \
    helper.cpp \
    main.cpp \
    window.cpp \
    soundmaker.cpp \
    generator.cpp
LIBS += -L/opt/local/lib \
    -lsndfile

INCLUDEPATH += /opt/local/include/

# install
target.path = $$[QT_INSTALL_EXAMPLES]/opengl/2dpainting
sources.files = $$SOURCES \
    $$HEADERS \
    $$RESOURCES \
    $$FORMS \
    2dpainting.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/opengl/2dpainting
INSTALLS += target \
    sources
symbian:include($$QT_SOURCE_TREE/examples/symbianpkgrules.pri)
RESOURCES += 
OTHER_FILES += 
