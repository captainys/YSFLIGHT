#include <stdio.h>
#include <Cocoa/Cocoa.h>
#include <GameController/GameController.h>
#include "ysjoyreader_objc.h"



static bool newGameController=false;

void YsJoyReader_MacOS_InitializeController(void)
{
    // Ref https://stackoverflow.com/questions/55226373/how-do-i-use-apples-gamecontroller-framework-from-a-macos-command-line-tool
    NSNotificationCenter * center = [NSNotificationCenter defaultCenter];
    [center 
        addObserverForName:GCControllerDidConnectNotification 
        object:nil
        queue:nil
        usingBlock:^(NSNotification * note){newGameController=true;}
     ];
}

int YsJoyReader_MacOS_NumControllers(void)
{
    NSArray *ary=[GCController controllers];
    return (int)[ary count];
}

void YsJoyReader_MacOS_GetController_Caps(struct YsJoyReader_MacOS_Caps &caps,int controllerID)
{
    NSArray *ary=[GCController controllers];
    if(controllerID<[ary count])
    {
        GCController *controller=ary[controllerID];
        strncpy(caps.name,controller.vendorName.UTF8String,YsJoyReader_MacOS_Caps_Name_Len-1);
        caps.name[YsJoyReader_MacOS_Caps_Name_Len-1]=0;
        GCExtendedGamepad *profile=[ary[i] extendedGamepad];
        if(nil!=profile)
        {
            caps.axis[0]=true;
            caps.axis[1]=true;
            caps.axis[2]=true;
            caps.axis[3]=true;
            caps.upDownLeftRight[0]=true;
            caps.upDownLeftRight[1]=true;
            caps.upDownLeftRight[2]=true;
            caps.upDownLeftRight[3]=true;
            caps.button[0]=true;
            caps.button[1]=true;
            caps.button[2]=true;
            caps.button[3]=true;
            caps.button[4]=true;
            caps.button[5]=true;
            caps.button[6]=true;
            caps.button[7]=true;
        }
        else
        {
            GCMicroGamepad *profile=[ary[i] microGamepad];
            if(nil!=profile)
            {
                caps.axis[0]=false;
                caps.axis[1]=false;
                caps.axis[2]=false;
                caps.axis[3]=false;
                caps.upDownLeftRight[0]=true;
                caps.upDownLeftRight[1]=true;
                caps.upDownLeftRight[2]=true;
                caps.upDownLeftRight[3]=true;
                caps.button[0]=true;
                caps.button[1]=true;
                caps.button[2]=false;
                caps.button[3]=false;
                caps.button[4]=false;
                caps.button[5]=false;
                caps.button[6]=false;
                caps.button[7]=false;
            }
            else
            {
                caps.axis[0]=false;
                caps.axis[1]=false;
                caps.axis[2]=false;
                caps.axis[3]=false;
                caps.upDownLeftRight[0]=false;
                caps.upDownLeftRight[1]=false;
                caps.upDownLeftRight[2]=false;
                caps.upDownLeftRight[3]=false;
                caps.button[0]=false;
                caps.button[1]=false;
                caps.button[2]=false;
                caps.button[3]=false;
                caps.button[4]=false;
                caps.button[5]=false;
                caps.button[6]=false;
                caps.button[7]=false;
            }
        }

    }
}

void YsJoyReader_MacOS_ReadController(YsJoyReader_MacOS_Readings &reading,int controllerID)
{
    NSArray *ary=[GCController controllers];
    if(controllerID<[ary count])
    {
        GCController *controller=ary[controllerID];
        GCExtendedGamepad *profile=[controller extendedGamepad];
        if(nil!=profile)
        {
                reading.axis[0]=profile.leftThumbstick.xAxis.value;
                reading.axis[1]=profile.leftThumbstick.yAxis.value;
                reading.axis[2]=profile.rightThumbstick.xAxis.value;
                reading.axis[3]=profile.rightThumbstick.yAxis.value;

                reading.button[0]=profile.buttonA.pressed
                reading.button[1]=profile.buttonB.pressed
                reading.button[2]=profile.buttonX.pressed
                reading.button[3]=profile.buttonY.pressed
                reading.button[4]=profile.leftShoulder.pressed
                reading.button[5]=profile.rightShoulder.pressed
                reading.button[6]=profile.leftTrigger.pressed
                reading.button[7]=profile.rightTrigger.pressed

                reading.upDownLeftRight[]=profile.dpad.up.pressed;
                reading.upDownLeftRight[]=profile.dpad.down.pressed;
                reading.upDownLeftRight[]=profile.dpad.left.pressed;
                reading.upDownLeftRight[]=profile.dpad.right.pressed;
        }
        else
        {
            GCMicroGamepad *profile=[controller microGamepad];
            if(nil!=profile)
            {
                reading.axis[0]=0;
                reading.axis[1]=0;
                reading.axis[2]=0;
                reading.axis[3]=0;

                reading.button[0]=profile.buttonA.pressed;
                reading.button[1]=profile.buttonX.pressed;
                reading.button[2]=false;
                reading.button[3]=false;
                reading.button[4]=false;
                reading.button[5]=false;
                reading.button[6]=false;
                reading.button[7]=false;

                reading.upDownLeftRight[]=profile.dpad.up.pressed;
                reading.upDownLeftRight[]=profile.dpad.down.pressed;
                reading.upDownLeftRight[]=profile.dpad.left.pressed;
                reading.upDownLeftRight[]=profile.dpad.right.pressed;
            }
            else
            {
                reading.axis[0]=0;
                reading.axis[1]=0;
                reading.axis[2]=0;
                reading.axis[3]=0;

                reading.button[0]=false;
                reading.button[1]=false;
                reading.button[2]=false;
                reading.button[3]=false;
                reading.button[4]=false;
                reading.button[5]=false;
                reading.button[6]=false;
                reading.button[7]=false;

                reading.upDownLeftRight[]=false;
                reading.upDownLeftRight[]=false;
                reading.upDownLeftRight[]=false;
                reading.upDownLeftRight[]=false;
            }
        }
    }
}