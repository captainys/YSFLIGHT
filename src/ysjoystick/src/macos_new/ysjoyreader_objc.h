#ifndef YSJOYREADER_MACOS_GAME_CONTROLLER_API_IF
#define YSJOYREADER_MACOS_GAME_CONTROLLER_API_IF

#ifdef __cplusplus
extern "C" {
#endif

#define YsJoyReader_MacOS_Caps_Name_Len 256

struct YsJoyReader_MacOS_Caps
{
    char name[YsJoyReader_MacOS_Caps_Name_Len];
    bool axis[4];
    bool upDownLeftRight[4];
    bool button[8];
};

struct YsJoyReader_MacOS_Readings
{
    float axis[4];
    bool upDownLeftRight[4];
    bool button[8];
};

void YsJoyReader_MacOS_InitializeController(void);
int YsJoyReader_MacOS_NumControllers(void);
void YsJoyReader_MacOS_GetController_Caps(struct YsJoyReader_MacOS_Caps &caps,int controllerID);
void YsJoyReader_MacOS_ReadController(YsJoyReader_MacOS_Readings &reading,int controllerID);

#ifdef __cplusplus
}
#endif

#endif