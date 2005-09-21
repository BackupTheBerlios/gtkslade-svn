
#include "main.h"

// difference: Finds the difference between 2 numbers
// ----------------------------------------------- >>
int difference(int v1, int v2)
{
	return max(v1, v2) - min(v1, v2);
}

// distance: Returns the distance between two points
// ---------------------------------------------- >>
double distance(int x1, int y1, int x2, int y2)
{
	double dist_x = (double)x2 - (double)x1;
	double dist_y = (double)y2 - (double)y1;

	return sqrt(dist_x * dist_x + dist_y * dist_y);
}

// midpoint: Returns the mid point of a line between two points
// --------------------------------------------------------- >>
point2_t midpoint(point2_t p1, point2_t p2)
{
	int mid_x = min(p1.x, p2.x) + (difference(p1.x, p2.x) / 2);
	int mid_y = min(p1.y, p2.y) + (difference(p1.y, p2.y) / 2);

	return point2_t(mid_x, mid_y);
}

// distance_to_line: Finds the shortest distance from a vertex to a line
// ------------------------------------------------------------------ >>
// (code borrowed & slightly modified from Doom Builder, by CodeImp)
double distance_to_line(int x1, int y1, int x2, int y2, int vx, int vy)
{
	double u;

	// Get length of linedef
	double ld = distance(x1, y1, x2, y2);

	// Check if not zero length
	if(ld)
	{
		// Get the intersection offset
		u = double((vx - x1) * (x2 - x1) + (vy - y1) * (y2 - y1)) / (ld * ld);

		// Limit intersection offset to the line
		double lbound = 1 / ld;
		double ubound = 1 - lbound;
		if(u < lbound) u = lbound;
		if(u > ubound) u = ubound;
	}
	else
	{
		u = 0;
	}

	// Calculate intersection point
	double ix = double(x1) + u * double(x2 - x1);
	double iy = double(y1) + u * double(y2 - y1);

	// Return distance between intersection and point
	// which is the shortest distance to the line
	return distance(double(vx), double(vy), ix, iy);
}

// point_in_rect: Determines if a point is within a rectangle
// ------------------------------------------------------- >>
bool point_in_rect(int x1, int y1, int x2, int y2, int x, int y)
{
	return (x >= x1) && (x <= x2) && (y >= y1) && (y <= y2);
}

// lines_intersect: Checks if 2 lines intersect eachother
// --------------------------------------------------- >>
bool lines_intersect(rect_t line1, rect_t line2)
{
	float r, s;

	r = float(line1.y1()-line2.y1())*float(line2.width())-float(line1.x1()-line2.x1())*float(line2.height()) /
		float(line1.width())*float(line2.height())-float(line1.height())*float(line2.width());

	s = float(line1.y1()-line2.y1())*float(line1.width())-float(line1.x1()-line2.x1())*float(line1.height()) / 
		float(line1.width())*float(line2.height())-float(line1.height())*float(line2.width());

	if (0.0f <= r <= 1.0f && 0.0f <= s <= 1.0f)
		return true;
	else
		return false;
}

rect_t h_difference(rect_t r1, rect_t r2)
{
	int width = difference(r1.width(), r2.width());
	int left;

	if (r1.right() != r2.right())
		left = max(r1.right(), r2.right()) - width;
	else
		left = min(r1.left(), r2.left());

	return rect_t(left, min(r1.top(), r2.top()), width, max(r1.height(), r2.height()), 0);
}

rect_t v_difference(rect_t r1, rect_t r2)
{
	int height = difference(r1.height(), r2.height());
	int top;

	if (r1.bottom() != r2.bottom())
		top = max(r1.bottom(), r2.bottom()) - height;
	else
		top = min(r1.top(), r2.top());

	return rect_t(min(r1.left(), r2.left()), top, max(r1.width(), r2.width()), height, 0);
}
