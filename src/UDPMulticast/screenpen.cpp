#include "screenpen.h"
#include "ui_screenpen.h"

#include <QScreen>
#include <QShortcut>
#include <QPainter>
#include <QTimer>
#include <QColorDialog>

ScreenPen::ScreenPen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScreenPen),
    color(Qt::red),
    pen(QPen(QBrush(Qt::red), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)),
    brush(QBrush(Qt::transparent, Qt::SolidPattern)),
    bg_color(QColor(0, 0, 0, 1)),
    flag_whiteboard(false)
{
    ui->setupUi(this);

    ui->screenPen_menu->setCursor(Qt::PointingHandCursor);
    ui->screenPen_menu->move((QGuiApplication::primaryScreen()->geometry().width() - ui->screenPen_menu->size().width()) / 2, 0);

    setWindowFlag(Qt::FramelessWindowHint, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    showFullScreen();

    connect(new QShortcut(QKeySequence(Qt::Key_Escape), this), SIGNAL(activated()), this, SLOT(clearDrawType()));
    connect(new QShortcut(QKeySequence(Qt::Key_Plus), this), SIGNAL(activated()), this, SLOT(addPenWidth()));
    connect(new QShortcut(QKeySequence(Qt::Key_Equal), this), SIGNAL(activated()), this, SLOT(addPenWidth()));
    connect(new QShortcut(QKeySequence(Qt::Key_Minus), this), SIGNAL(activated()), this, SLOT(reducePenWidth()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this), SIGNAL(activated()), this, SLOT(on_btn_exit_clicked()));

    clear();
    this->setFocus();
}

ScreenPen::~ScreenPen()
{
    delete ui;
}

void ScreenPen::mousePressEvent(QMouseEvent *e)
{
    startPnt = e->pos();
    endPnt = e->pos();
    if(e->buttons() & Qt::RightButton)
    {
        clear();
    }
}

void ScreenPen::mouseReleaseEvent(QMouseEvent *)
{
    image = image_temp;
}

void ScreenPen::mouseMoveEvent(QMouseEvent *e)
{
    if(e->buttons() & Qt::LeftButton)
    {
        if(draw_type == BRUSH_DRAW)
        {
            startPnt = endPnt;
        }
        endPnt = e->pos();

        if(draw_type == BRUSH_DRAW)
        {
            image = image_temp;
        }
        else
        {
            image_temp = image;
        }
        draw(image_temp);
    }
}

void ScreenPen::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), bg_color); // 设置透明颜色   * 全透明（0）在 Windows 系统下会使鼠标信号穿透 z轴，故透明度设为 1
    painter.drawImage(0, 0, image_temp);
}

void ScreenPen::drawCursor()
{
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(pen);
    switch(draw_type)
    {
    case BRUSH_DRAW:
    {
        painter.setPen(pen.color());
        painter.setBrush(pen.color());
        QPolygon polygon;
        polygon << QPoint(0, pixmap.height())
                << QPoint(pen.width(), pixmap.height())
                << QPoint(pen.width(), pixmap.height() - pen.width())
                << QPoint(0, pixmap.height() - pen.width());
        painter.drawPolygon(polygon);
        break;
    }
    case LINE_DRAW:
        painter.drawLine(pixmap.width(), 0, 0, pixmap.height());
        break;
    case RECT_DRAW:
        painter.drawRect(pen.width(), pixmap.height() - pen.width() - 16, 24, 16);
        break;
    case ELLIPSE_DRAW:
        painter.drawEllipse(pen.width(), pixmap.height() - pen.width() - 16, 24, 16);
        break;
    default:
        unsetCursor();
        return;
    }
    setCursor(QCursor(pixmap, 0, pixmap.height()));
}

void ScreenPen::draw(QImage &image)
{
    QPainter painter(&image);
    painter.setPen(pen);
    painter.setBrush(brush);
    painter.setRenderHint(QPainter::Antialiasing, true);
    switch(draw_type)
    {
    case BRUSH_DRAW:
        painter.drawLine(startPnt, endPnt);
        break;
    case LINE_DRAW:
        painter.drawLine(startPnt, endPnt);
        break;
    case RECT_DRAW:
    {
        painter.setBrush(QBrush(Qt::transparent, Qt::SolidPattern));
        QRect rect(startPnt, endPnt);
        painter.drawRect(rect);
        break;
    }
    case ELLIPSE_DRAW:
    {
        QRect rect(startPnt, endPnt);
        painter.setBrush(QBrush(Qt::transparent, Qt::SolidPattern));
        painter.drawEllipse(rect);
        break;
    }
    default:
        break;
    }
    update();
}

void ScreenPen::clear()
{
    image_temp = QImage(QGuiApplication::primaryScreen()->geometry().width(), QGuiApplication::primaryScreen()->geometry().height(), QImage::Format_ARGB32);
    image_temp.fill(Qt::transparent);
    image = image_temp;
    update();
}

void ScreenPen::on_btn_brush_clicked()
{
    draw_type = BRUSH_DRAW;
    drawCursor();
}

void ScreenPen::on_btn_line_clicked()
{
    draw_type = LINE_DRAW;
    drawCursor();
}

void ScreenPen::on_btn_rect_clicked()
{
    draw_type = RECT_DRAW;
    drawCursor();
}

void ScreenPen::on_btn_ellipse_clicked()
{
    draw_type = ELLIPSE_DRAW;
    drawCursor();
}

void ScreenPen::on_btn_color_clicked()
{
    color = QColorDialog::getColor(color, this);
    if(color.isValid())
    {
        pen.setColor(color);
        drawCursor();
    }
}

void ScreenPen::on_btn_whiteboard_clicked()
{
    clear();
    if(flag_whiteboard)
    {
        bg_color = QColor(0, 0, 0, 1);
    }
    else
    {
        bg_color = QColor(255, 255, 255, 255);
    }
    this->repaint();
    flag_whiteboard = !flag_whiteboard;
}

void ScreenPen::on_btn_exit_clicked()
{
    this->close();
}

void ScreenPen::clearDrawType()
{
    draw_type = NONE_DRAW;
    drawCursor();
}

void ScreenPen::addPenWidth()
{
    if(pen.width() < 15)
    {
        pen.setWidth(pen.width() + 1);
        if(draw_type != NONE_DRAW)
        {
            drawCursor();
        }
    }
}

void ScreenPen::reducePenWidth()
{
    if(pen.width() > 1)
    {
        pen.setWidth(pen.width() - 1);
        if(draw_type != NONE_DRAW)
        {
            drawCursor();
        }
    }
}
