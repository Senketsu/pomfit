
#ifndef POMFIT_STRUCTS_H
	#define POMFIT_STRUCTS_H


struct UpProfile {

	char Name[256];
	char URL[128];
	char FormName[128];
	char FormType[128];
	char FormFileName[128];
	int MaxSize;
	bool bCookie;
	bool bNames;
	bool bFormName;
	bool bPrepend;
	char RegEx_URL[128];
	char RegEx_DEL[128];
	char PrepString[128];
};
typedef struct UpProfile UpProfile;

struct MuchSQLite {
		int iCol;
		int (*execute) (const char *format,...);
		int (*execute2) (const char *format,int argc,...);
		const char* (*fetchone_c) (char *column);

	};
typedef struct MuchSQLite MuchSQLite;
MuchSQLite SQLite;
#endif
