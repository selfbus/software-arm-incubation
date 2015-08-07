/*
 *  dimmer.h - 
 *
 *  Copyright (c) 2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef SCENE_H_
#define SCENE_H_

#include "channel.h"
#include "switch.h"
/*
 *
 */
class Scene: public _Switch_
{
public:
	Scene(unsigned int no, unsigned int longPress, unsigned int  channelConfig, unsigned int busReturn, unsigned int value);
    virtual void inputChanged(int value);
    virtual void checkPeriodic(void);
private:
    unsigned int saveScene;
    unsigned int numberScene;
    int sceneComObjNo;
};

#endif /* SCENE_H_ */
