#include <mainwindow.h>
#include "mainwindow.h"
#include <QApplication>
#include <stdlib.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.resize(1200,600);
    w.show();
 
    return a.exec();
}
