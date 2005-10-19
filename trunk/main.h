
// Disable some useless warnings
#pragma warning (disable: 4996)
#pragma warning (disable: 4244)
#pragma warning (disable: 4018)

// Main includes
#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <algorithm>

// Define some variable types
#define BYTE guint8
#define WORD guint16
#define DWORD guint32

// A macro to check if a value exists in a vector
#define vector_exists(vec, val) find(vec.begin(), vec.end(), val) != vec.end()

// A macro to add a value to a vector if the value doesn't already exist in the vector
#define vector_add_nodup(vec, val) if (!(vector_exists(vec, val))) vec.push_back(val)

// Define min and max
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif

// Some basic colours
#define COL_WHITE	rgba_t(255, 255, 255, 255)
#define COL_BLACK	rgba_t(0, 0, 0, 255)
#define COL_RED		rgba_t(255, 0, 0, 255)
#define COL_GREEN	rgba_t(0, 255, 0, 255)
#define COL_BLUE	rgba_t(0, 0, 255, 255)

// Editor stuff
#define MAJOR_UNIT 32

// Version string
#define __SLADEVERS	"v1.1 beta"

// Program includes
#include "structs.h"
#include "tokenizer.h"
#include "cvar.h"
#include "wad.h"
#include "mathstuff.h"

// Functions
string str_upper(string str);
void print(bool debug, char *message, ...);
void log_message(char *message, ...);
void message_box(string message, GtkMessageType type);
void wait_gtk_events();
string file_browser(string extension);
bool yesno_box(string message);
string entry_box(string prompt);
string parse_string(char *str, ...);
