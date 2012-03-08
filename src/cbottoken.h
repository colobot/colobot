// cbottoken.h

#ifndef _CBOTTOKEN_H_
#define	_CBOTTOKEN_H_



enum ObjectType;



// Procédures.

extern char* RetObjectName(ObjectType type);
extern char* RetObjectAlias(ObjectType type);
extern char* RetHelpFilename(ObjectType type);
extern char* RetHelpFilename(const char *token);
extern BOOL IsType(const char *token);
extern BOOL IsFunction(const char *token);
extern char* RetHelpText(const char *token);



#endif //_CBOTTOKEN_H_
