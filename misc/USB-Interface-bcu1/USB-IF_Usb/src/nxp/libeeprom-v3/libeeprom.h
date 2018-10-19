#ifndef API_H_
#define API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


/* Define IAP command codes */
enum {
    EELIB_IAP_COMMAND_EEPROM_WRITE = 61,
    EELIB_IAP_COMMAND_EEPROM_READ = 62,
};
    

/* Define IAP status codes */
enum {
    EELIB_IAP_STATUS_CMD_SUCCESS = 0,
    EELIB_IAP_STATUS_INVALID_COMMAND = 1,
    EELIB_IAP_STATUS_SRC_ADDR_ERROR = 2,
    EELIB_IAP_STATUS_DST_ADDR_ERROR = 3,
    EELIB_IAP_STATUS_SRC_ADDR_NOT_MAPPED = 4,
    EELIB_IAP_STATUS_DST_ADDR_NOT_MAPPED = 5,
    EELIB_IAP_STATUS_COUNT_ERROR = 6,
};
    
typedef uint32_t EELIB_Command[5];
typedef uint32_t EELIB_Result[5];

/* Entry point to libeeprom (replaces ROM IAP entry) */
void EELIB_entry (EELIB_Command command, EELIB_Result result);

/* Return version number */
uint32_t EELIB_getVersion (void);

/* Idle handler.
 * Libeeprom calls this function while waiting for the end of an
 * EEPROM operation.
 */
void EELIB_idleHandler (void);

#ifdef __cplusplus
}
#endif

#endif  /* ifndef API_H_ */
