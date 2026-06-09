#include "mainwindow.h"

#include "board.h"
#include "game.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  game_ = new Game(15, this);
  ui->chessBoard->set_board(&game_->board());

  // 复盘按钮默认隐藏
  ui->forwardButton->setVisible(false);
  ui->backwardButton->setVisible(false);
  ui->exitReplayButton->setVisible(false);

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

  // 7. 原有按钮事件
  connect(ui->newGameButton, &QPushButton::clicked, game_, &Game::reset);
  connect(ui->undoButton, &QPushButton::clicked, game_, &Game::undo);
  connect(ui->exitButton, &QPushButton::clicked, this, &MainWindow::close);

  // 8. 保存棋局
  connect(ui->saveGameButton, &QPushButton::clicked, this, [this]() {
    QString path = QFileDialog::getSaveFileName(
        this, QStringLiteral("保存棋局"), QString(),
        QStringLiteral("JSON 文件 (*.json)"));
    if (path.isEmpty())
      return;
    if (game_->saveToFile(path)) {
      statusBar()->showMessage(QStringLiteral("棋局已保存"), 3000);
    } else {
      QMessageBox::warning(this, QStringLiteral("保存失败"),
                           QStringLiteral("无法保存棋局文件"));
    }
  });

  // 9. 加载棋局（进入复盘模式）
  connect(ui->loadGameButton, &QPushButton::clicked, this, [this]() {
    // 如果正在对弈中，提示用户
    QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("加载棋局"), QString(),
        QStringLiteral("JSON 文件 (*.json)"));
    if (path.isEmpty())
      return;
    if (!game_->loadFromFile(path)) {
      QMessageBox::warning(this, QStringLiteral("加载失败"),
                           QStringLiteral("无法加载棋局文件"));
    }
  });

  // 10. 复盘前进
  connect(ui->forwardButton, &QPushButton::clicked, game_, &Game::stepForward);

  // 11. 复盘后退
  connect(ui->backwardButton, &QPushButton::clicked, game_,
          &Game::stepBackward);

  // 12. 退出复盘
  connect(ui->exitReplayButton, &QPushButton::clicked, game_,
          &Game::stopReplay);

  // 13. 复盘模式开始
  connect(game_, &Game::replayStarted, this, [this]() {
    // 隐藏正常模式按钮
    ui->newGameButton->setEnabled(false);
    ui->undoButton->setEnabled(false);
    ui->saveGameButton->setEnabled(false);
    ui->loadGameButton->setEnabled(false);
    // 显示复盘按钮
    ui->forwardButton->setVisible(true);
    ui->backwardButton->setVisible(true);
    ui->exitReplayButton->setVisible(true);
    statusBar()->showMessage(QStringLiteral("复盘模式 — 第 0 / %1 步")
                                 .arg(game_->replayTotalSteps()));
  });

  // 14. 复盘模式结束
  connect(game_, &Game::replayEnded, this, [this]() {
    // 恢复正常模式按钮
    ui->newGameButton->setEnabled(true);
    ui->undoButton->setEnabled(true);
    ui->saveGameButton->setEnabled(true);
    ui->loadGameButton->setEnabled(true);
    // 隐藏复盘按钮
    ui->forwardButton->setVisible(false);
    ui->backwardButton->setVisible(false);
    ui->exitReplayButton->setVisible(false);
    ui->chessBoard->update();
    statusBar()->showMessage(QStringLiteral("当前：黑方落子"));
  });

  // 15. 复盘步数变化
  connect(game_, &Game::replayStepChanged, this, [this](int step, int total) {
    statusBar()->showMessage(
        QStringLiteral("复盘模式 — 第 %1 / %2 步").arg(step).arg(total));
  });
}
