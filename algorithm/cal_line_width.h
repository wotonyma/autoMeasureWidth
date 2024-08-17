#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

void CalLineWidth_Test();

struct CalLineWidth {

	struct FilterEdge {
		std::vector<int> ids;		//some edge id
		std::vector<cv::Point> pts;	//some edge points
		cv::Vec4d line;				//fit_line of some edge pts;
	};

	static cv::Vec4d fit_line(const std::vector<cv::Point>& edge);
	
	static auto fitLinePoints(const cv::Vec4d& line, double x) -> std::pair<cv::Point2d, cv::Point2d>;

	static auto edgeDistFormFitLine(const std::vector<cv::Point>& edge, const cv::Vec4d& line)
		-> std::tuple<double, double, double>;

	static double pointDistFromLine(cv::Point2d pt, const std::pair<cv::Point2d, cv::Point2d>& line);

	cv::Mat preProcImage(cv::Mat ori);

	std::vector<cv::Point2f> detectLineNode(cv::Mat ori);

	std::vector<FilterEdge> categoryEdges(cv::Mat edges);
	
	std::vector<std::vector<FilterEdge>> groupFilterEdges(const std::vector<FilterEdge>& edges);

	void cal_line_width(cv::Mat ori);

	auto cal_lines_group_width() -> std::vector<std::tuple<double, double, double>>; //外距, 内距, 中间距

	cv::Mat drawLine(cv::Mat ori);
	void drawLineOnBGR(cv::Mat& bgr);

	bool edges_close_enable = false;	//是否将临近边缘连起来

	int min_edge_len = 80;			//边缘最小长度
	float max_dist_thresh = 15.0;		//分组时点距离边缘最大距离
	float avg_dist_thresh = 3.0;		//直线起伏程度

	bool detect_node_enable = true;	//是否寻找线节
	bool subpixel_enable = true;
	int downsampled_ratio = 8;
	int max_node = 16;

	int node_mark_size = 150;	//将结点划掉的markSize

	int min_fit_pt_num = 500;	//最终一条线最少点数
	float min_theta_diff = 0.005; //平行线最小角度差

private:
	std::vector<std::vector<cv::Point>> filter_contours;
	std::vector<cv::Vec4i> filter_hierarchy;
	std::vector<std::vector<FilterEdge>> line_groups;
	cv::Point2d img_center;
};


auto pointsDistFromLineByEigen(const std::vector<cv::Point>& edge, const cv::Vec4d& line)
	-> std::tuple<double, double, double>;