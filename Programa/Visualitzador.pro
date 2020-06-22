TEMPLATE    = app
QT         += opengl 

INCLUDEPATH +=  /usr/include/glm

INCLUDEPATH += ../Model

FORMS += MyForm.ui

HEADERS += MyForm.h GLView.h

SOURCES += main.cpp MyForm.cpp \
        GLView.cpp ../Model/model.cpp
