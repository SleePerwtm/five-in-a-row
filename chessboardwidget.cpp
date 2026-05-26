#include "chessboardwidget.h"

#include "board.h"

#include <QColor>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPoint>

ChessBoardWidget::ChessBoardWidget(QWidget* parent) : QWidget(parent) {}

/* 设置要绘制的 Board 对象 */
void ChessBoardWidget::set_board(const Board* board) {
  board_ = board;
  update();
}

/* 设置棋盘路数 */
void ChessBoardWidget::set_board_size(int size) { board_size_ = size; }

/* 重写的方法，用来绘制棋盘 */
void ChessBoardWidget::paintEvent(QPaintEvent* event) {
  // 以当前 Widget 为画布创建画家
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿

  int marg1n = margin();
  int cell_size = cellSize();
  int last = board_size_ - 1;

  // 设置背景色为木色
  painter.fillRect(rect(), QColor{"#DEB887"});

  // 绘制网格线
  painter.setPen(QPen{Qt::black, 1});
  for (int i = 0; i < board_size_; ++i) {
    // 绘制横线
    painter.drawLine(marg1n, marg1n + i * cell_size, marg1n + last * cell_size,
                     marg1n + i * cell_size);
    // 绘制竖线
    painter.drawLine(marg1n + i * cell_size, marg1n, marg1n + i * cell_size,
                     marg1n + last * cell_size);
  }

  // 星位
  int stars[][2] = {{3, 3},  {3, 7},  {3, 11}, {7, 3},  {7, 7},
                    {7, 11}, {11, 3}, {11, 7}, {11, 11}};
  painter.setBrush(Qt::black);
  painter.setPen(Qt::NoPen);
  for (auto& s : stars) {
    if (s[0] < board_size_ && s[1] < board_size_) {
      QPoint c = cellToPixel(s[0], s[1]);
      painter.drawEllipse(c, 3, 3);
    }
  }

  // 棋子
  int radius = cell_size * 0.4;
  if (board_) {
    for (int row = 0; row < board_size_; ++row) {
      for (int col = 0; col < board_size_; ++col) {
        Chess chess = board_->at(col, row);
        if (chess == Chess::kEmpty) {
          continue;
        }

        QPoint center = cellToPixel(row, col);
        painter.setPen(Qt::black);
        if (chess == Chess::kBlack) {
          painter.setBrush(Qt::black);
        } else {
          painter.setBrush(Qt::white);
        }
        painter.drawEllipse(center, radius, radius);
      }
    }
  }
}

/* 重写的方法，用来处理点击逻辑 */
void ChessBoardWidget::mouseReleaseEvent(QMouseEvent* event) {
  QPoint pos = event->pos();
  QPoint cell = pixelToCell(pos.x(), pos.y());

  if (cell.x() == -1)
    return;

  int row = cell.x();
  int col = cell.y();

  if (board_ && !board_->isEmpty(col, row))
    return;

  emit cellClicked(row, col);
}

int ChessBoardWidget::cellSize() const { return width() / board_size_; }

int ChessBoardWidget::margin() const { return cellSize() / 2; }

bool ChessBoardWidget::inBound(int px, int py) const {
  if (px > width() || py > height()) {
    return false;
  }
  return true;
}

/* 像素坐标 -> 棋盘行列 */
QPoint ChessBoardWidget::pixelToCell(int px, int py) const {
  int col = -1, row = -1;
  if (inBound(px, py)) {
    col = px / cellSize();
    row = py / cellSize();
  }
  return QPoint{row, col};
}

/* 棋盘行列 -> 交叉点的像素坐标 */
QPoint ChessBoardWidget::cellToPixel(int row, int col) const {
  int px, py;
  px = margin() + col * cellSize();
  py = margin() + row * cellSize();
  return QPoint{px, py};
}