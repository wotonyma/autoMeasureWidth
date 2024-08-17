#pragma once

#include <QWidget>
#include <QImage>
#include <QRunnable>
#include <memory>

/*top-level wnd*/
class MeasureWidget : public QWidget
{
	Q_OBJECT

public:
	MeasureWidget(QWidget *parent = nullptr);
	~MeasureWidget();
};

/*camera control and option*/
class CamOptWidget : public QWidget
{
	Q_OBJECT

public:
	CamOptWidget(QWidget* parent = nullptr);
	~CamOptWidget();
	void initTree();
	void initOption();//选中相机创建窗口
};

/*algorithm option*/
class CalLineWidth;

/*return data */
using MeasData = std::vector<std::tuple<double, double, double>>;
//message proxy object
class ProxyMeasMsg : public QObject 
{
	Q_OBJECT

public:
	static ProxyMeasMsg& instance();
signals:
	void measImgaeMsg(QImage qimg);
	//void measFinshData(QStringList list);
	void measDataMsg(MeasData data);

private:
	ProxyMeasMsg();
	~ProxyMeasMsg() = default;
};

//threadpool task
class MeasTask : public QRunnable
{
public:
	virtual void run() override;
	std::unique_ptr<CalLineWidth> meas;
};

class MeasWidthOptWidget : public QWidget
{
	Q_OBJECT

public:
	MeasWidthOptWidget(QWidget* parent = nullptr);
	~MeasWidthOptWidget();

public slots:
	void measImageLaneWidth();

private:
	std::unique_ptr<CalLineWidth> meas;
	//thread?
};