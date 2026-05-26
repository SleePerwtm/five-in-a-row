#include "board.h"

#include <iostream>

#include "mainwindow.h"

#include <QApplication>

int main(int argc, char** argv) {
  std::cout << "Hello, from five-in-a-row!\n";

  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return QCoreApplication::exec();
}
