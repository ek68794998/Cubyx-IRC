#pragma warning (disable: 4100)

#include "profile.h"

bool INI::IsEntry(char sLine[256]) {
	if (sLine[0] == '[') { return false; }
	if (sLine[strlen(sLine)-2] == ']') { return false; }
	if (strcmp("=", sLine) == 0) { return false; }
	unsigned int icount = 0;
	unsigned int x = strlen(sLine);
	for (unsigned int i = 0; i < x; i++) {
		if (sLine[i] == '=') { icount++; }
	}
	if (icount != 1) { return false; }
	return true;
}
void INI::L_Trim(char *buf) {
	char *p1 = buf;
	char *p2 = buf;
	while (p2[0]) {
		if (p2[0] > 0x20) { break; }
		p2++;
	}
	while (p2[0]) {
		p1[0] = p2[0];
		p1++;
		p2++;
	}
	p1[0] = 0x0;
}

bool INI::GetEntry(char sLine[256], char *psKey, char *psValue) {
	if (!IsEntry(sLine)) { return false; }

	int iPos = -1;
	unsigned int x = strlen(sLine);
	for (unsigned int i = 0; i < x; i++) {
		if (sLine[i] == '=') {
			iPos = i;
			break;
		}
	}
	// remove leading spaces
	strcpy(psKey,sLine);
	psKey[iPos] = '\0';
	strcpy(psValue,sLine + iPos + 1);
	psValue[strlen(psValue) - 1] = '\0';
	return true;
}

void INI::WriteEntry(const char *psKey, const char *psValue, FILE *FP) {
	char newentry[256];
	strcpy(newentry,psKey);
	strcat(newentry,"=");
	strcat(newentry,psValue);
	strcat(newentry,"\n");
	fputs(newentry,FP);
}

bool INI::IsSection(char sLine[256]) {
	if (sLine[0] != '[') { return false; }
	if (sLine[strlen(sLine)-2] != ']') { return false; }
	return true;
}

void INI::WriteSection(const char *psName, FILE *FP) {
	char newsection[256];
	strcpy(newsection,"[");
	strcat(newsection,psName);
	strcat(newsection,"]\n");
	fputs(newsection,FP);
}

bool INI::GetSection(char sLine[256], char *psSection) {
	if (!IsSection(sLine)) { return false; }
	strcpy(psSection, sLine + 1);
	psSection[strlen(psSection) - 2] = '\0';
	return true;
}

void INI::StripQuotes(char *buf) {
	char *p;
	char *q;
	p = buf;
	while (*p && isspace(*p)) { p++; }
	if (!(*p == '\"' || *p == '\'')) { return; }
	q = p+strlen(p);
	while(*q != *p && q > p) q--;
	if (q == p) { return; }
	int len = int(q - p - 1);
	memmove(buf, p+1, len);
	buf[len] = 0;
}

int INI::GetString(const char *section, const char *entry, const char *defaultString, char *buffer, int bufLen) {
	// Open sourceFile
	FILE *FP;
	if ((FP = fopen(m_Filename.c_str(),"r")) == NULL) {
		strcpy(buffer,defaultString);
		return strlen(buffer);
	}
	char sLine[256];
	bool bFoundSection = false;
	bool bFoundEntry = false;
	char curSection[256];
	char curKey[256];
	char curValue[256];
	while (fgets(sLine,256,FP)) {
		if (GetSection(sLine,curSection)) {
			if (strcmp(curSection,section) == 0) { bFoundSection = true; }
			else { bFoundSection = false; }
		} else if (GetEntry(sLine,curKey,curValue)) {
			int x = strlen(curKey);
			for (int i = x-1; i >= 0; i--) {
				if (curKey[i] == ' ') curKey[i] = 0;
				else { break; }
			}
			if (bFoundSection && (strcmp(curKey,entry) == 0)) {
				bFoundEntry = true;
				break;
			}
		}
	}
	fclose(FP);
	if (bFoundEntry) { strcpy(buffer,curValue); }
	else { strcpy(buffer,defaultString); }
	L_Trim(buffer);
	INI::StripQuotes(buffer);
	if (sLine[0] == ' ') {
		char tmp[256];
		strcpy(tmp,sLine);
	}
	return strlen(buffer);
}
std::string INI::GetString(const char *section, const char *entry, const char *defaultString) {
	char buffer[512];
	GetString(section,entry,defaultString,buffer,sizeof(buffer)+1);
	return (const char *)buffer;
}
int INI::GetInt(const char *section, const char *entry, int defaultInt) {
	char buf[256];
	char iBuf[34]; // "34" is max space "itoa" required under 32 bit C++
	sprintf(iBuf,"%d",defaultInt);
	GetString(section,entry,iBuf,buf,256);
	return atoi(buf);
}
float INI::GetFloat(const char *section, const char *entry, float defaultInt) {
	char buf[256];
	char iBuf[68]; // "34" is max space "itoa" required under 32 bit C++
	sprintf(iBuf,"%f",defaultInt);
	GetString(section,entry,iBuf,buf,256);
	return (float)atof(buf);
}
int INI::WriteString(const char *section, const char *entry, const char *string) {
	char tmpFilename[256];
	// work better on network!
	strcpy(tmpFilename, tmpnam(0));
	// Open tmpFile
	FILE *FPtmp;
	if ((FPtmp = fopen(tmpFilename,"w")) == NULL) { return 0; }

	// Open sourceFile
	FILE *FPsource;
	if ((FPsource = fopen(m_Filename.c_str(),"r")) == NULL) {
		FPsource = fopen(m_Filename.c_str(),"w");
		if (FPsource == NULL) { return 0; }
		fclose(FPsource);
		FPsource = fopen(m_Filename.c_str(),"r");
		if (FPsource == NULL) {
			unlink(m_Filename.c_str());
			return 0;
		}
	}

	// Write Source to Tmp
	char sLine[256];
	char curSection[256];
	char curKey[256];
	char curValue[256];
	bool bDone = false;
	bool bInSection = false;
	while (fgets(sLine,256,FPsource)) {
		if (GetSection(sLine,curSection)) {
			if (strcmp(curSection,section) == 0) {
				bInSection = true;
			} else {
				if (bInSection) {
					bInSection = false;
					if (!bDone) {
						bDone = true;
						WriteEntry(entry,string,FPtmp);
					}
				}
			}
			WriteSection(curSection,FPtmp);
		} else if (GetEntry(sLine,curKey,curValue)) {
			if (bInSection && (strcmp(curKey,entry) == 0)) {
				bDone = true;
				WriteEntry(curKey,string,FPtmp);
			} else {
				WriteEntry(curKey,curValue,FPtmp);
			}
		}
	}
	if (!bDone)
	{
		bDone = true;
		if (!bInSection) { WriteSection(section,FPtmp); }
		WriteEntry(entry,string,FPtmp);
	}
	fclose(FPsource);
	fclose(FPtmp);
	// Copy Tmp to Source
	if (unlink(m_Filename.c_str()) == -1) {
		unlink(tmpFilename);
		return 0;
	}
	int result;
	result = rename(tmpFilename,m_Filename.c_str());
	if (result != 0) { perror("Error renaming file"); }
	// All successfully done
	return strlen(string);
}

int INI::DelString(const char *section, const char *entry, const char *string) {
	char tmpFilename[256];
	// work better on network!
	strcpy(tmpFilename,tmpnam(0));
	// Open tmpFile
	FILE *FPtmp;
	if ((FPtmp = fopen(tmpFilename,"w")) == NULL) { return 0; }

	// Open sourceFile
	FILE *FPsource;
	if ((FPsource = fopen(m_Filename.c_str(),"r")) == NULL) {
		FPsource = fopen(m_Filename.c_str(),"w");
		if (FPsource == NULL) { return 0; }
		fclose(FPsource);
		FPsource = fopen(m_Filename.c_str(),"r");
		if (FPsource == NULL) {
			unlink(m_Filename.c_str());
			return 0;
		}
	}

	// Write Source to Tmp
	char sLine[256];
	char curSection[256];
	char curKey[256];
	char curValue[256];
	bool bDone = false;
	bool bInSection = false;
	while (fgets(sLine,256,FPsource)) {
		if (GetSection(sLine,curSection)) {
			if (strcmp(curSection,section) == 0) {
				bInSection = true;
			} else {
				if (bInSection) {
					bInSection = false;
					if (!bDone) {
						bDone = true;
						WriteEntry(entry,string,FPtmp);
					}
				}
			}
			WriteSection(curSection,FPtmp);
		} else if (GetEntry(sLine,curKey,curValue)) {
			if (bInSection && (strcmp(curKey,entry) == 0)) {
				bDone = true;
				if (strcmp(curKey, entry) || strcmp(curValue, string)) {
					WriteEntry(curKey, string, FPtmp);
				}
			} else {
				if (strcmp(curKey, entry) || strcmp(curValue, string)) {
					WriteEntry(curKey, curValue, FPtmp);
				}
			}
		}
	}
	fclose(FPsource);
	fclose(FPtmp);
	// Copy Tmp to Source
	if (unlink(m_Filename.c_str()) == -1) {
		unlink(tmpFilename);
		return 0;
	}
	int result;
	result= rename(tmpFilename,m_Filename.c_str());
	if (result != 0) { perror("Error renaming file"); }
	// All successfully done
	return strlen(string);
}
