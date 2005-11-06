
#include "main.h"
#include "struct_3d.h"
#include "mathstuff.h"

// difference: Finds the difference between 2 numbers
// ----------------------------------------------- >>
int difference(int v1, int v2)
{
	return max(v1, v2) - min(v1, v2);
}

// distance: Returns the distance between two points
// ---------------------------------------------- >>
double distance(double x1, double y1, double x2, double y2)
{
	double dist_x = x2 - x1;
	double dist_y = y2 - y1;

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
double distance_to_line(int x1, int y1, int x2, int y2, double vx, double vy)
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

// cross: Returns the cross product of 2 vectors
// ------------------------------------------ >>
point3_t cross(point3_t p1, point3_t p2)
{
	point3_t cross_product;	

	cross_product.x = ((p1.y * p2.z) - (p1.z * p2.y));
	cross_product.y = ((p1.z * p2.x) - (p1.x * p2.z));
	cross_product.z = ((p1.x * p2.y) - (p1.y * p2.x));

	return cross_product;
}

// dot_product: Returns the dot product of 2 vectors
// ---------------------------------------------- >>
float dot_product(point3_t v1, point3_t v2)
{
	return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

// angle_between_vectors: Returns the angle between 2 vectors
// ------------------------------------------------------- >>
float angle_between_vectors(float x1, float y1, float x2, float y2)
{
	float dot = (x1 * x2) + (y1 * y2);
	float len = sqrt(x1 * x1 + y1 * y1) * sqrt(x2 * x2 + y2 * y2);
	float cosin = dot / len;

	return acos(cosin) * 180.0f / 3.1415926535897932384f;
}

float angle_between_vectors_2(point3_t v1, point3_t v2)
{
	float dot = (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
	float len = sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z)
				* sqrt(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z);
	float cosin = dot / len;

	return acos(cosin) * 180.0f / 3.1415926535897932384f;
}

// get_2d_angle: Returns the angle between 2 2d vectors (0.0f-360.0f)
// --------------------------------------------------------------- >>
float get_2d_angle(point3_t axis, point3_t vector)
{
	point3_t p1(axis.x, axis.y, 0.0f);
	point3_t p2(vector.x, vector.y, 0.0f);

	float a = angle_between_vectors_2(p1.normalize(), p2.normalize());
	float side = p2.y * p1.x - p2.x * p1.y;

	float ret;

	if (side > 0.0f)
		ret = 360.0f - a;
	else
		ret = a;

	if (ret > 360.0f)
		ret = 360.0f;
	if (ret < 0.0f)
		ret = 0.0f;

	return ret;
}
