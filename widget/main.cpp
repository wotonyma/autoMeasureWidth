
#include <iostream>
#include "qapplication.h"
#include "measure_widget.h"

#include "opencv_test.h"

int main(int argc, char* argv[])
{
	print_opencv_version();
	QApplication app(argc, argv);
	auto measure_wnd = new MeasureWidget;
	measure_wnd->show();
	auto rc = app.exec();
	return rc;
}