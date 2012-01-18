#pragma once

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <string>
#include <time.h>
#include <ctype.h>
#include <list>
#include <string>
#include <vector>
#include <map>
#include <set>
using namespace std;

class INI {
public:
	INI(const char *file) {
		m_Filename = file;
	}

	static void StripQuotes(char *buf);

	bool IsEntry(char sLine[256]);
	void L_Trim(char *buf);
	bool GetEntry(char sLine[256], char *psKey, char *psValue);
	void WriteEntry(const char *psKey, const char *psValue, FILE *FP);

	bool IsSection(char sLine[256]);
	void WriteSection(const char *psName, FILE *FP);
	bool GetSection(char sLine[256], char *psSection);

	int GetString(const char *section, const char *entry, const char *defaultString, char *buffer, int bufLen);
	std::string GetString(const char *section, const char *entry, const char *defaultString);
	int GetInt(const char *section, const char *entry, int defaultInt);
	float GetFloat(const char *section, const char *entry, float defaultInt);
	int WriteString(const char *section, const char *entry, const char *string);
	int DelString(const char *section, const char *entry, const char *string);
	const char *GetFile() { return m_Filename.c_str(); }

private:
	std::string m_Filename;
};