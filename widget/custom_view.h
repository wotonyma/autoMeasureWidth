#pragma once

#include <QGraphicsView>
#include <memory>
#include <optional>
#include <QImage>

class CustomView : public QGraphicsView
{
	Q_OBJECT

public:
	CustomView(QWidget *parent = nullptr);
	~CustomView();

	void setImage(QImage qimg);

signals:
	void anchorPixelChanged(QPointF pt);	//鼠标在图片上移动坐标变化信号
	void anchorPressedPixel(QPointF pt);	//鼠标点击图片某像素信号

protected:
	void wheelEvent(QWheelEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;

private:
	std::optional<QPointF> mousePos2ImagePixel(QMouseEvent* event) const;

	QGraphicsScene* _scene{ nullptr };
	std::unique_ptr<QGraphicsItem> _item; //ccd img item
};
