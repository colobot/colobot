// cmdtoken.h

#ifndef _CMDTOKEN_H_
#define	_CMDTOKEN_H_



enum ObjectType;
enum WaterType;
enum PyroType;
enum CameraType;



// Procédures.

extern BOOL Cmd(char *line, char *token);
extern char* SearchOp(char *line, char *op);

extern int GetInt(char *line, int rank, int def);
extern float GetFloat(char *line, int rank, float def);
extern void GetString(char *line, int rank, char *buffer);
extern ObjectType GetTypeObject(char *line, int rank, ObjectType def);
extern WaterType GetTypeWater(char *line, int rank, WaterType def);
extern D3DTypeObj GetTypeTerrain(char *line, int rank, D3DTypeObj def);
extern int GetBuild(char *line, int rank);
extern int GetResearch(char *line, int rank);
extern PyroType GetPyro(char *line, int rank);
extern CameraType GetCamera(char *line, int rank);
extern char* GetCamera(CameraType type);

extern int OpInt(char *line, char *op, int def);
extern float OpFloat(char *line, char *op, float def);
extern void OpString(char *line, char *op, char *buffer);
extern ObjectType OpTypeObject(char *line, char *op, ObjectType def);
extern WaterType OpTypeWater(char *line, char *op, WaterType def);
extern D3DTypeObj OpTypeTerrain(char *line, char *op, D3DTypeObj def);
extern int OpResearch(char *line, char *op);
extern PyroType OpPyro(char *line, char *op);
extern CameraType OpCamera(char *line, char *op);
extern int OpBuild(char *line, char *op);
extern D3DVECTOR OpPos(char *line, char *op);
extern D3DVECTOR OpDir(char *line, char *op);
extern D3DCOLOR OpColor(char *line, char *op, D3DCOLOR def);
extern D3DCOLORVALUE OpColorValue(char *line, char *op, D3DCOLORVALUE def);



#endif //_CMDTOKEN_H_
