/*
 * sb_proto_test.h
 */

#ifndef SB_PROTO_TEST_H_
#define SB_PROTO_TEST_H_

#include <string.h>

typedef void (TestCaseSetup) (void);
typedef void (TelegramCheck) (void);

typedef enum
{
  TEL_RX
, TEL_TX
, END
} TelegramType;

typedef struct
{
    TelegramType    type;
    char unsigned    length;
    char unsigned    response_count;
    TelegramCheck * check;
    char unsigned    bytes[23];
} Telegram;

typedef struct
{
    char            * name;
    char unsigned     status;
    TestCaseSetup * setup;
    Telegram        * telegram;
} Test_Case;


#endif /* SB_PROTO_TEST_H_ */
