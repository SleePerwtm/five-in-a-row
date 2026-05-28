# 五子棋 Qt 实现指南

## 0. 当前项目状态

```
board.h / board.cpp         — ✅ 已完成（核心逻辑完整）
chessboardwidget.h / .cpp   — ⚠️ 头文件有架子，实现是空的
game.h / game.cpp           — ❌ 空壳，需要从头实现
mainwindow.h / .cpp         — ⚠️ 只有骨架，需要补信号槽连接
mainwindow.ui               — ✅ UI 布局已完成
main.cpp                    — ⚠️ 混了测试代码，需要清理
CMakeLists.txt              — ✅ 构建配置已完成
```

---

## 1. 整体架构设计

### 1.1 核心思想：分层

四层结构，从上到下依赖：

```
┌─────────────────────────────┐
│  MainWindow (UI 层)          │  串联所有信号槽
│  按钮、状态栏、弹窗          │
└──────────┬──────────────────┘
           │
┌──────────▼──────────────────┐
│  Game (控制层)               │  游戏流程控制
│  落子、悔棋、重置、切换回合  │  只管逻辑，不管绘制
│  发出信号通知 UI 层          │
└──────────┬──────────────────┘
           │
┌──────────▼──────────────────┐
│  Board (数据+规则层)         │  棋盘数据存储
│  落子、查询、判胜负          │  纯 C++，不依赖 Qt
└──────────┬──────────────────┘
           │
┌──────────▼──────────────────┐
│  ChessBoardWidget (视图层)   │  棋盘绘制和鼠标交互
│  从 Board 读取数据来画       │  只负责"看"，不管逻辑
│  用户点击后发射信号          │
└─────────────────────────────┘
```

### 1.2 数据流

```
用户点击 → ChessBoardWidget 发射 cellClicked(row,col)
  → MainWindow 收到 → 调用 Game::placePiece(row,col)
    → Game 调用 Board::placeAt(col,row,Chess)
    → 检查胜负
    → 发射 piecePlaced(row, col, Chess) 信号
    → 发射 currentPlayerChanged(Chess) 信号
  → MainWindow 收到 piecePlaced → 调用 ChessBoardWidget::update() 重绘
  → MainWindow 收到 currentPlayerChanged → 更新状态栏文字
```

### 1.3 关键设计决策

**ChessBoardWidget 不做数据存储。** 它只持有 `Board*` 指针，画图时从 Board 读数据。棋盘数据只存在 Board 里一份（Game 拥有 Board），避免两份不同步。

**信号单向流动。** Game 发信号 → MainWindow 接收。MainWindow 调用 Game 的方法。Game 不知道 UI 存在。

---

## 2. 实现步骤

按照从底层到顶层的顺序：

### 步骤 0：Board 已经完成，无需修改

---

### 步骤 1：实现 ChessBoardWidget（视图层）

这个控件只做两件事：
1. **画棋盘**（从 Board 读取数据来画）
2. **处理鼠标点击**（转成棋盘坐标，发射信号）

#### 1.1 头文件需清理

当前 `chessboardwidget.h` 有重复 include 和一些多余声明，整理如下：

```cpp
#ifndef CHESSBOARDWIDGET_H
#define CHESSBOARDWIDGET_H

#include <QWidget>
#include "board.h"

class ChessBoardWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChessBoardWidget(QWidget* parent = nullptr);

    // 设置它要绘制的 Board 对象（由 MainWindow 调用）
    void setBoard(const Board* board);

    // 设置棋盘路数
    void setBoardSize(int size);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

signals:
    // 用户点击了合法交叉点 (row, col)
    void cellClicked(int row, int col);

private:
    int boardSize_ = 15;
    const Board* board_ = nullptr;

    // 像素坐标 → 棋盘行列，无效时 x() 返回 -1
    QPoint pixelToCell(int px, int py) const;

    // 棋盘行列 → 交叉点的像素坐标
    QPoint cellToPixel(int row, int col) const;

    int cellSize() const;
    int margin() const;
};

#endif
```

#### 1.2 实现

**核心 Qt 概念：**

- `QPainter painter(this)` — 创建一个以当前 Widget 为画布的画笔
- `paintEvent` — Qt 在需要重绘时自动调用，你只管在里面画
- `update()` — 告诉 Qt "我需要重绘"，Qt 会在下一帧调用 paintEvent
- `QPen` = 线条属性（颜色、粗细），`QBrush` = 填充属性

**坐标计算（以 15 路棋盘、Widget 600×600 为例）：**

```
推导：
  总宽度 W = 左边距 M + (N-1) 个格子间距 S + 右边距 M
  即：W = 2M + (N-1)·S
  取 M = S/2，代入得 W = S + (N-1)·S = N·S
  ∴ cellSize = W / N, margin = cellSize / 2

具体数值（W=600, N=15）：
  cellSize = 600 / 15  = 40
  margin   = 40 / 2    = 20

像素 → 棋盘：
  col = round((px - margin) / cellSize)
  row = round((py - margin) / cellSize)

棋盘 → 像素：
  px = margin + col * cellSize
  py = margin + row * cellSize
```

**`chessboardwidget.cpp`：**

```cpp
#include "chessboardwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>

ChessBoardWidget::ChessBoardWidget(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(600, 600);
}

void ChessBoardWidget::setBoard(const Board* board) {
    board_ = board;
    update();
}

void ChessBoardWidget::setBoardSize(int size) {
    boardSize_ = size;
    update();
}

// ========== 辅助坐标函数 ==========

int ChessBoardWidget::cellSize() const {
    return width() / boardSize_;
}

int ChessBoardWidget::margin() const {
    return cellSize() / 2;
}

QPoint ChessBoardWidget::cellToPixel(int row, int col) const {
    return QPoint(margin() + col * cellSize(),
                  margin() + row * cellSize());
}

QPoint ChessBoardWidget::pixelToCell(int px, int py) const {
    int m = margin();
    int cs = cellSize();

    int col = std::round((px - m) / static_cast<double>(cs));
    int row = std::round((py - m) / static_cast<double>(cs));

    if (col < 0 || col >= boardSize_ || row < 0 || row >= boardSize_)
        return QPoint(-1, -1);

    // 容差检查：离交叉点不超过 cellSize 的一半
    QPoint center = cellToPixel(row, col);
    int dx = px - center.x();
    int dy = py - center.y();
    if (dx * dx + dy * dy > (cs / 2) * (cs / 2))
        return QPoint(-1, -1);

    return QPoint(row, col);  // x=row, y=col
}

// ========== 绘制 ==========

void ChessBoardWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 抗锯齿

    int m = margin();
    int cs = cellSize();
    int last = boardSize_ - 1;

    // 1. 背景
    painter.fillRect(rect(), QColor("#DEB887"));

    // 2. 网格线
    painter.setPen(QPen(Qt::black, 1));
    for (int i = 0; i < boardSize_; ++i) {
        painter.drawLine(m, m + i * cs,
                         m + last * cs, m + i * cs);  // 横线
        painter.drawLine(m + i * cs, m,
                         m + i * cs, m + last * cs);  // 竖线
    }

    // 3. 星位
    int stars[][2] = {
        {3,3}, {3,7}, {3,11}, {7,3}, {7,7}, {7,11}, {11,3}, {11,7}, {11,11}
    };
    painter.setBrush(Qt::black);
    painter.setPen(Qt::NoPen);
    for (auto& s : stars) {
        if (s[0] < boardSize_ && s[1] < boardSize_) {
            QPoint c = cellToPixel(s[0], s[1]);
            painter.drawEllipse(c, 3, 3);
        }
    }

    // 4. 棋子（从 Board 读取数据）
    int radius = cs * 0.4;
    if (board_) {
        for (int r = 0; r < boardSize_; ++r) {
            for (int c = 0; c < boardSize_; ++c) {
                Chess chess = board_->at(c, r);  // Board 坐标是 (x,y) = (col,row)
                if (chess == Chess::kEmpty)
                    continue;

                QPoint center = cellToPixel(r, c);
                if (chess == Chess::kBlack) {
                    painter.setPen(Qt::black);
                    painter.setBrush(Qt::black);
                } else {
                    painter.setPen(Qt::black);
                    painter.setBrush(Qt::white);
                }
                painter.drawEllipse(center, radius, radius);
            }
        }
    }
}

// ========== 鼠标事件 ==========

void ChessBoardWidget::mouseReleaseEvent(QMouseEvent* event) {
    QPoint pos = event->pos();
    QPoint cell = pixelToCell(pos.x(), pos.y());

    if (cell.x() == -1) return;  // 无效位置

    int row = cell.x();
    int col = cell.y();

    // 检查是否已有棋子（从 Board 读取）
    if (board_ && !board_->isEmpty(col, row))
        return;

    emit cellClicked(row, col);
}
```

---

### 步骤 2：实现 Game（控制层）

Game 只管逻辑，不管 UI。它拥有一份 Board 实例，外部通过调用 Game 的方法来驱动游戏。

#### 2.1 需要理解的 Qt 概念

**信号（Signal）**：在 `signals:` 区域声明，不需要写实现体。Qt MOC 自动生成。

**发射信号**：用 `emit signalName(args)`。

**Q_OBJECT 宏**：凡是用了信号槽的类，必须在类声明中写 `Q_OBJECT`，且必须继承 `QObject`。

#### 2.2 `game.h`

```cpp
#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QPoint>
#include <vector>
#include "board.h"

class Game : public QObject {
    Q_OBJECT

public:
    explicit Game(int boardSize = 15, QObject* parent = nullptr);

    bool placePiece(int row, int col);  // 落子
    bool undo();                         // 悔棋
    void reset();                        // 重置

    Chess currentPlayer() const;
    const Board& board() const;          // 供 ChessBoardWidget 读取
    bool isGameOver() const;

signals:
    void piecePlaced(int row, int col, Chess chess);
    void pieceRemoved(int row, int col);
    void gameOver(Chess winner);
    void currentPlayerChanged(Chess player);
    void gameReset();

private:
    Board board_;
    Chess currentPlayer_ = Chess::kBlack;
    bool gameOver_ = false;
    std::vector<QPoint> moveHistory_;  // x=row, y=col

    void switchPlayer();
};

#endif
```

#### 2.3 `game.cpp`

```cpp
#include "game.h"

Game::Game(int boardSize, QObject* parent)
    : QObject(parent), board_(boardSize, boardSize)
{
}

bool Game::placePiece(int row, int col) {
    if (gameOver_)
        return false;

    // Board 坐标是 (x,y) = (col, row)
    if (!board_.placeAt(col, row, currentPlayer_))
        return false;

    moveHistory_.push_back(QPoint(row, col));

    emit piecePlaced(row, col, currentPlayer_);

    if (board_.hasFiveAt(col, row, currentPlayer_)) {
        gameOver_ = true;
        emit gameOver(currentPlayer_);
        return true;
    }

    switchPlayer();
    return true;
}

bool Game::undo() {
    if (moveHistory_.empty())
        return false;

    QPoint last = moveHistory_.back();
    moveHistory_.pop_back();
    int row = last.x();
    int col = last.y();

    board_.forceClear(col, row);

    gameOver_ = false;

    switchPlayer();

    emit pieceRemoved(row, col);
    return true;
}

void Game::reset() {
    board_.clear();
    moveHistory_.clear();
    currentPlayer_ = Chess::kBlack;
    gameOver_ = false;

    emit gameReset();
    emit currentPlayerChanged(currentPlayer_);
}

Chess Game::currentPlayer() const {
    return currentPlayer_;
}

const Board& Game::board() const {
    return board_;
}

bool Game::isGameOver() const {
    return gameOver_;
}

void Game::switchPlayer() {
    currentPlayer_ = (currentPlayer_ == Chess::kBlack) ? Chess::kWhite : Chess::kBlack;
    emit currentPlayerChanged(currentPlayer_);
}
```

> **注意：`Board::forceClear` 的用途**。由于 `Board::placeAt` 不允许覆盖已有棋子，悔棋时需要直接清空指定位置。所以需要在 Board 类中增加一个内部用的 `forceClear` 方法。

在 `board.h` 中加入：
```cpp
// 强制清空某位置（供悔棋使用，不检查是否已有棋子）
void forceClear(int x, int y);
```

在 `board.cpp` 中加入：
```cpp
void Board::forceClear(int x, int y) {
    if (inBound(x, y)) {
        pieces_[index(x, y)] = Chess::kEmpty;
    }
}
```

然后在 `Game::undo` 中，在 `switchPlayer()` 之前调用：
```cpp
board_.forceClear(col, row);
```

---

### 步骤 3：实现 MainWindow（UI 串联层）

MainWindow 只做一件事：用 `connect` 把各个部件的信号和槽连起来。

#### 3.1 `mainwindow.h`

```cpp
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

private:
    Ui::MainWindow* ui;
    Game* game_;

    void connectSignals();
};

#endif
```

#### 3.2 `mainwindow.cpp`

```cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "game.h"
#include "chessboardwidget.h"
#include "board.h"

#include <QPushButton>
#include <QMessageBox>
#include <QStatusBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    game_ = new Game(15, this);

    // 让 ChessBoardWidget 知道去哪里读棋盘数据
    ui->chessBoard->setBoard(&game_->board());

    connectSignals();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::connectSignals() {

    // 1. 用户点击棋盘 → Game 处理落子
    connect(ui->chessBoard, &ChessBoardWidget::cellClicked,
            game_, &Game::placePiece);
    // 注意：这里信号签名 (int row, int col) 和槽签名 (int row, int col) 完全匹配

    // 2. 落子成功 / 悔棋成功 → 棋盘重绘
    connect(game_, &Game::piecePlaced,
            ui->chessBoard, [this](int, int, Chess) {
                ui->chessBoard->update();
            });

    connect(game_, &Game::pieceRemoved,
            ui->chessBoard, [this](int, int) {
                ui->chessBoard->update();
            });

    // 3. 游戏重置 → 棋盘重绘 + 更新状态栏
    connect(game_, &Game::gameReset,
            ui->chessBoard, [this]() {
                ui->chessBoard->update();
                statusBar()->showMessage(QStringLiteral("当前：黑方落子"));
            });

    // 4. 游戏结束 → 弹窗
    connect(game_, &Game::gameOver,
            this, [this](Chess winner) {
                QString msg = (winner == Chess::kBlack)
                    ? QStringLiteral("黑方获胜！")
                    : QStringLiteral("白方获胜！");
                QMessageBox::information(this, QStringLiteral("游戏结束"), msg);
            });

    // 5. 当前玩家切换 → 更新状态栏
    connect(game_, &Game::currentPlayerChanged,
            this, [this](Chess player) {
                QString turn = (player == Chess::kBlack)
                    ? QStringLiteral("黑方") : QStringLiteral("白方");
                statusBar()->showMessage(
                    QStringLiteral("当前：") + turn + QStringLiteral("落子"));
            });

    // 6. 按钮事件
    // "新游戏"
    connect(ui->pushButton_2, &QPushButton::clicked,
            game_, &Game::reset);
    // "悔棋"
    connect(ui->pushButton, &QPushButton::clicked,
            game_, &Game::undo);
    // "退出"
    connect(ui->pushButton_3, &QPushButton::clicked,
            this, &MainWindow::close);
}
```

---

### 步骤 4：清理 `main.cpp`

删掉所有 Board 测试代码，只保留 Qt 启动逻辑：

```cpp
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char** argv) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();  // 等价于 QCoreApplication::exec()
}
```

---

## 3. Chess 枚举与 Qt 信号槽

Qt 信号槽传递自定义类型需要注册。在 `board.cpp` 文件顶部加入：

```cpp
#include <QMetaType>

// 在全局作用域注册 Chess 枚举类型
namespace {
    struct Registrar {
        Registrar() {
            qRegisterMetaType<Chess>("Chess");
        }
    };
    static Registrar chessRegistrar;
}
```

---

## 4. 编译运行

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
./five-in-a-row
```

如果需要重新构建（比如改了 .h 文件但 make 没检测到）：

```bash
rm -rf build && mkdir build && cd build
cmake ..
make -j$(nproc)
```

---

## 5. 各文件改动清单

| 文件 | 改动内容 |
|------|---------|
| `board.h` | 加 `forceClear` 声明 |
| `board.cpp` | 加 `forceClear` 实现、加 Chess 枚举注册 |
| `chessboardwidget.h` | 重写（清理重复 include，调整接口） |
| `chessboardwidget.cpp` | 实现 paintEvent、mouseReleaseEvent |
| `game.h` | 从头编写 |
| `game.cpp` | 从头编写 |
| `mainwindow.h` | 加入 `Game*` 成员和 `connectSignals` |
| `mainwindow.cpp` | 实现构造函数和所有 connect |
| `main.cpp` | 删测试代码，只保留 Qt 启动 |

---

## 6. 常见编译与运行问题

**Q: 报 `vtable` 或 `undefined reference` 错误**
A: 先确保所有含 `Q_OBJECT` 的 .h 文件都在 `CMakeLists.txt` 的 `qt_add_executable` 中列出。如果已列出仍报错，执行 `rm -rf build && cmake ..` 让 MOC 重新生成。

**Q: `Chess` 类型在 connect 中报错 "Type is not registered"**
A: 加第 3 节中的注册代码。

**Q: 棋盘画出来了但点击没反应**
A: 检查 `mouseReleaseEvent` 是否正确写了大写 `Release`（不是 `Press`），是否 override 了基类虚函数。

**Q: 棋子位置对不齐**
A: 确保 `pixelToCell` 和 `cellToPixel` 中的公式一致：都用 `margin() + index * cellSize()`。

**Q: ChessBoardWidget 在 .ui 文件里显示不出来**
A: 检查 `.ui` 文件中 `<customwidget>` 段的 `<extends>QWidget</extends>` 和 `<header>chessboardwidget.h</header>` 是否正确。
