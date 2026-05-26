#include "mainwindow.h"

#include "board.h"
#include "game.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  game_ = new Game(15, this);
  ui->chessBoard->set_board(&game_->board());

  connectSignals();
}

MainWindow::~MainWindow() { delete ui; }

ChessBoardWidget* MainWindow::chessBoardWidget() const {
  return ui->chessBoard;
}

void MainWindow::connectSignals() {
  // 1. 用户点击棋盘 -> Game 处理落子
  connect(ui->chessBoard, &ChessBoardWidget::cellClicked, game_,
          &Game::placePiece);

  // 2. 落子成功 -> 棋盘重绘
  connect(game_, &Game::piecePlaced, ui->chessBoard,
          [this](int, int, Chess) { ui->chessBoard->update(); });

  // 3. 悔棋成功 -> 棋盘重绘
  connect(game_, &Game::pieceRemoved, ui->chessBoard,
          [this](int, int) { ui->chessBoard->update(); });

  // 4. 游戏重置 -> 棋盘重绘 + 状态栏
  connect(game_, &Game::gameReset, ui->chessBoard, [this]() {
    ui->chessBoard->update();
    statusBar()->showMessage(QStringLiteral("当前：黑方落子"));
  });

  // 5. 游戏结束 -> 弹窗
  connect(game_, &Game::gameOver, this, [this](Chess winner) {
    QString msg = (winner == Chess::kBlack) ? QStringLiteral("黑方获胜！")
                                            : QStringLiteral("白方获胜！");
    QMessageBox::information(this, QStringLiteral("游戏结束"), msg);
  });

  // 6. 玩家切换 -> 更新状态栏
  connect(game_, &Game::currentPlayerChanged, this, [this](Chess player) {
    QString turn = (player == Chess::kBlack) ? QStringLiteral("黑方")
                                             : QStringLiteral("白方");
    statusBar()->showMessage(QStringLiteral("当前：") + turn +
                             QStringLiteral("落子"));
  });

  // 7. 按钮事件
  connect(ui->newGameButton, &QPushButton::clicked, game_, &Game::reset);
  connect(ui->undoButton, &QPushButton::clicked, game_, &Game::undo);
  connect(ui->exitButton, &QPushButton::clicked, this, &MainWindow::close);
}
