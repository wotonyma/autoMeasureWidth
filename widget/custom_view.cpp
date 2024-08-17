#include "custom_view.h"
#include <QApplication>
#include <QGraphicsItem>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <iostream>


CustomView::CustomView(QWidget *parent)
	: QGraphicsView(parent)
{					//设置场景
	this->setViewport(new QOpenGLWidget(this));
	this->viewport()->setMouseTracking(true);	//设置跟踪鼠标
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //设置滑动条关闭
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setDragMode(QGraphicsView::ScrollHandDrag);			//设置鼠标拖拽模式
	//QApplication::setOverrideCursor(Qt::ArrowCursor);//一直生效(设置鼠标形状)
	this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse); //设置鼠标点为操作焦点,如缩放之类
	this->setResizeAnchor(QGraphicsView::AnchorUnderMouse);

	_scene = new QGraphicsScene(this);
	this->setScene(_scene);
	_item = std::make_unique<QGraphicsPixmapItem>();
	_scene->addItem(_item.get());

	_item->setCursor(Qt::ArrowCursor);
	this->setCursor(Qt::ArrowCursor);
	this->viewport()->setCursor(Qt::ArrowCursor);
}

CustomView::~CustomView()
{
	_scene->removeItem(_item.get());
}

void CustomView::setImage(QImage qimg)
{
	auto pixmap_item = dynamic_cast<QGraphicsPixmapItem*>(_item.get());
	if (pixmap_item->pixmap().isNull())	//first set
	{
		double kw = this->viewport()->width() / (double)qimg.width();
		double kh = this->viewport()->height() / (double)qimg.height();
		double k = std::min(kw, kh);
		this->scale(k, k);
	}
	try {
		dynamic_cast<QGraphicsPixmapItem*>(_item.get())->setPixmap(QPixmap::fromImage(qimg));
	} 
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	
}

void CustomView::wheelEvent(QWheelEvent* event)
{
	int wheelDeltaVal = event->delta();
	if (wheelDeltaVal > 0) {
		this->scale(1.1, 1.1);
	}
	else {
		this->scale(1 / 1.1, 1 / 1.1);
	}
	QGraphicsView::wheelEvent(event);
}

void CustomView::mouseMoveEvent(QMouseEvent* event)
{
	auto opt_pixel = mousePos2ImagePixel(event);
	if (opt_pixel.has_value())
		emit anchorPixelChanged(opt_pixel.value());
	QGraphicsView::mouseMoveEvent(event);
}

void CustomView::mousePressEvent(QMouseEvent* event)
{
	auto opt_pixel = mousePos2ImagePixel(event);
	if (opt_pixel.has_value())
		emit anchorPressedPixel(opt_pixel.value());
	QGraphicsView::mousePressEvent(event);
}

//获取鼠标焦点所在的图像像素点坐标
inline std::optional<QPointF> CustomView::mousePos2ImagePixel(QMouseEvent* event) const
{
	auto mouse_pos = event->pos();
	auto scene_pos = mapToScene(mouse_pos);
	auto item = itemAt(mouse_pos);
	if (item != nullptr)
		return item->mapFromScene(scene_pos);
	return std::optional<QPointF>();
}
