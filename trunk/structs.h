// rgba_t: A 32-bit colour definition
struct rgba_t
{
	BYTE r, g, b, a;

	// Constructors
	rgba_t() { r = 0; g = 0; b = 0; a = 0; }
	
	rgba_t(BYTE R, BYTE G, BYTE B, BYTE A)
	{
		r = R;
		g = G;
		b = B;
		a = A;
	}

	// Functions
	void set(BYTE R, BYTE G, BYTE B, BYTE A)
	{
		r = R;
		g = G;
		b = B;
		a = A;
	}

	void set(rgba_t colour)
	{
		r = colour.r;
		g = colour.g;
		b = colour.b;
		a = colour.a;
	}

	float fr() { return (float)r / 255.0f; }
	float fg() { return (float)g / 255.0f; }
	float fb() { return (float)b / 255.0f; }
	float fa() { return (float)a / 255.0f; }
};

// point2_t: A 2d point
struct point2_t
{
	int x, y;

	// Constructors
	point2_t() { x = 0; y = 0; }

	point2_t(int X, int Y)
	{
		x = X;
		y = Y;
	}

	// Functions
	void set(int X, int Y)
	{
		x = X;
		y = Y;
	}

	void set(point2_t p)
	{
		x = p.x;
		y = p.y;
	}

	bool equals(point2_t p)
	{
		if (x == p.x && y == p.y)
			return true;
		else
			return false;
	}

	/*
	float magnitude()
	{
		return (float)sqrt((x * x) + (y * y));
	}
	*/

	point2_t perpendicular()
	{
		return point2_t(-y, x);
	}
};

// rect_t: A rectangle
struct rect_t
{
	point2_t tl, br;

	// Constructors
	rect_t() { tl.set(0, 0); br.set(0, 0); }

	rect_t(point2_t TL, point2_t BR)
	{
		tl.set(TL);
		br.set(BR);
	}

	rect_t(int x1, int y1, int x2, int y2)
	{
		tl.set(x1, y1);
		br.set(x2, y2);
	}

	rect_t(int x, int y, int width, int height, int align)
	{
		if (align == 0) // Top-left
		{
			tl.set(x, y);
			br.set(x + width, y + height);
		}

		if (align == 1) // Centered
		{
			tl.set(x - (width / 2), y - (height / 2));
			br.set(x + (width / 2), y + (height / 2));
		}
	}

	// Functions
	void set(point2_t TL, point2_t BR)
	{
		tl.set(TL);
		br.set(BR);
	}

	void set(int x1, int y1, int x2, int y2)
	{
		tl.set(x1, y1);
		br.set(x2, y2);
	}

	void set(rect_t rect)
	{
		tl.set(rect.tl);
		br.set(rect.br);
	}

	int x1() { return tl.x; }
	int y1() { return tl.y; }
	int x2() { return br.x; }
	int y2() { return br.y; }

	int left()		{ return min(tl.x, br.x); }
	int top()		{ return min(tl.y, br.y); }
	int right()		{ return max(br.x, tl.x); }
	int bottom()	{ return max(br.y, tl.y); }

	int width() { return br.x - tl.x; }
	int height() { return br.y - tl.y; }

	int awidth() { return max(br.x, tl.x) - min(tl.x, br.x); }
	int aheight() { return max(br.y, tl.y) - min(tl.y, br.y); }

	void resize(int x, int y)
	{
		if (x1() < x2())
		{
			tl.x -= x;
			br.x += x;
		}
		else
		{
			tl.x += x;
			br.x -= x;
		}

		if (y1() < y2())
		{
			tl.y -= y;
			br.y += y;
		}
		else
		{
			tl.y += y;
			br.y -= y;
		}
	}
};

#define RECT_TOPLEFT	0
#define RECT_CENTER		1

struct patch_header_t
{
	short	width;
	short	height;
	short	left;
	short	top;
};

struct col_header_t
{
	BYTE	row_offset;
	BYTE	n_pixels;
};

struct key_value_t
{
	string	name;
	int		value;

	key_value_t() { name = ""; value = 0; }
	key_value_t(string name, int value)
	{
		this->name = name;
		this->value = value;
	}
};

struct table_t
{
	key_value_t	**rows;
	int			n_rows;

	table_t() { rows = NULL; n_rows = 0; }

	void add(string name, int value)
	{
		n_rows++;
		rows = (key_value_t **)realloc(rows, n_rows * sizeof(key_value_t *));
		rows[n_rows - 1] = new key_value_t(name, value);
	}

	void set(string name, int value)
	{
		for (int k = 0; k < n_rows; k++)
		{
			if (rows[k]->name == name)
				rows[k]->value = value;
		}
	}

	void increment(string name, int value)
	{
		for (int k = 0; k < n_rows; k++)
		{
			if (rows[k]->name == name)
				rows[k]->value += value;
		}
	}

	int get(string name)
	{
		for (int k = 0; k < n_rows; k++)
		{
			if (rows[k]->name == name)
				return rows[k]->value;
		}

		return -1;
	}

	void sort_by_values(bool ascend)
	{
		for (int p = 1; p < n_rows; p++)
		{
			key_value_t* tmp = rows[p];
			int j;

			if (ascend)
			{
				for (j = p; j > 0 && tmp->value < rows[j - 1]->value; j--)
					rows[j] = rows[j - 1];
			}
			else
			{
				for (j = p; j > 0 && tmp->value > rows[j - 1]->value; j--)
					rows[j] = rows[j - 1];
			}

			rows[j] = tmp;
		}
	}
};


// Yeah I know I could easily replace these with std::vectors, and I have where I can,
// but the line drawing stuff I really don't want to go though again :P
struct pointlist_t
{
	point2_t	*points;
	DWORD		n_points;

	pointlist_t() { points = NULL; n_points = 0; }
	~pointlist_t() { if (points) free(points); }

	bool exists(point2_t point)
	{
		for (DWORD n = 0; n < n_points; n++)
		{
			if (points[n].x == point.x &&
				points[n].y == point.y)
				return true;
		}

		return false;
	}

	bool add(point2_t point, bool no_dup)
	{
		bool dup = exists(point);

		if (no_dup && dup)
			return false;

		n_points++;
		points = (point2_t *)realloc(points, n_points * sizeof(point2_t));
		points[n_points - 1].set(point);

		if (dup)
			return false;
		else
			return true;
	}

	int find_index(point2_t point)
	{
		for (DWORD n = 0; n < n_points; n++)
		{
			if (points[n].x == point.x &&
				points[n].y == point.y)
				return n;
		}

		return -1;
	}

	void remove(point2_t point)
	{
		int n = find_index(point);

		if (n == -1)
			return;

		for (DWORD i = n; i < n_points - 1; i++)
			points[i] = points[i + 1];

		n_points--;
		points = (point2_t *)realloc(points, n_points * sizeof(point2_t));
	}

	void clear()
	{
		n_points = 0;
		free(points);
		points = NULL;
	}

	point2_t get_last()
	{
		if (n_points > 0)
			return points[n_points - 1];
		else
			return point2_t(0, 0);
	}
};

struct numlist_t
{
	int		*numbers;
	DWORD	n_numbers;

	numlist_t() { numbers = NULL; n_numbers = 0; }
	~numlist_t() { if (numbers) free(numbers); }

	bool exists(int num)
	{
		for (DWORD n = 0; n < n_numbers; n++)
		{
			if (numbers[n] == num)
				return true;
		}

		return false;
	}

	bool add(int num, bool no_dup)
	{
		if (no_dup && exists(num))
			return false;

		n_numbers++;
		numbers = (int *)realloc(numbers, n_numbers * sizeof(int));
		numbers[n_numbers - 1] = num;

		return true;
	}

	void add_numlist(numlist_t *list, bool no_dup)
	{
		for (DWORD a = 0; a < list->n_numbers; a++)
			add(list->numbers[a], no_dup);
	}

	int find_index(int num)
	{
		for (DWORD n = 0; n < n_numbers; n++)
		{
			if (numbers[n] == num)
				return n;
		}

		return -1;
	}

	int find_index_last(int num)
	{
		for (int n = n_numbers - 1; n >= 0; n--)
		{
			if (numbers[n] == num)
				return n;
		}

		return -1;
	}

	void remove(int num)
	{
		int n = find_index_last(num);

		if (n == -1)
			return;

		for (DWORD i = n; i < n_numbers - 1; i++)
			numbers[i] = numbers[i + 1];

		n_numbers--;
		numbers = (int *)realloc(numbers, n_numbers * sizeof(int));
	}

	void remove_last()
	{
		n_numbers--;
		numbers = (int *)realloc(numbers, n_numbers * sizeof(int));
	}

	void clear()
	{
		n_numbers = 0;
		free(numbers);
		numbers = NULL;
	}

	int get_last()
	{
		if (n_numbers > 0)
			return numbers[n_numbers - 1];
		else
			return 0;
	}
};

#define SORT_ASCENDING 1
#define SORT_DESCENDING 2