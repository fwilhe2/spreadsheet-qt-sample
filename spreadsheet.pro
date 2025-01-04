QT += widgets
requires(qtConfig(treeview))
#unix:qtHaveModule(dbus): QT += dbus widgets

HEADERS += spreadsheet.h spreadsheetdelegate.h spreadsheetitem.h
SOURCES += main.cpp \
           spreadsheet.cpp \
           spreadsheetdelegate.cpp \
           spreadsheetitem.cpp
RESOURCES += spreadsheet.qrc

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/itemviews/spreadsheet
INSTALLS += target

