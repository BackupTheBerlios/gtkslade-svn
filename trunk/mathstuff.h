
#ifndef __MATHSTUFF_H__
#define __MATHSTUFF_H__

int difference(int v1, int v2);
double distance(double x1, double y1, double x2, double y2);
point2_t midpoint(point2_t p1, point2_t p2);
double distance_to_line(int x1, int y1, int x2, int y2, double vx, double vy);
bool point_in_rect(int x1, int y1, int x2, int y2, int x, int y);
rect_t h_difference(rect_t r1, rect_t r2);
rect_t v_difference(rect_t r1, rect_t r2);

#endif
