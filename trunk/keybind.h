
struct keybind_t
{
	string	name;
	string	key;
	string	defaultkey;
	BYTE	mods;
	BYTE	defaultmods;

	bool	pressed;
};

#define KMOD_SHIFT	0x01
#define	KMOD_CTRL	0x02
#define KMOD_ALT	0x04

/*
struct keybind_t
{
	string	name;
	int		key;
	int		defaultkey;
	BYTE	mods;

	bool	pressed;
};
*/

class BindList
{
private:
	vector<keybind_t> keys;

public:
	BindList() { set_defaults(); }
	~BindList(){}

	void set_defaults();

	void add(string name, string key, BYTE mods = 0);
	void set(string key, GdkModifierType mods, vector<string> *list = NULL);
	void unset(string key, GdkModifierType mods, vector<string> *list = NULL);
	bool pressed(string name);
	void press(string name);
	void clear(string name);
	void clear_all();
	void change(string name, string key, BYTE mods);
	void change_default(string name);

	/*
	void add(string name, int key, BYTE mods);
	void set(int key, GdkModifierType mods);
	void unset(int key, GdkModifierType mods);
	bool pressed(string name);
	void press(string name);
	void clear(string name);
	void clear_all();
	void change(string name, int key, BYTE mods);
	void change_default(string name);
	*/

	//string get_name(int index);
	//string get_bind(int index);
	keybind_t* get_bind(int index);
	size_t n_keys() { return keys.size(); }

	void save(FILE* fp);
	void load(Tokenizer *tz);
};
