#ifndef DRAW_H_
#define DRAW_H_

const float default_scale = 1.4;

cv::Rect scaleRectWithLimits(const cv::Rect& source_rect, const cv::Point& tl_limit, const cv::Point& br_limit, float scale);

#endif
