
struct namevalue_t
{
	string	name;
	short	value;
	bool	flag;

	namevalue_t() { flag = false; value = 0; }
};

struct argtype_t
{
	vector<namevalue_t> values;
	string				name;
	bool				has_flags;

	argtype_t();
	//argtype_t(string name);

	void add_value(string name, short value, bool flag)
	{
		namevalue_t newval;
		newval.name = name;
		newval.value = value;
		newval.flag = flag;
		values.push_back(newval);

		if (flag)
			this->has_flags = true;
	}

	string get_name(short value)
	{
		for (int v = 0; v < values.size(); v++)
		{
			if (has_flags)
			{
				if (values[v].value & value)
					return values[v].name;
			}
			else
			{
				if (values[v].value == value)
					return values[v].name;
			}
		}

		return "Unknown";
	}

	void parse(Tokenizer *tz);
};

argtype_t* get_arg_type(string name);
