
#ifndef ROL_JAL_BIM112_SRC_CHANNEL_H_
#define ROL_JAL_BIM112_SRC_CHANNEL_H_

#include <sblib/types.h>
#include <sblib/timeout.h>

#define NO_OF_CHANNELS 4
#define NO_OF_OUTPUTS  (NO_OF_CHANNELS * 2)
#define NO_OF_AUTOMATIC 4
#define NO_OF_SCENES    8
#define NO_OF_ALARMS    4

extern const int outputPins[NO_OF_OUTPUTS];
extern Timeout PWMDisabled;
#define PWM_TIMEOUT 50
#define PWM_PERIOD     857
#define PWM_DUTY_33    (588)

#define EE_CHANNEL_CFG_SIZE    72
#define EE_ALARM_HEADER_SIZE   10
#define EE_ALARM_CFG_SIZE       8

#define FEATURE_AUTOMATIC_A       0x0001
#define FEATURE_AUTOMATIC_B       0x0002
#define FEATURE_CENTRAL           0x0004
#define FEATURE_REFERENCE         0x0008
#define FEATURE_STATUS            0x0010
#define FEATURE_LIMIT             0x0020
#define FEATURE_ALARM             0x0040
#define FEATURE_BLOCK             0x0080
#define FEATURE_SCENE             0x0100
#define FEATURE_STORE_SCENE       0x0200
#define FEATURE_SHORT_OPERATION   0x0400
#define FEATURE_STATUS_MOVING     0x0800
#define FEATURE_RESTORE_AFTER_REF 0x1000

#define LOCK_POS_UP_DOWN         0x01
#define LOCK_POS_RELEASE_UP      0x02
#define LOCK_SCENE               0x08
#define LOCK_AUTOMATIC           0x10
#define LOCK_ABS_POSITION        0x20
#define LOCK_UP_DOWN             0x40
#define LOCK_HAND                0x80

typedef struct
{
    unsigned char priority;
    unsigned char monitorTime;
    unsigned char engageAction;
    unsigned char releaseAction;
    Timeout       monitor;
} AlarmConfig;

class Channel
{
public:
    typedef enum
    {
        IDLE       = 0x00
      , PROTECT    = 0x01
      // in the following states,the motor is already running
      , MOVE       = 0x80
      , SLAT_MOVE  = 0x81
      , EXTEND     = 0x82
      , DELAY      = 0x83
    } ChannelState;

    typedef enum
    {
        STOP, UP, DOWN
    } ChannelDirection;

    enum
    {
        IN_TOP_POSITION = 0x01, IN_BOT_POSITION = 0x02
    };
    enum { SHUTTER, BLIND};

    Channel(unsigned int number, unsigned int address);
    virtual unsigned int channelType(void);

    unsigned int isRunning(void);
    bool centralEnabled();
    bool automaticAEnabled();
    bool automaticBEnabled();
    unsigned short currentPosition(void);
    virtual void objectUpdate(unsigned int objno);
            void startUp(void);
            void startDown(void);
            void stop(void);
    virtual void periodic(void);
    virtual void moveTo(short position);
            void moveFor(unsigned int time, unsigned int direction);
            void handleAutomaticFunction(unsigned int pos, unsigned int block, unsigned int value);
            void handleMove(unsigned int value);
            void handleStep(unsigned int value);

protected:
            void _handleState(void);
    virtual void _savePosition(bool currentPosition);
    virtual bool _inSavedPosition(void);
    virtual void _startTracking(void);
    virtual bool _trackPosition(void);
    virtual bool _trackSlatPosition(void);
    virtual bool _restorePosition(void);
    virtual void _moveToAutomaticPosition(unsigned int pos);
    virtual bool _storeScene(unsigned int i);
    virtual void _moveToScene(unsigned int i);
    virtual bool _isInPosition(unsigned char pos);
    virtual bool _stillInAutoPosition(void);

    unsigned int timeToPercentage(unsigned int startTime, unsigned int maxTime);
    void _updatePosState(unsigned int current, unsigned int mask, unsigned int objno);
    void _enableFeature(unsigned int address, unsigned int feature, unsigned int mask = 0xFFFF);
    void handleScene(unsigned int value);

    // the following fields store the config for this channel
    unsigned short pauseChangeDir;                //!< min. time between changing the direction of the motor
    unsigned short motorOnDelay;                  //!< time after motor on and movement start of blinds
    unsigned short motorOffDelay;                 //!< time after motor off and movement stop of blinds
    unsigned short minMoveTime;                   //!< minimum motor activation time
    unsigned int   openTime;                      //!< time the channel needs from fully closed to fully open
    unsigned int   closeTime;                     //!< time the channel needs from fully open   to fully closed
    unsigned int   openTimeExt;                   //!< extension time for the opening direction
    unsigned int   closeTimeExt;                  //!< extension time for the closing direction
    unsigned char  automaticPos[NO_OF_AUTOMATIC]; //!< the 4 automatic positions
    unsigned char  automaticConfig;               //!< configuration of the automatic behavior
    unsigned char  sceneNumber[NO_OF_SCENES];     //!< the 4 automatic positions
    unsigned char  scenePos[NO_OF_SCENES];        //!< the 4 automatic positions
    unsigned short shortTime;                     //!< how long should the blind/shutter move as reaction to a short
    unsigned char  topLimitPos;                   //!< limit for the top position
    unsigned char  botLimitPos;                   //!< limit for the top position
    unsigned char  lockConfig;                    //!< defines what the generic lock object should lock
    unsigned char  reactionLock;                  //!< reaction when a lock is engaged
    unsigned char  obj24Config;      //!< configuration of the object 24
    unsigned char  oneBitPosition;   //!< position for the one bit operation
    AlarmConfig    alarm[NO_OF_ALARMS]; //!< configuration of the alarms
    unsigned char  busDownAction;
    unsigned char  busReturnAction;
    unsigned char  autoConfig;
    unsigned char  autoPosition;

    // the following fields are the current state of the channel
    unsigned char  number;           //!< need to calculate the object numbers
    unsigned char  firstObjNo;       //!< avoid multiple calculations
    bool           positionValid;    //!< the internal position is valid
    unsigned short features;         //!< bitmask of enabled features
    unsigned char  limits;           //!< indicate whether the limit positions have been reached
    ChannelState   state;            //!< current state of the channel
    ChannelDirection direction;        //!< in which direction should the channel move
    unsigned int   moveForTime;      //!< specify for how long the blind/shutter should be moved
    unsigned int   startTime;        //!< start time of a movement (needed for position calculation)
    // track position of shutter/blind
             short position;         //!< current channel position
             short startPosition;    //!< position when the movement started
             short targetPosition;   //!< requested target position
             short savedPosition;    //!< position before an automatic commands was triggered
    Timeout        timeout;
};

inline unsigned int Channel::isRunning(void)
{
    if (state & MOVE)
    {
        return direction;
    }
    return 0;
}

inline unsigned short Channel::currentPosition(void)
{
    return (unsigned short) position;
}

inline bool Channel::centralEnabled(void)
{
    return features & FEATURE_CENTRAL;
}

inline bool Channel::automaticAEnabled(void)
{
    return features & FEATURE_AUTOMATIC_A;
}

inline bool Channel::automaticBEnabled(void)
{
    return features & FEATURE_AUTOMATIC_B;
}

inline  unsigned int Channel::channelType(void)
{
    return SHUTTER;
}

inline bool Channel::_isInPosition(unsigned char pos)
{
    return position == pos;
}

inline bool Channel::_stillInAutoPosition(void)
{
    return position == autoPosition;
}

#endif /* ROL_JAL_BIM112_SRC_CHANNEL_H_ */
