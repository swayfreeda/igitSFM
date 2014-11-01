//========================================================================//
//Description: a software for image based modeling,the input are some images, //              and the output is a textured model.
// Author: Sui Wei
// Time: 8/29/2014
// Company: Institute of Automation, Chinese Academy of Science




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
