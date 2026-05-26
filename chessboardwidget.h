#ifndef CHESSBOARDWIDGET_H
#define CHESSBOARDWIDGET_H

#include "board.h"

#include <QPoint>
#include <QWidget>

class ChessBoardWidget : public QWidget {
  Q_OBJECT

public:
  explicit ChessBoardWidget(QWidget* parent = nullptr);

  /* 设置要绘制的 Board 对象 */
  void set_board(const Board* board);
  /* 设置棋盘路数 */
  void set_board_size(int size);

protected:
  /* 重写的方法，用来绘制棋盘 */
  void paintEvent(QPaintEvent* event) override;
  /* 重写的方法，用来处理点击逻辑 */
  void mouseReleaseEvent(QMouseEvent* event) override;

signals:
  /* 用户点击了某个交叉点，通过 Game 类处理 */
  void cellClicked(int row, int col);

private:
  int board_size_ = 15;
  const Board* board_ = nullptr;

  int cellSize() const;
  int margin() const;

  bool inBound(int px, int py) const;

  /* 像素坐标 -> 棋盘行列 */
  QPoint pixelToCell(int px, int py) const;
  /* 棋盘行列 -> 交叉点的像素坐标 */
  QPoint cellToPixel(int row, int col) const;
};

#endif // CHESSBOARDWIDGET_H