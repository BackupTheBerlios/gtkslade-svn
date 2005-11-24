
struct keybind_t
{
	string	name;
	string	key;
	string	defaultkey;
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
	void press(string name);
	void clear(string name);
	void clear_all();
	void change(string name, string key);
	void change_default(string name);

	string get_name(int index);
	string get_bind(int index);
	size_t n_keys() { return keys.size(); }

	void save(FILE* fp);
	void load(Tokenizer *tz);
};
