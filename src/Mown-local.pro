#-------------------------------------------------
#
# Project created by QtCreator 2015-01-23T18:27:58
#
#-------------------------------------------------

BOOST_PATH = "C:\local\boost_1_57_0"

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#QT += webkit webkitwidgets
QT += webkitwidgets

TARGET = Mown
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
        contentfactory.cpp \
        article.cpp \
        mown.cpp \
        articletag.cpp \
        clickablestatusbar.cpp \
        projectsettings.cpp

HEADERS  += mainwindow.h \
            contentfactory.h \
            article.h \
            mown.h \
            articletag.h \
            qdebugstream.h \
            clickablestatusbar.h \
            projectsettings.h

FORMS    += mainwindow.ui

CONFIG += c++11

win32-g++ {
    INCLUDEPATH += $$BOOST_PATH
    DEPENDPATH += $$BOOST_PATH\stage\lib

    LIBS += "$$BOOST_PATH\stage\lib\libboost_filesystem-mgw49-mt-1_57.a"
    LIBS += "$$BOOST_PATH\stage\lib\libboost_system-mgw49-mt-1_57.a"
    LIBS += "$$BOOST_PATH\stage\lib\libboost_date_time-mgw49-mt-1_57.a"
}


win32-msvc2012
{
    INCLUDEPATH += $$BOOST_PATH

    INCLUDEPATH += C:\local\yaml-cpp-0.5.1\include
    LIBS += C:\local\yaml-cpp-0.5.1\MinSizeRel\libyaml-cppmd.lib
}

win32-msvc2013
{
    #INCLUDEPATH += $$BOOST_PATH
    #DEPENDPATH += $$BOOST_PATH\stage

    #INCLUDEPATH += D:\Dev\yaml-cpp-0.5.1\include
    #LIBS += D:\Dev\yaml-cpp-0.5.1\build\x64\MinSizeRel\libyaml-cppmd.lib


    #LIBS += $$BOOST_PATH\stage\lib\libboost_filesystem-vc120-mt-1_57.lib
    #LIBS += $$BOOST_PATH\stage\lib\libboost_system-vc120-mt-1_57.lib
    #LIBS += $$BOOST_PATH\stage\lib\libboost_date_time-vc120-mt-1_57.lib

    #LIBS += $$BOOST_PATH\lib

}
message($$CONFIG)

