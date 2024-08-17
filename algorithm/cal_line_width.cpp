#include "cal_line_width.h"
//#include "find_cross_center.h"

#include <chrono>

void CalLineWidth_Test()
{
	cv::Mat ori = cv::imread("e:/img/line/line13.bmp", cv::IMREAD_GRAYSCALE);

	CalLineWidth cal;
	cal.cal_line_width(ori);
	auto res = cal.cal_lines_group_width();
	auto color_img = cal.drawLine(ori);
	cv::imwrite("e:/img/color_img.bmp", color_img);
	
}

//#include <Eigen/Dense>
//auto pointsDistFromLineByEigen(const std::vector<cv::Point>& edge, const cv::Vec4d& line)
//	-> std::tuple<double, double, double>
//{
//	Eigen::Matrix<double, Eigen::Dynamic, 3> edge_pts(edge.size(), 3); //Eigen::MatrixX3d edge_pts;
//
//	Eigen::Matrix<double, 3, Eigen::Dynamic> line_mat(3, 1);
//	//Eigen::VectorXd line_mat(3, 1);
//	auto l_pts = CalLineWidth::fitLinePoints(line, 0); //line two pts
//	//line_mat << A, B, C;
//
//	auto formulaABC = [](cv::Point pt1, cv::Point pt2) -> Eigen::Matrix<double, 3, 1> {
//		double x1 = pt1.x, y1 = pt1.y;
//		double x2 = pt2.x, y2 = pt2.y;
//		double A = (y1 - y2) / sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
//		double B = -(x1 - x2) / sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
//		double C = (x1 * y2 - x2 * y1) / sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
//		return { A, B, C };
//		};
//
//	line_mat.col(0) = formulaABC(l_pts.first, l_pts.second);
//
//	for (int j = 0; j < edge.size(); ++j) {
//		edge_pts(j, 0) = edge[j].x;
//		edge_pts(j, 1) = edge[j].y;
//		edge_pts(j, 2) = 1;
//	}
//	//std::cout << edge_pts << std::endl;
//	//std::cout << "***************************\n";
//	//std::cout << line_mat << std::endl;
//
//	auto m = edge_pts* line_mat;
//	
//	//auto dists = edge_pts.dot(line_mat);
//	
//	//std::cout << m << std::endl;
//	auto m1 = m.array().abs();
//	return { m1.col(0).minCoeff(), m1.col(0).maxCoeff(), m1.col(0).mean() };
//}

inline cv::Vec4d CalLineWidth::fit_line(const std::vector<cv::Point>& edge)
{
	cv::Vec4d line;
	cv::fitLine(edge, line, cv::DIST_L1, 0, 0.01, 0.01);
	return line;
}

/// <summary>
/// find another point of fileLine Vec4d line.
/// </summary>
/// <param name="line">output of cv::fitline</param>
/// <param name="x">specify another point.x</param>
/// <returns></returns>
inline auto CalLineWidth::fitLinePoints(const cv::Vec4d& line, double x) 
	-> std::pair<cv::Point2d, cv::Point2d>
{
	double cos_theta = line[0];
	double sin_theta = line[1];
	double x0 = line[2], y0 = line[3];

	if (0 == cos_theta)
		return { { x0, 0 }, {x0, y0} };

	double k = sin_theta / cos_theta;
	double b = y0 - k * x0;
	double y = k * x + b;
	return { { x, y }, {x0, y0} };
}

auto CalLineWidth::edgeDistFormFitLine(const std::vector<cv::Point>& edge, const cv::Vec4d& line) 
	-> std::tuple<double, double, double>
{
	double min = INT32_MAX;
	double max = INT32_MIN;
	double avg = 0;
	auto l_pts = fitLinePoints(line, 0); //line two pts

	for (auto pt : edge)
	{
		auto dist = pointDistFromLine(pt, l_pts);
		auto abs_dist = abs(dist);
		min = abs_dist < min ? abs_dist : min;
		max = abs_dist > max ? abs_dist : max;
		avg += dist;
	}
	return { min, max, abs(avg / edge.size()) };
	return std::tuple<double, double, double>();
}

inline double CalLineWidth::pointDistFromLine(cv::Point2d pt, const std::pair<cv::Point2d, cv::Point2d>& line)
{
	double x1 = line.first.x;
	double y1 = line.first.y;
	double x2 = line.second.x;
	double y2 = line.second.y;
	double x0 = pt.x;
	double y0 = pt.y;
	double distance;
	distance = ((y1 - y2) * x0 - (x1 - x2) * y0 + (x1 * y2 - x2 * y1)) / sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
	return distance;
}

cv::Mat CalLineWidth::preProcImage(cv::Mat ori)
{
	cv::Mat gaus, edges;
	cv::GaussianBlur(ori, gaus, { 3, 3 }, 1.5);
	cv::Canny(gaus, edges, 30, 60);

	if (edges_close_enable) {	//微小断续close
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, { 3, 3 });
		cv::morphologyEx(edges, edges, cv::MORPH_CLOSE, kernel); //轮廓开闭处理有误差
	}

	//mask image around
	{
		cv::Mat temp(ori.rows, ori.cols, CV_8UC1, cv::Scalar(0));
		const int len = 10;
		cv::Mat roi_c = temp(cv::Rect(len, len, ori.cols - 2 * len, ori.rows - 2 * len));
		cv::Mat roi_o = edges(cv::Rect(len, len, ori.cols - 2 * len, ori.rows - 2 * len));
		roi_o.copyTo(roi_c);
		edges = temp;
	}
	//cv::imwrite("e:/img/morph1.bmp", morph);

	if (detect_node_enable) { //处理线交叉节
		auto nodes = detectLineNode(ori);
		for (auto pt : nodes)
		{
			cv::drawMarker(edges, { int(pt.x), int(pt.y) }, { 0 }, cv::MARKER_STAR, node_mark_size, 2); //线宽2防止没画掉
		}
		//cv::imwrite("e:/img/morph2.bmp", edges);
	}

	return edges;
}

std::vector<cv::Point2f> CalLineWidth::detectLineNode(cv::Mat ori)
{
	int ratio = downsampled_ratio; //降采样为了寻找角点
	int max_c = max_node;  //最大寻找数量

	cv::Mat thumb;
	cv::resize(ori, thumb, { ori.cols / ratio, ori.rows / ratio }, cv::INTER_LANCZOS4);
	std::vector<cv::Point2f> corners;

	cv::goodFeaturesToTrack(thumb, corners, max_c, 0.1, 3);
	//cv::goodFeaturesToTrack(thumb, corners, 4, 0.1, 1, cv::noArray(), 3, true);

	if (subpixel_enable) {
		//指定亚像素计算迭代标注
		cv::TermCriteria criteria = cv::TermCriteria(cv::TermCriteria::MAX_ITER + cv::TermCriteria::EPS, 40, 0.01); 
		//亚像素检测角点
		cv::cornerSubPix(thumb, corners, cv::Size(5, 5), cv::Size(-1, -1), criteria);
	}

	/*cv::Mat colorImg;
	cv::cvtColor(thumb, colorImg, cv::COLOR_GRAY2BGR);
	for (auto pt : corners) {
		cv::circle(colorImg, pt, 3, { 0,0,255 }, -2);
	}
	cv::imwrite("e:/img/snap_color.bmp", colorImg);*/

	for (auto& pt : corners)
	{
		pt.x *= (double)ratio;
		pt.y *= (double)ratio;
	}

	//cv::Mat colorImg_ori;
	//cv::cvtColor(ori, colorImg_ori, cv::COLOR_GRAY2BGR);
	//for (auto pt : corners) {
	//	cv::drawMarker(colorImg_ori, { int(pt.x), int(pt.y) }, { 0,0,255 }, cv::MARKER_STAR, node_mark_size);
	//	//cv::drawMarker(colorImg_ori, { int(pt.x), int(pt.y) }, { 0,0,255 }, cv::MARKER_DIAMOND, node_mark_size);
	//}
	//cv::imwrite("e:/img/ori_color.bmp", colorImg_ori);

	return corners;
}

std::vector<CalLineWidth::FilterEdge> CalLineWidth::categoryEdges(cv::Mat edges)
{
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(edges, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

	for (int i = 0; i < contours.size(); ++i)
	{
		if (contours[i].size() > min_edge_len) //filter by edge length
		{
			filter_contours.emplace_back(contours[i]);
			filter_hierarchy.emplace_back(hierarchy[i]);
		}
	}

	if (filter_contours.empty())
		throw std::runtime_error("can't detect long edge");

	FilterEdge first;
	first.ids = { 0 };
	first.pts = filter_contours.front();
	first.line = fit_line(first.pts);

	std::vector<FilterEdge> edge_groups;
	edge_groups.emplace_back(std::move(first));

	auto st = std::chrono::steady_clock::now();

	for (int i = 1; i < filter_contours.size(); ++i)
	{
		auto& edge = filter_contours[i];
		bool isNewGroup = true;
		for (int j = 0; j < edge_groups.size(); ++j)
		{
			auto& line = edge_groups[j].line;
			//auto [min, max, avg] = pointsDistFromLineByEigen(edge, line);
			auto [min, max, avg] = edgeDistFormFitLine(edge, line);
			if (max < max_dist_thresh && avg < avg_dist_thresh) //如果大概在一条线上
			{
				//update the group ids,pts,fit_line
				isNewGroup = false;
				edge_groups[j].ids.emplace_back(i);	
				std::copy(edge.begin(), edge.end(), std::back_inserter(edge_groups[j].pts));
				edge_groups[j].line = fit_line(edge_groups[j].pts);
			}
		}
		if (isNewGroup) { //新的边缘
			FilterEdge new_group;
			new_group.ids = { i };
			new_group.pts = filter_contours[i];
			new_group.line = fit_line(new_group.pts);
			edge_groups.emplace_back(std::move(new_group));
		}
	}

	auto ed = std::chrono::steady_clock::now();
	std::cout << "measure speed: " << std::chrono::duration_cast<std::chrono::milliseconds>(ed - st).count() << " ms." << std::endl;
	return edge_groups;
}

std::vector<std::vector<CalLineWidth::FilterEdge>> CalLineWidth::groupFilterEdges(const std::vector<FilterEdge>& edges)
{
	std::vector<std::vector<CalLineWidth::FilterEdge>> group_edges;

	std::vector<CalLineWidth::FilterEdge> first = { edges.front() };
	group_edges.emplace_back(first);

	for (int i = 1; i < edges.size(); ++i)
	{
		if (edges[i].pts.size() < min_fit_pt_num)
			continue;

		bool isNew = true;
		double cos_theta = edges[i].line[0];
		double sin_theta = edges[i].line[1];
		for (auto& group : group_edges)
		{
			double g_cos_theta = group.front().line[0];
			double g_sin_theta = group.front().line[1];
			if (abs(g_cos_theta - cos_theta) < min_theta_diff && abs(g_sin_theta - sin_theta) < min_theta_diff)
			{
				group.emplace_back(edges[i]);
				isNew = false;
				break;
			}
		}
		if (isNew) {
			std::vector<CalLineWidth::FilterEdge> new_group = { edges[i] };
			group_edges.emplace_back(new_group);
		}
	}

	return group_edges;
}

void CalLineWidth::cal_line_width(cv::Mat ori)
{
	auto edge = preProcImage(ori);	//处理获取图像边缘

	auto filter_edge_groups = categoryEdges(edge);	//对边缘分类

	if (filter_edge_groups.size() < 2)
		throw std::runtime_error("can't detect more than 2 lines.");

	std::sort(filter_edge_groups.begin(), filter_edge_groups.end(), 
		[](FilterEdge& grp1, FilterEdge& grp2) {
			return grp1.pts.size() > grp2.pts.size();
		});

	line_groups = groupFilterEdges(filter_edge_groups);
	img_center = { ori.cols / 2.0, ori.rows / 2.0 };
}

auto CalLineWidth::cal_lines_group_width() -> std::vector<std::tuple<double, double, double>>
{
	auto sorted_parallel_dist = [](const std::vector<CalLineWidth::FilterEdge>& group, cv::Point2d pt) {
		std::vector<double> dists;
		for (auto& edge : group) {
			auto line_pts = fitLinePoints(edge.line, 0);
			auto dist = pointDistFromLine(pt, line_pts);
			dists.emplace_back(dist);
		}
		std::sort(begin(dists), end(dists), std::less<double>());
		return dists;
		};

	std::vector<std::tuple<double, double, double>> res;
	for (auto& group : line_groups)
	{
		auto dists = sorted_parallel_dist(group, img_center);
		if (group.size() == 4) {
			std::tuple<double, double, double> width = {
				dists[3] - dists[0],
				dists[2] - dists[1],
				(dists[3] + dists[2] - dists[1] - dists[0]) / 2.0
			};
			res.emplace_back(width);
		}
		else if (group.size() == 2) {
			std::tuple<double, double, double> width = {
				dists[1] - dists[0], dists[1] - dists[0], dists[1] - dists[0]
			};
			res.emplace_back(width);
		}
	}
	return res;
}

cv::Mat CalLineWidth::drawLine(cv::Mat ori)
{
	cv::Mat bgr_img;
	cv::cvtColor(ori, bgr_img, cv::COLOR_GRAY2BGR);

	//int max_parallel_line = 0; //平行线最大数量

	for (auto& group : line_groups)
	{
		if (group.size() != 4 && group.size() != 2)
			continue;
		for (auto& fl : group)
		{
			auto& line = fl.line;
			auto pt1 = fitLinePoints(line, 0).first;
			auto pt2 = fitLinePoints(line, ori.cols - 1).first;
			cv::line(bgr_img, pt1, pt2, { 0, 255, 0 });
		}
	}

	return bgr_img;
}

void CalLineWidth::drawLineOnBGR(cv::Mat& bgr)
{
	for (auto& group : line_groups)
	{
		if (group.size() != 4 && group.size() != 2)
			continue;
		for (auto& fl : group)
		{
			auto& line = fl.line;
			auto pt1 = fitLinePoints(line, 0).first;
			auto pt2 = fitLinePoints(line, bgr.cols - 1).first;
			cv::line(bgr, pt1, pt2, { 0, 255, 0 });
		}
	}
}
