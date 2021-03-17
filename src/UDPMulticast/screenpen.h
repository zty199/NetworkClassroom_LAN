#ifndef SCREENPEN_H
#define SCREENPEN_H

#include <QWidget>
#include <QPen>
#include <QMouseEvent>

namespace Ui {
class ScreenPen;
}

class ScreenPen : public QWidget
{
    Q_OBJECT

public:
    explicit ScreenPen(QWidget *parent = nullptr);
    ~ScreenPen();

protected:
    void paintEvent(QPaintEvent *);

private:
    Ui::ScreenPen *ui;

    // 定义图形类型
    enum
    {
        NONE_DRAW,
        BRUSH_DRAW,
        LINE_DRAW,
        RECT_DRAW,
        ELLIPSE_DRAW
    } draw_type;

    QPoint startPnt;        // 起点
    QPoint endPnt;          // 终点
    QImage image, image_temp;

    QColor color;           // 画笔颜色
    QPen pen;               // 图形边框
    QBrush brush;           // 图形填充
    QColor bg_color;        // 背景颜色（白板或透明）
    bool flag_whiteboard;   // 白板模式

    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

    void drawCursor();
    void draw(QImage &image);
    void clear();

private slots:
    void on_btn_brush_clicked();
    void on_btn_line_clicked();
    void on_btn_rect_clicked();
    void on_btn_ellipse_clicked();
    void on_btn_color_clicked();
    void on_btn_whiteboard_clicked();
    void on_btn_exit_clicked();

    void clearDrawType();
    void addPenWidth();
    void reducePenWidth();

};

#endif // SCREENPEN_H
