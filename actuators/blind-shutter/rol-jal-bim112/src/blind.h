/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  Handle the blind (Raffstore) mode of an channel.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef ROL_JAL_BIM112_SRC_BLIND_H_
#define ROL_JAL_BIM112_SRC_BLIND_H_

#include <channel.h>

class Blind: public Channel
{
public:
    Blind() = delete;
    Blind(uint8_t newNumber, uint32_t newAddress, uint16_t newPosition, uint16_t newSlatPosition);
    virtual ~Blind() = default;
    virtual unsigned int channelType(void);
    //virtual void periodic(void);
    virtual void moveTo(short position);

    void moveSlatTo(short position);
    void moveSlatFor(unsigned int time, unsigned int direction);

    virtual unsigned short currentSlatPosition();

protected:
    virtual void _savePosition(bool currentPosition);
    virtual void _sendPosition();
    virtual bool _inSavedPosition(void);
    virtual void _startTracking(void);
    //virtual bool _trackPosition(void);
    virtual bool _trackSlatPosition(void);
    virtual bool _restorePosition(void);
    virtual void _moveToAutomaticPosition(unsigned int pos);
    virtual bool _storeScene(unsigned int i);
    virtual void _moveToScene(unsigned int i);
    virtual bool _isInPosition(unsigned char pos);
    virtual bool _stillInAutoPosition(void);
    virtual void _moveToOneBitPostion();

    unsigned int   slatTime;                          //!< time the slats need from fully closed to fully open
    unsigned int   slatPosAfterMove;                  //!< position the slats should take after a move command
    unsigned short slatAutoPosition;                  //!< slat position of the last automatic drive
    unsigned int   slatMoveForTime;                   //!< specify for how long the blind should be moved
    unsigned char  oneBitSlatPosition;                //!< slat position for the one bit operation

    unsigned char  automaticSlatPos[NO_OF_AUTOMATIC]; //!< the 4 automatic slat positions
    unsigned char  sceneSlatPos[NO_OF_SCENES];    //!< the slat positions for the scenes

    // track position of slats
    short slatPosition;         //!< current channel position
    short slatStartPosition;    //!< position when the movement started
    short slatTargetPosition;   //!< requested target position
    short slatSavedPosition;    //!< position before an automatic commands was triggered
};

inline unsigned int Blind::channelType(void)
{
    return BLIND;
}

inline bool Blind::_isInPosition(unsigned char pos)
{
    return (position == pos) && (slatPosition == pos);
}


inline bool Blind::_stillInAutoPosition(void)
{
    return (position == autoPosition) && (slatPosition == slatAutoPosition);
}

inline unsigned short Blind::currentSlatPosition(void)
{
    return (unsigned short) slatPosition;
}
#endif /* ROL_JAL_BIM112_SRC_BLIND_H_ */
