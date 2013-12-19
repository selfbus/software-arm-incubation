/*
 * sb_proto_test.h
 */

#ifndef SB_PROTO_TEST_H_
#define SB_PROTO_TEST_H_

#include <string.h>

typedef void (Test_Case_Setup) (void);
typedef void (Telegram_Check) (void);

typedef enum
{
  TEL_RX
, TEL_TX
, END
} Telegram_Type;

typedef struct
{
    Telegram_Type    type;
    char unsigned    length;
    char unsigned    response_count;
    Telegram_Check * check;
    char unsigned    bytes[23];
} Telegram;

typedef struct
{
    char            * name;
    char unsigned     status;
    Test_Case_Setup * setup;
    Telegram        * telegram;
} Test_Case;


#endif /* SB_PROTO_TEST_H_ */
