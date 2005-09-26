
struct keybind_t
{
	string	name;
	string	key;
	bool	pressed;
};

class BindList
{
private:
	vector<keybind_t> keys;

public:
	BindList() { set_defaults(); }
	~BindList(){}

	void set_defaults();

	void add(string name, string key);
	void set(string key);
	void unset(string key);
	bool pressed(string name);
	void clear(string name);
	void clear_all();
};
