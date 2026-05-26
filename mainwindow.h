#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "chessboardwidget.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Game;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow() override;

  ChessBoardWidget* chessBoardWidget() const;

private:
  Ui::MainWindow* ui;
  Game* game_;

  void connectSignals();
};

#endif // MAINWINDOW_H
