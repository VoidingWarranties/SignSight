#ifndef DRAW_H_
#define DRAW_H_

void drawBoundingRectangles(cv::Mat& image, const std::vector<std::vector<cv::Point> >& contours, const cv::Scalar& color, const float scale = 1.4);

#endif
