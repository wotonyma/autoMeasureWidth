#include "measure_widget.h"

#include "qlayout.h"
#include "qtabwidget.h"
#include "qsplitter.h"
#include "qtreeview.h"
#include "qformlayout.h"
#include "qstandarditemmodel.h"
#include "qheaderview.h"
#include "qpushbutton.h"
#include "qlineedit.h"
#include "qspinbox.h"
#include "qdebug.h"
#include "qdir.h"
#include "qdatetime.h"
#include "qfiledialog.h"
#include "qcheckbox.h"
#include <QtConcurrent>

#include <charconv>

#include "custom_view.h"
#include "Basler.h"
#include "cam_worker.h"
#include "cal_line_width.h"
#include "fmt/printf.h"
#include "spdlog/spdlog.h"


MeasureWidget::MeasureWidget(QWidget *parent)
	: QWidget(parent)
{
	this->resize(720, 480);
	this->setAttribute(Qt::WA_DeleteOnClose);
	auto lyt_v = new QVBoxLayout(this);
	auto lyt_v_h0 = new QHBoxLayout;
	lyt_v->addLayout(lyt_v_h0);

	auto cam_view = new CustomView;
	cam_view->setObjectName("view");
	auto tab_wnd = new QTabWidget;
	tab_wnd->setObjectName("tab");

	auto cam_opt = new CamOptWidget;
	tab_wnd->addTab(cam_opt, "camera");

	auto meas_opt = new MeasWidthOptWidget;
	tab_wnd->addTab(meas_opt, "measure");

	auto splitter = new QSplitter(Qt::Horizontal);
	QString style = QString("QSplitter::handle { background-color: rgb(179, 179, 179); }") //分割线的颜色
		+ QString("QSplitter {border: 2px solid green}");
	splitter->setStyleSheet(style);
	splitter->setHandleWidth(5);
	splitter->setChildrenCollapsible(false);//不允许把分割出的子窗口拖小到0，最小值被限定为sizeHint或maxSize/minSize
	splitter->addWidget(cam_view);
	splitter->addWidget(tab_wnd);
	splitter->setStretchFactor(0, 8);//设置初始比例
	splitter->setStretchFactor(0, 2);
	//splitter->setSizes({ 500,220 });//拉伸会比例拉伸

	lyt_v_h0->addWidget(splitter);

	//display img slot
	connect(&ProxyMeasMsg::instance(), &ProxyMeasMsg::measImgaeMsg, cam_view, &CustomView::setImage);
}

MeasureWidget::~MeasureWidget()
{
}

/******************************camera option**********************/
CamOptWidget::CamOptWidget(QWidget* parent)
{
	/*
	* tree_view[camera list]
	* -----------
	* open / close
	* exposure Time
	* save camera
	*/
	auto lyt_v = new QVBoxLayout(this);
	lyt_v->setObjectName("lyt_v");

	initTree();
	initOption();
}

CamOptWidget::~CamOptWidget()
{
}

void CamOptWidget::initTree()
{
	auto lyt_v = this->findChild<QVBoxLayout*>("lyt_v");

	auto tree_v = new QTreeView;
	tree_v->setObjectName("tree");
	tree_v->header()->hide();
	tree_v->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置节点不可编辑

	lyt_v->addWidget(tree_v, 1);

	QStandardItemModel* model = new QStandardItemModel(tree_v);
	/*QStandardItem 的所有权会被转交给model, 随model释放*/
	auto gige = new QStandardItem("Gige");
	/*auto usb = new QStandardItem("USB");
	auto clink = new QStandardItem("CamLink");*/
	QList cam_class = { gige };
	QStandardItem* xd = new QStandardItem();

	auto usr_ids = Basler::DetectCameras(); //获取basler网口相机列表
	QList<QStandardItem*> item_list;
	for (auto& usr_id : usr_ids)
		item_list << new QStandardItem(usr_id.c_str());

	gige->appendColumn(item_list);
	model->appendColumn(cam_class);

	tree_v->setModel(model);
	tree_v->expandAll();

}

void CamOptWidget::initOption()
{
	auto lyt_v = this->findChild<QVBoxLayout*>("lyt_v");

	auto opt_w = new QWidget;
	auto form = new QFormLayout(opt_w);

	auto ctl_btn = new QPushButton("打开相机");
	auto exp_box = new QSpinBox;
	exp_box->setRange(0, INT32_MAX);
	auto save_btn = new QPushButton("exec");
	form->addRow("相机操作:", ctl_btn);
	form->addRow("曝光时间:", exp_box);
	form->addRow("保存图片:", save_btn);

	ctl_btn->setMaximumWidth(80);
	exp_box->setMaximumWidth(80);
	save_btn->setMaximumWidth(80);

	lyt_v->addWidget(opt_w, 3);
	opt_w->hide();
	opt_w->setStyleSheet(".QWidget{border:1px solid gray}");

	CamWorker::instance().start();

	auto bindCameraById = [=](QString usr_id) {
		//the same camera, do nothing.
		if (CamWorker::instance().userIdList.front() == usr_id.toStdString())
			return;
		opt_w->disconnect();

		auto cam = CamWorker::instance().cam_vec.front().get();
		cam->closeCamera();
		CamWorker::instance().userIdList = { usr_id.toStdString() };
		
		auto pa = this->topLevelWidget();
		auto view = pa->findChild<CustomView*>("view");

		connect(ctl_btn, &QPushButton::clicked, [this, cam, view, ctl_btn, exp_box, save_btn] {
			if (cam->isOpen()) {
				QMetaObject::invokeMethod(cam, [cam, view] {
					cam->disconnect(cam, &CamOBJ::grabbedImageSig, view, &CustomView::setImage);
					cam->closeCamera();
					});
				ctl_btn->setText("打开相机");
				save_btn->setDisabled(true);
			}
			else {
				QMetaObject::invokeMethod(cam, [cam, exp_box,view] { 
					cam->connect(cam, &CamOBJ::grabbedImageSig, view, &CustomView::setImage);
					auto cur_usr_id = CamWorker::instance().userIdList.at(0);
					cam->openCameraByUserID(cur_usr_id);
					cam->setFreeRunMode();
					cam->startGrab();
					emit exp_box->setValue(cam->getCam()->intExp); 
					});
				ctl_btn->setText("关闭相机");
				save_btn->setEnabled(true);
			}
			});

		connect(exp_box, &QSpinBox::editingFinished, [exp_box, cam] {		//exp
			cam->setExposureTimeRaw(exp_box->value());
			});

		connect(save_btn, &QPushButton::clicked, [] {
			auto opt_img = CamWorker::instance().cam_vec[0]->getCameraCurrentImage();
			if (!opt_img.has_value())
			{
				//ProxyMsgObject::instance().insertHtmlMsg("ccd camera get current image failed!", 3);
				qDebug() << "ccd camera get current image failed!";
				return;
			}
			auto& qimg = opt_img.value();

			auto usrName = QDir::home().dirName();
			QDateTime curDateTime = QDateTime::currentDateTime();
			auto strTime = curDateTime.toString("yyyy-MM-dd__hh-mm-ss");
			auto imgName = QString("Image__") + strTime + ".bmp";
			auto fullName = QString("C:/Users/%1/Pictures/%2").arg(usrName).arg(imgName);

			QString fileName = QFileDialog::getSaveFileName(nullptr,
				"Save Image", fullName,
				"Images (*.png *.bmp *.jpg *.tif *.ppm *.GIF);;All Files (*)");
			if (fileName.isEmpty()) {
				//ProxyMsgObject::instance().insertHtmlMsg("save image failed!", 3);
				qDebug() << "save image failed!";
				return;
			}
			qimg.save(fileName);

			});	
		};

	auto tree_v = this->findChild<QTreeView*>("tree");
	connect(tree_v, &QTreeView::doubleClicked, [bindCameraById, opt_w](const QModelIndex& index) {
		if (!index.parent().isValid())
			return;
		qDebug() << index.data().toString();
		bindCameraById(index.data().toString());
		opt_w->show();
		});
}

/*measure alogrithm option and parameter*/
MeasWidthOptWidget::MeasWidthOptWidget(QWidget* parent)
	: meas(std::make_unique<CalLineWidth>())
{
	auto lyt_v = new QVBoxLayout(this);

	//algorithm argument setting widget/////////////////////////////
	auto argu_w = new QWidget;
	argu_w->setStyleSheet(".QWidget{border:1px solid gray}");
	auto lyt_v_f = new QFormLayout(argu_w);

	auto add_check_box = [this, lyt_v_f](const QString& str, bool& st) {
		auto cb = new QCheckBox;
		cb->setChecked(st);
		connect(cb, &QCheckBox::stateChanged, [this, cb, &st] { st = cb->isChecked(); });
		lyt_v_f->addRow(str, cb);
		};

	auto add_line_edit = [this, lyt_v_f](const QString& str, auto&& val) {
		auto edit = new QLineEdit;
		edit->setText(QString::number(val));
		connect(edit, &QLineEdit::returnPressed, [this, edit, &val] {
			std::string str_val = edit->text().toStdString();
			std::from_chars_result res = std::from_chars(str_val.data(), str_val.data() + str_val.size(), val);
			if (res.ec != std::errc{})
				std::cout << str_val << " charconv failed!" << std::endl;
			});
		lyt_v_f->addRow(str, edit);
		};

	add_check_box("close edges", meas->edges_close_enable);
	add_line_edit("min edge len", meas->min_edge_len);
	add_line_edit("max dist thresh", meas->max_dist_thresh);
	add_line_edit("avg dist thresh", meas->avg_dist_thresh);
	add_check_box("detect node", meas->detect_node_enable);
	add_check_box("subpixel node", meas->subpixel_enable);
	add_line_edit("downsampled", meas->downsampled_ratio);
	add_line_edit("max node", meas->max_node);
	add_line_edit("node mark size", meas->node_mark_size);
	add_line_edit("min fit pt num", meas->min_fit_pt_num);
	add_line_edit("min theta diff", meas->min_theta_diff);

	//algorithm result display widget////////////////////////
	auto res_w = new QWidget;
	res_w->setStyleSheet(".QWidget{border:1px solid gray}");
	auto lyt_v_f2 = new QFormLayout(res_w);
	auto le_pix_len = new QLineEdit("1.0");
	auto le_outer_dist = new QLineEdit;
	auto le_inner_dist = new QLineEdit;
	auto le_mid_dist = new QLineEdit;
	auto btn_meas_exec = new QPushButton("measure");
	auto btn_continue_grab = new QPushButton("continue");

	lyt_v_f2->addRow("像素宽度:", le_pix_len);
	lyt_v_f2->addRow("外边距:", le_outer_dist);
	lyt_v_f2->addRow("内边距:", le_inner_dist);
	lyt_v_f2->addRow("中间距:", le_mid_dist);
	lyt_v_f2->addRow("开始测量:", btn_meas_exec);
	lyt_v_f2->addRow("继续采集:", btn_continue_grab);

	lyt_v->addWidget(argu_w);
	lyt_v->addWidget(res_w);
	
	//display data slot

	connect(&ProxyMeasMsg::instance(), &ProxyMeasMsg::measDataMsg, 
		[le_pix_len, le_outer_dist, le_inner_dist, le_mid_dist](MeasData data) {
			auto ratio = le_pix_len->text().toDouble();
			QString outers{ "| " }, inners{ "| " }, mids{ "| " };
			for (auto& [outer, inner, mid] : data)
			{
				outers += QString("%1 | ").arg(outer * ratio);
				inners += QString("%1 | ").arg(inner * ratio);
				mids += QString("%1 | ").arg(mid * ratio);
				//std::cout << outer << ", " << inner << ", " << mid << std::endl;
			}
			le_outer_dist->setText(outers);
			le_inner_dist->setText(inners);
			le_mid_dist->setText(mids);
		});

	connect(btn_meas_exec, &QPushButton::clicked, this, &MeasWidthOptWidget::measImageLaneWidth);

	connect(btn_continue_grab, &QPushButton::clicked, [] {
		try {
			auto camobj = CamWorker::instance().cam_vec.at(0);
			if (camobj) { camobj->startGrab(); }
		}
		catch (const std::exception& e) {
			std::cout << e.what() << std::endl;
		}
		});
}

MeasWidthOptWidget::~MeasWidthOptWidget()
{
}

void MeasWidthOptWidget::measImageLaneWidth()
{
	auto task = new MeasTask;
	task->meas = std::make_unique<CalLineWidth>(*meas);
	bool auto_del = task->autoDelete();
	QThreadPool::globalInstance()->start(task);
}

ProxyMeasMsg::ProxyMeasMsg()
{
	qRegisterMetaType<MeasData>("MeasData");
	qRegisterMetaType<MeasData>("MeasData&");
}

ProxyMeasMsg& ProxyMeasMsg::instance()
{
	static ProxyMeasMsg obj;
	return obj;
}

void MeasTask::run()
{
	auto& camobj = CamWorker::instance().cam_vec.front();
	if (!camobj->isOpen()) {
		spdlog::warn("camera isn't open!");
		return;
	}

	if (!camobj->getCam()->mCamera->IsGrabbing()) {
		spdlog::warn("camera isn't grabbing!");
		return;
	}	

	auto wrapper = camobj->getCameraCurrentImage();
	if (!wrapper.has_value()) {
		spdlog::warn("can't get camera current frame image!");
		return;
	}

	QImage qimg = wrapper.value();
	auto bgr_qimg = qimg.convertToFormat(QImage::Format_BGR888);//clone

	auto qimg_cvt_mat_wrap = [](QImage& qimg, int type) -> cv::Mat {/*此处不用引用会有异常风险*/
		cv::Mat wrap = { qimg.height(), qimg.width(), type, qimg.bits(), size_t(qimg.bytesPerLine()) };
		return wrap;
		};

	//local test imgae:
	//qimg.load("e:/img/line/line15.bmp");
	//bgr_qimg = qimg.convertToFormat(QImage::Format_BGR888);

	cv::Mat cv_img = qimg_cvt_mat_wrap(qimg, CV_8UC1).clone(); //clone
	cv::Mat cv_bgr = qimg_cvt_mat_wrap(bgr_qimg, CV_8UC3); //wrap to drawline

	CamWorker::instance().cam_vec.front()->stopGrab(); //cv_img clone finished ,camera can stop grab.
	
	try {
		meas->cal_line_width(cv_img);
		auto res = meas->cal_lines_group_width();
		meas->drawLineOnBGR(cv_bgr);

		emit ProxyMeasMsg::instance().measImgaeMsg(bgr_qimg);
		emit ProxyMeasMsg::instance().measDataMsg(res);
	}
	catch (const std::exception& e)
	{
		spdlog::warn(e.what());
	}
}


