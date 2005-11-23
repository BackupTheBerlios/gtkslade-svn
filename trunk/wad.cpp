// << ------------------------------------ >>
// << SLADE - SlayeR's LeetAss Doom Editor >>
// << By Simon Judd, 2004-05               >>
// << ------------------------------------ >>
// << wad.cpp - Wad class functions        >>
// << ------------------------------------ >>

// INCLUDES ------------------------------ >>
#include "main.h"
#include "map.h"

// VARIABLES ----------------------------- >>
WadList	wads;

CVAR(Int, node_builder, 0, CVAR_SAVE)

// EXTERNAL VARIABLES -------------------- >>
extern string map_lumps[12];
extern Map map;

// Wad::open: Opens a wad file
// ------------------------ >>
bool Wad::open(string filename)
{
	close();
	path = filename;

	// Open the file
	FILE *fp = fopen(filename.c_str(), "rb");

	// Check if it opened correctly
	if (!fp)
	{
		//printf("Wad file \"%s\" cannot be found.\n", filename.c_str());
		return false;
	}

	// Read the header
	fread(&type, 1, 4, fp);			// Wad type
	fread(&num_lumps, 4, 1, fp);	// No. of lumps in wad
	fread(&dir_offset, 4, 1, fp);	// Offset to directory

	// Lock the wad if IWAD
	if (type[0] == 'I')
		locked = true;
	else
		locked = false;

	// Setup the directory
	//directory = (dir_entry_t *)calloc(num_lumps, sizeof(dir_entry_t));
	directory = (Lump **)calloc(num_lumps, sizeof(Lump*));

	// Read the directory
	fseek(fp, dir_offset, SEEK_SET);
	for (DWORD d = 0; d < num_lumps; d++)
	{
		char name[9] = "";
		DWORD offset = 0;
		DWORD size = 0;

		fread(&offset, 4, 1, fp);	// Offset
		fread(&size, 4, 1, fp);		// Size
		fread(name, 1, 8, fp);		// Name
		
		name[8] = '\0';
		//directory[d]->Rename(strupr(name));
		directory[d] = new Lump(offset, size, g_ascii_strup(name, -1)/*strupr(name)*/);

		if (directory[d]->Name() == "P_START" || directory[d]->Name() == "PP_START")
			patches[START] = d;

		if (directory[d]->Name() == "P_END" || directory[d]->Name() == "PP_END")
			patches[END] = d;

		if (directory[d]->Name() == "F_START" || directory[d]->Name() == "FF_START")
			flats[START] = d;

		if (directory[d]->Name() == "F_END" || directory[d]->Name() == "FF_END")
			flats[END] = d;

		if (directory[d]->Name() == "S_START" || directory[d]->Name() == "SS_START")
			sprites[START] = d;

		if (directory[d]->Name() == "S_END" || directory[d]->Name() == "SS_END")
			sprites[END] = d;

		if (directory[d]->Name() == "TX_START")
			tx[START] = d;

		if (directory[d]->Name() == "TX_END")
			tx[END] = d;
	}

	// Read wad data into memory (only if wad isn't locked)
	if (!locked)
	{
		for (DWORD d = 0; d < num_lumps; d++)
		{
			//directory[d].data = (BYTE *)malloc(directory[d].size);

			fseek(fp, directory[d]->Offset(), SEEK_SET);
			fread(directory[d]->Data(), directory[d]->Size(), 1, fp);
		}
	}

	// Find all available maps
	for (DWORD l = 0; l < num_lumps; l++)
	{
		if (directory[l]->Size() == 0)
		{
			string mapname = directory[l]->Name();
			bool done = false;
			bool existing_map_lumps[12] = { false, false, false, false,
											false, false, false, false,
											false, false, false, false };

			while (!done)
			{
				l++;

				if (l == num_lumps)
					break;

				string name = directory[l]->Name();
				done = true;

				for (int s = 0; s < 12; s++)
				{
					if (name.substr(0, map_lumps[s].length()) == map_lumps[s])
					{
						existing_map_lumps[s] = true;
						done = false;
					}
				}
			}
			
			l--;

			if (!memchr(existing_map_lumps, 0, 5))
				available_maps.push_back(mapname);
		}
	}

	sort(available_maps.begin(), available_maps.end());
	//available_maps.sort();

	fclose(fp);
	return true;
}

// Wad::dump_directory: Writes all directory entries to the console
// ------------------------------------------------------------- >>
void Wad::dump_directory()
{
	for (DWORD l = 0; l < num_lumps; l++)
		printf("%d: %s (%db at %d)\n", l, directory[l]->Name().c_str(), directory[l]->Size(), directory[l]->Offset());
}

// Wad::get_lump_index: Returns the index of the first lump with the specified name
//                      Returns -1 if no matching lump is found
// ----------------------------------------------------------------------------- >>
long Wad::get_lump_index(string name)
{
	for (DWORD l = 0; l < num_lumps; l++)
	{
		if (str_upper(directory[l]->Name()) == str_upper(name))
			return l;
		//else
		//	log_message("\"%s\" != \"%s\"\n", str_upper(directory[l]->Name()).c_str(), str_upper(name).c_str());
	}

	return -1;
}

// Same as above but searches from an offset
// -------------------------------------- >>
long Wad::get_lump_index(string name, DWORD offset)
{
	if (num_lumps == 0)
		return -1;

	for (DWORD l = offset; l < num_lumps; l++)
	{
		if (str_upper(directory[l]->Name()) == str_upper(name))
			return l;
	}

	return -1;
}

// Wad::get_lump: Returns a pointer to the first matching lump from offset
// -------------------------------------------------------------------- >>
Lump* Wad::get_lump(string name, DWORD offset)
{
	for (DWORD l = offset; l < num_lumps; l++)
	{
		if (str_upper(directory[l]->Name()) == str_upper(name))
			return directory[l];
	}

	return NULL;
}

// Wad::add_lump: Adds a new 0-sized lump before <index>
// -------------------------------------------------- >>
void Wad::add_lump(string name, long index)
{
	num_lumps++;
	directory = (Lump **)realloc(directory, num_lumps * sizeof(Lump *));

	if (index == -1)
		index = num_lumps;

	for (long l = num_lumps - 1; l > index; l--)
		directory[l] = directory[l - 1];

	directory[index] = new Lump(0, 0, name);
}

// Wad::replace_lump: Replaces lump data with new data
// ------------------------------------------------ >>
void Wad::replace_lump(string name, DWORD new_size, BYTE *data, DWORD offset)
{
	int index = get_lump_index(name, offset);

	if (index > -1)
	{
		//free(directory[index]->Data());
		directory[index]->Resize(new_size);
		memcpy(directory[index]->Data(), data, new_size);
	}
}

// Wad::delete_lump: Deletes a lump
// ----------------------------- >>
void Wad::delete_lump(string name, DWORD offset)
{
	long index = -1;

	for (DWORD l = offset; l < num_lumps; l++)
	{
		if (str_upper(directory[l]->Name()) == str_upper(name))
		{
			index = l;
			break;
		}
	}

	if (index == -1)
		return;

	delete directory[index];

	for (DWORD l = index; l < num_lumps - 1; l++)
		directory[l] = directory[l + 1];

	num_lumps--;
	directory = (Lump **)realloc(directory, num_lumps * sizeof(Lump *));
}

// Wad::lump_mem: Loads a lump's data into memory
// ------------------------------------------- >>
/*
void* Wad::lump_mem(DWORD index)
{
	FILE *fp = fopen(path.c_str(), "rb");
	void *lump = malloc(directory[index].size);

	fseek(fp, directory[index].offset, SEEK_SET);
	fread(lump, directory[index].size, 1, fp);

	fclose(fp);

	return lump;
}
*/

void copy_file(string input, string output)
{
	FILE *in = fopen(input.c_str(), "rb");
	if (in)
	{
		FILE *out = fopen(output.c_str(), "wb");
		while (!feof(in))
		{
			BYTE b = 0;
			fread(&b, 1, 1, in);
			fwrite(&b, 1, 1, out);
		}
		fclose(out);
		fclose(in);
	}
}

// Wad::save: Saves a wad file to disk (backs up the file, then overwrites it)
// ------------------------------------------------------------------------ >>
void Wad::save(bool nodes)
{
	if (locked)
	{
		message_box("Saving to the IWAD is disallowed. Use 'Save As' instead.", GTK_MESSAGE_INFO);
		return;
	}

	string bakfile = path + ".bak";
	printf("%s\n%s\n", path.c_str(), bakfile.c_str());

	// Remove old backup file
	remove(bakfile.c_str());

	// Copy current file contents to new backup file
	copy_file(path, bakfile);

	// Determine directory offset & individual lump offsets
	dir_offset = 12;

	for (DWORD l = 0; l < num_lumps; l++)
	{
		directory[l]->SetOffset(dir_offset);
		dir_offset += directory[l]->Size();
	}

	// Open wadfile for writing
	FILE *fp = fopen(path.c_str(), "wb");

	// Write the header
	fwrite(type, 1, 4, fp);
	fwrite(&num_lumps, 4, 1, fp);
	fwrite(&dir_offset, 4, 1, fp);

	// Write the lumps
	for (DWORD l = 0; l < num_lumps; l++)
		fwrite(directory[l]->Data(), directory[l]->Size(), 1, fp);

	// Write the directory
	for (DWORD l = 0; l < num_lumps; l++)
	{
		char name[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		long offset = directory[l]->Offset();
		long size = directory[l]->Size();

		for (int c = 0; c < directory[l]->Name().length(); c++)
			name[c] = directory[l]->Name()[c];

		fwrite(&offset, 4, 1, fp);
		fwrite(&size, 4, 1, fp);
		fwrite(name, 1, 8, fp);
	}

	fclose(fp);

	// Build nodes
	if (nodes)
	{
		char command[1024] = "";
		string tempname = path + ".temp";
		copy_file(path, tempname);

		if (node_builder == 0)
			sprintf(command, "zdbsp %s -m %s -o %s", tempname.c_str(), map.name.c_str(), path.c_str());
		else if (node_builder == 1)
			sprintf(command, "zennode %s %s -o %s", tempname.c_str(), map.name.c_str(), path.c_str());

		system(command);
		remove(tempname.c_str());
	}
}

void Wad::close()
{
	if (path == "")
		return;

	available_maps.clear();

	for (int a = 0; a < num_lumps; a++)
		delete directory[a];

	free(directory);
	//directory.clear();

	num_lumps = 0;
	path = "";
	locked = true;
}