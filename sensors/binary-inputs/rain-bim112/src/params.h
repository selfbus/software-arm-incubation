//
// Created by knx2header.py
//
// DO NOT EDIT THIS FILE. IT WILL BE OVERWRITTEN!
//
    
#ifndef INC_RAIN_PARAMS_H
#define INC_RAIN_PARAMS_H

#define userEeprom (*(bcu.userEeprom))

/**
	@brief ui_StartupDelay Bits: 16
		@ref UI_STARTUPDELAY_0_S (0): 0 s
		@ref UI_STARTUPDELAY_1_S (1): 1 s
		@ref UI_STARTUPDELAY_2_S (2): 2 s
		@ref UI_STARTUPDELAY_3_S (3): 3 s
		@ref UI_STARTUPDELAY_4_S (4): 4 s
		@ref UI_STARTUPDELAY_5_S (5): 5 s
		@ref UI_STARTUPDELAY_6_S (6): 6 s
		@ref UI_STARTUPDELAY_7_S (7): 7 s
		@ref UI_STARTUPDELAY_8_S (8): 8 s
		@ref UI_STARTUPDELAY_9_S (9): 9 s
		@ref UI_STARTUPDELAY_10_S (10): 10 s
		@ref UI_STARTUPDELAY_11_S (11): 11 s
		@ref UI_STARTUPDELAY_12_S (12): 12 s
		@ref UI_STARTUPDELAY_13_S (13): 13 s
		@ref UI_STARTUPDELAY_14_S (14): 14 s
		@ref UI_STARTUPDELAY_15_S (15): 15 s
		@ref UI_STARTUPDELAY_16_S (16): 16 s
		@ref UI_STARTUPDELAY_17_S (17): 17 s
		@ref UI_STARTUPDELAY_18_S (18): 18 s
		@ref UI_STARTUPDELAY_19_S (19): 19 s
		@ref UI_STARTUPDELAY_20_S (20): 20 s
		@ref UI_STARTUPDELAY_21_S (21): 21 s
		@ref UI_STARTUPDELAY_22_S (22): 22 s
		@ref UI_STARTUPDELAY_23_S (23): 23 s
		@ref UI_STARTUPDELAY_24_S (24): 24 s
		@ref UI_STARTUPDELAY_25_S (25): 25 s
		@ref UI_STARTUPDELAY_26_S (26): 26 s
		@ref UI_STARTUPDELAY_27_S (27): 27 s
		@ref UI_STARTUPDELAY_28_S (28): 28 s
		@ref UI_STARTUPDELAY_29_S (29): 29 s
		@ref UI_STARTUPDELAY_30_S (30): 30 s
		@ref UI_STARTUPDELAY_31_S (31): 31 s
		@ref UI_STARTUPDELAY_32_S (32): 32 s
		@ref UI_STARTUPDELAY_33_S (33): 33 s
		@ref UI_STARTUPDELAY_34_S (34): 34 s
		@ref UI_STARTUPDELAY_35_S (35): 35 s
		@ref UI_STARTUPDELAY_36_S (36): 36 s
		@ref UI_STARTUPDELAY_37_S (37): 37 s
		@ref UI_STARTUPDELAY_38_S (38): 38 s
		@ref UI_STARTUPDELAY_39_S (39): 39 s
		@ref UI_STARTUPDELAY_40_S (40): 40 s
		@ref UI_STARTUPDELAY_41_S (41): 41 s
		@ref UI_STARTUPDELAY_42_S (42): 42 s
		@ref UI_STARTUPDELAY_43_S (43): 43 s
		@ref UI_STARTUPDELAY_44_S (44): 44 s
		@ref UI_STARTUPDELAY_45_S (45): 45 s
		@ref UI_STARTUPDELAY_46_S (46): 46 s
		@ref UI_STARTUPDELAY_47_S (47): 47 s
		@ref UI_STARTUPDELAY_48_S (48): 48 s
		@ref UI_STARTUPDELAY_49_S (49): 49 s
		@ref UI_STARTUPDELAY_50_S (50): 50 s
		@ref UI_STARTUPDELAY_51_S (51): 51 s
		@ref UI_STARTUPDELAY_52_S (52): 52 s
		@ref UI_STARTUPDELAY_53_S (53): 53 s
		@ref UI_STARTUPDELAY_54_S (54): 54 s
		@ref UI_STARTUPDELAY_55_S (55): 55 s
		@ref UI_STARTUPDELAY_56_S (56): 56 s
		@ref UI_STARTUPDELAY_57_S (57): 57 s
		@ref UI_STARTUPDELAY_58_S (58): 58 s
		@ref UI_STARTUPDELAY_59_S (59): 59 s
		@ref UI_STARTUPDELAY_60_S (60): 60 s
*/
enum UI_STARTUPDELAY {
 	UI_STARTUPDELAY_0_S = 0
,	UI_STARTUPDELAY_1_S = 1
,	UI_STARTUPDELAY_2_S = 2
,	UI_STARTUPDELAY_3_S = 3
,	UI_STARTUPDELAY_4_S = 4
,	UI_STARTUPDELAY_5_S = 5
,	UI_STARTUPDELAY_6_S = 6
,	UI_STARTUPDELAY_7_S = 7
,	UI_STARTUPDELAY_8_S = 8
,	UI_STARTUPDELAY_9_S = 9
,	UI_STARTUPDELAY_10_S = 10
,	UI_STARTUPDELAY_11_S = 11
,	UI_STARTUPDELAY_12_S = 12
,	UI_STARTUPDELAY_13_S = 13
,	UI_STARTUPDELAY_14_S = 14
,	UI_STARTUPDELAY_15_S = 15
,	UI_STARTUPDELAY_16_S = 16
,	UI_STARTUPDELAY_17_S = 17
,	UI_STARTUPDELAY_18_S = 18
,	UI_STARTUPDELAY_19_S = 19
,	UI_STARTUPDELAY_20_S = 20
,	UI_STARTUPDELAY_21_S = 21
,	UI_STARTUPDELAY_22_S = 22
,	UI_STARTUPDELAY_23_S = 23
,	UI_STARTUPDELAY_24_S = 24
,	UI_STARTUPDELAY_25_S = 25
,	UI_STARTUPDELAY_26_S = 26
,	UI_STARTUPDELAY_27_S = 27
,	UI_STARTUPDELAY_28_S = 28
,	UI_STARTUPDELAY_29_S = 29
,	UI_STARTUPDELAY_30_S = 30
,	UI_STARTUPDELAY_31_S = 31
,	UI_STARTUPDELAY_32_S = 32
,	UI_STARTUPDELAY_33_S = 33
,	UI_STARTUPDELAY_34_S = 34
,	UI_STARTUPDELAY_35_S = 35
,	UI_STARTUPDELAY_36_S = 36
,	UI_STARTUPDELAY_37_S = 37
,	UI_STARTUPDELAY_38_S = 38
,	UI_STARTUPDELAY_39_S = 39
,	UI_STARTUPDELAY_40_S = 40
,	UI_STARTUPDELAY_41_S = 41
,	UI_STARTUPDELAY_42_S = 42
,	UI_STARTUPDELAY_43_S = 43
,	UI_STARTUPDELAY_44_S = 44
,	UI_STARTUPDELAY_45_S = 45
,	UI_STARTUPDELAY_46_S = 46
,	UI_STARTUPDELAY_47_S = 47
,	UI_STARTUPDELAY_48_S = 48
,	UI_STARTUPDELAY_49_S = 49
,	UI_STARTUPDELAY_50_S = 50
,	UI_STARTUPDELAY_51_S = 51
,	UI_STARTUPDELAY_52_S = 52
,	UI_STARTUPDELAY_53_S = 53
,	UI_STARTUPDELAY_54_S = 54
,	UI_STARTUPDELAY_55_S = 55
,	UI_STARTUPDELAY_56_S = 56
,	UI_STARTUPDELAY_57_S = 57
,	UI_STARTUPDELAY_58_S = 58
,	UI_STARTUPDELAY_59_S = 59
,	UI_STARTUPDELAY_60_S = 60
};

/**
	@brief en_inBetrieb Bits: 8
		@ref EN_INBETRIEB_NOT_ACTIVE (0): not active
		@ref EN_INBETRIEB_10_MIN (40): 10 min
		@ref EN_INBETRIEB_30_MIN (120): 30 min
		@ref EN_INBETRIEB_1_H (1): 1 h
		@ref EN_INBETRIEB_3_H (3): 3 h
		@ref EN_INBETRIEB_6_H (6): 6 h
		@ref EN_INBETRIEB_12_H (12): 12 h
		@ref EN_INBETRIEB_24_H (24): 24 h
*/
enum EN_INBETRIEB {
 	EN_INBETRIEB_NOT_ACTIVE = 0
,	EN_INBETRIEB_10_MIN = 40
,	EN_INBETRIEB_30_MIN = 120
,	EN_INBETRIEB_1_H = 1
,	EN_INBETRIEB_3_H = 3
,	EN_INBETRIEB_6_H = 6
,	EN_INBETRIEB_12_H = 12
,	EN_INBETRIEB_24_H = 24
};

/**
	@brief en_SendRS Bits: 8
		@ref EN_SENDRS_ONLY_REQUEST (1): only request
		@ref EN_SENDRS_AT_CHANGES (2): at changes
		@ref EN_SENDRS_CYCLIC (4): cyclic
		@ref EN_SENDRS_AT_CHANGES_AND_CYCLIC (6): at changes and cyclic
*/
enum EN_SENDRS {
 	EN_SENDRS_ONLY_REQUEST = 1
,	EN_SENDRS_AT_CHANGES = 2
,	EN_SENDRS_CYCLIC = 4
,	EN_SENDRS_AT_CHANGES_AND_CYCLIC = 6
};

/**
	@brief en_TimeRS Bits: 8
		@ref EN_TIMERS_10_SEC (80): 10 sec
		@ref EN_TIMERS_30_SEC (240): 30 sec
		@ref EN_TIMERS_1_MIN (1): 1 min
		@ref EN_TIMERS_5_MIN (5): 5 min
		@ref EN_TIMERS_30_MIN (30): 30 min
		@ref EN_TIMERS_60_MIN (60): 60 min
*/
enum EN_TIMERS {
 	EN_TIMERS_10_SEC = 80
,	EN_TIMERS_30_SEC = 240
,	EN_TIMERS_1_MIN = 1
,	EN_TIMERS_5_MIN = 5
,	EN_TIMERS_30_MIN = 30
,	EN_TIMERS_60_MIN = 60
};

/**
	@brief en_setupRM Bits: 8
		@ref EN_SETUPRM_LOW (100): low
		@ref EN_SETUPRM_HIGH (50): high
		@ref EN_SETUPRM_VERY_HIGH (25): very high
*/
enum EN_SETUPRM {
 	EN_SETUPRM_LOW = 100
,	EN_SETUPRM_HIGH = 50
,	EN_SETUPRM_VERY_HIGH = 25
};

/**
	@brief en_InfoHeizung Bits: 8
		@ref EN_INFOHEIZUNG_NO_SENDING_POSSIBLE_REQUEST (0): no sending, possible request
		@ref EN_INFOHEIZUNG_SEND_AT_CHANGES (1): send at changes
*/
enum EN_INFOHEIZUNG {
 	EN_INFOHEIZUNG_NO_SENDING_POSSIBLE_REQUEST = 0
,	EN_INFOHEIZUNG_SEND_AT_CHANGES = 1
};

/**
	@brief NoVisibale Bits: 8
		@ref NOVISIBALE_NO_VISIBLE (0): no visible
		@ref NOVISIBALE_VISIBLE (1): visible
*/
enum NOVISIBALE {
 	NOVISIBALE_NO_VISIBLE = 0
,	NOVISIBALE_VISIBLE = 1
};

/**
	@brief en_OnDelay Bits: 8
		@ref EN_ONDELAY_1_SEC (1): 1 sec
		@ref EN_ONDELAY_2_SEC (2): 2 sec
		@ref EN_ONDELAY_3_SEC (3): 3 sec
		@ref EN_ONDELAY_4_SEC (4): 4 sec
		@ref EN_ONDELAY_5_SEC (5): 5 sec
		@ref EN_ONDELAY_10_SEC (10): 10 sec
		@ref EN_ONDELAY_15_SEC (15): 15 sec
		@ref EN_ONDELAY_20_SEC (20): 20 sec
		@ref EN_ONDELAY_30_SEC (30): 30 sec
		@ref EN_ONDELAY_40_SEC (40): 40 sec
		@ref EN_ONDELAY_50_SEC (50): 50 sec
		@ref EN_ONDELAY_60_SEC (60): 60 sec
*/
enum EN_ONDELAY {
 	EN_ONDELAY_1_SEC = 1
,	EN_ONDELAY_2_SEC = 2
,	EN_ONDELAY_3_SEC = 3
,	EN_ONDELAY_4_SEC = 4
,	EN_ONDELAY_5_SEC = 5
,	EN_ONDELAY_10_SEC = 10
,	EN_ONDELAY_15_SEC = 15
,	EN_ONDELAY_20_SEC = 20
,	EN_ONDELAY_30_SEC = 30
,	EN_ONDELAY_40_SEC = 40
,	EN_ONDELAY_50_SEC = 50
,	EN_ONDELAY_60_SEC = 60
};

/**
	@brief en_OffDelay Bits: 8
		@ref EN_OFFDELAY_10_SEC (1): 10 sec
		@ref EN_OFFDELAY_30_SEC (3): 30 sec
		@ref EN_OFFDELAY_1_MIN (6): 1 min
		@ref EN_OFFDELAY_2_MIN (12): 2 min
		@ref EN_OFFDELAY_3_MIN (18): 3 min
		@ref EN_OFFDELAY_5_MIN (30): 5 min
		@ref EN_OFFDELAY_10_MIN (60): 10 min
		@ref EN_OFFDELAY_15_MIN (90): 15 min
		@ref EN_OFFDELAY_20_MIN (120): 20 min
*/
enum EN_OFFDELAY {
 	EN_OFFDELAY_10_SEC = 1
,	EN_OFFDELAY_30_SEC = 3
,	EN_OFFDELAY_1_MIN = 6
,	EN_OFFDELAY_2_MIN = 12
,	EN_OFFDELAY_3_MIN = 18
,	EN_OFFDELAY_5_MIN = 30
,	EN_OFFDELAY_10_MIN = 60
,	EN_OFFDELAY_15_MIN = 90
,	EN_OFFDELAY_20_MIN = 120
};

/**
 * Com Object Enums
 */
enum COM {
	/**
	@li "Rain"
	  Rain - Message Rain Yes/No
	  ObjectSize: 1 Bit
	*/
	COM_RAIN = 0
	/**
	@li "Operating"
	  Operating - Info
	  ObjectSize: 1 Bit
	*/
,	COM_OPERATING = 1
	/**
	@li "Heating"
	  Heating - Message Heating On/Off
	  ObjectSize: 1 Bit
	*/
,	COM_HEATING = 2
	/**
	@li "Rain amount", "NoVisibale"
	  Rain amount - Production testing
	  ObjectSize: 1 Byte
	*/
,	COM_RAIN_AMOUNT_NOVISIBALE = 3
	/**
	@li "Temperature value", "NoVisibale"
	  Temperature value - Production testing
	  ObjectSize: 2 Bytes
	*/
,	COM_TEMPERATURE_VALUE_NOVISIBALE = 4
};

/**
		@brief Address: 4444
		@li "Startup timeout"
		@b Default: 1
			@brief ui_StartupDelay Bits: 16
				@ref UI_STARTUPDELAY_0_S (0): 0 s
				@ref UI_STARTUPDELAY_1_S (1): 1 s
				@ref UI_STARTUPDELAY_2_S (2): 2 s
				@ref UI_STARTUPDELAY_3_S (3): 3 s
				@ref UI_STARTUPDELAY_4_S (4): 4 s
				@ref UI_STARTUPDELAY_5_S (5): 5 s
				@ref UI_STARTUPDELAY_6_S (6): 6 s
				@ref UI_STARTUPDELAY_7_S (7): 7 s
				@ref UI_STARTUPDELAY_8_S (8): 8 s
				@ref UI_STARTUPDELAY_9_S (9): 9 s
				@ref UI_STARTUPDELAY_10_S (10): 10 s
				@ref UI_STARTUPDELAY_11_S (11): 11 s
				@ref UI_STARTUPDELAY_12_S (12): 12 s
				@ref UI_STARTUPDELAY_13_S (13): 13 s
				@ref UI_STARTUPDELAY_14_S (14): 14 s
				@ref UI_STARTUPDELAY_15_S (15): 15 s
				@ref UI_STARTUPDELAY_16_S (16): 16 s
				@ref UI_STARTUPDELAY_17_S (17): 17 s
				@ref UI_STARTUPDELAY_18_S (18): 18 s
				@ref UI_STARTUPDELAY_19_S (19): 19 s
				@ref UI_STARTUPDELAY_20_S (20): 20 s
				@ref UI_STARTUPDELAY_21_S (21): 21 s
				@ref UI_STARTUPDELAY_22_S (22): 22 s
				@ref UI_STARTUPDELAY_23_S (23): 23 s
				@ref UI_STARTUPDELAY_24_S (24): 24 s
				@ref UI_STARTUPDELAY_25_S (25): 25 s
				@ref UI_STARTUPDELAY_26_S (26): 26 s
				@ref UI_STARTUPDELAY_27_S (27): 27 s
				@ref UI_STARTUPDELAY_28_S (28): 28 s
				@ref UI_STARTUPDELAY_29_S (29): 29 s
				@ref UI_STARTUPDELAY_30_S (30): 30 s
				@ref UI_STARTUPDELAY_31_S (31): 31 s
				@ref UI_STARTUPDELAY_32_S (32): 32 s
				@ref UI_STARTUPDELAY_33_S (33): 33 s
				@ref UI_STARTUPDELAY_34_S (34): 34 s
				@ref UI_STARTUPDELAY_35_S (35): 35 s
				@ref UI_STARTUPDELAY_36_S (36): 36 s
				@ref UI_STARTUPDELAY_37_S (37): 37 s
				@ref UI_STARTUPDELAY_38_S (38): 38 s
				@ref UI_STARTUPDELAY_39_S (39): 39 s
				@ref UI_STARTUPDELAY_40_S (40): 40 s
				@ref UI_STARTUPDELAY_41_S (41): 41 s
				@ref UI_STARTUPDELAY_42_S (42): 42 s
				@ref UI_STARTUPDELAY_43_S (43): 43 s
				@ref UI_STARTUPDELAY_44_S (44): 44 s
				@ref UI_STARTUPDELAY_45_S (45): 45 s
				@ref UI_STARTUPDELAY_46_S (46): 46 s
				@ref UI_STARTUPDELAY_47_S (47): 47 s
				@ref UI_STARTUPDELAY_48_S (48): 48 s
				@ref UI_STARTUPDELAY_49_S (49): 49 s
				@ref UI_STARTUPDELAY_50_S (50): 50 s
				@ref UI_STARTUPDELAY_51_S (51): 51 s
				@ref UI_STARTUPDELAY_52_S (52): 52 s
				@ref UI_STARTUPDELAY_53_S (53): 53 s
				@ref UI_STARTUPDELAY_54_S (54): 54 s
				@ref UI_STARTUPDELAY_55_S (55): 55 s
				@ref UI_STARTUPDELAY_56_S (56): 56 s
				@ref UI_STARTUPDELAY_57_S (57): 57 s
				@ref UI_STARTUPDELAY_58_S (58): 58 s
				@ref UI_STARTUPDELAY_59_S (59): 59 s
				@ref UI_STARTUPDELAY_60_S (60): 60 s
*/
#define EE_STARTUP_TIMEOUT (((unsigned)userEeprom[0x4444] << 8u) \
		|  userEeprom[0x4445])

/**
		@brief Address: 4446
		@li "cyclic send "Operating" telegram"
		@b Default: 0
			@brief en_inBetrieb Bits: 8
				@ref EN_INBETRIEB_NOT_ACTIVE (0): not active
				@ref EN_INBETRIEB_10_MIN (40): 10 min
				@ref EN_INBETRIEB_30_MIN (120): 30 min
				@ref EN_INBETRIEB_1_H (1): 1 h
				@ref EN_INBETRIEB_3_H (3): 3 h
				@ref EN_INBETRIEB_6_H (6): 6 h
				@ref EN_INBETRIEB_12_H (12): 12 h
				@ref EN_INBETRIEB_24_H (24): 24 h
*/
#define EE_CYCLIC_SEND_OPERATING_TELEGRAM (userEeprom[0x4446])

/**
		@brief Address: 4447
		@li "Send object rain"
		@b Default: 2
			@brief en_SendRS Bits: 8
				@ref EN_SENDRS_ONLY_REQUEST (1): only request
				@ref EN_SENDRS_AT_CHANGES (2): at changes
				@ref EN_SENDRS_CYCLIC (4): cyclic
				@ref EN_SENDRS_AT_CHANGES_AND_CYCLIC (6): at changes and cyclic
*/
#define EE_SEND_OBJECT_RAIN (userEeprom[0x4447])

/**
		@brief Address: 4448
		@li "Time for cyclic sending", "Send object rain"
		@b Default: 240
			@brief en_TimeRS Bits: 8
				@ref EN_TIMERS_10_SEC (80): 10 sec
				@ref EN_TIMERS_30_SEC (240): 30 sec
				@ref EN_TIMERS_1_MIN (1): 1 min
				@ref EN_TIMERS_5_MIN (5): 5 min
				@ref EN_TIMERS_30_MIN (30): 30 min
				@ref EN_TIMERS_60_MIN (60): 60 min
*/
#define EE_TIME_FOR_CYCLIC_SENDING_SEND_OBJECT_RAIN (userEeprom[0x4448])

/**
		@brief Address: 4449
		@li "Sensitivity of rain sensor"
		@b Default: 50
			@brief en_setupRM Bits: 8
				@ref EN_SETUPRM_LOW (100): low
				@ref EN_SETUPRM_HIGH (50): high
				@ref EN_SETUPRM_VERY_HIGH (25): very high
*/
#define EE_SENSITIVITY_OF_RAIN_SENSOR (userEeprom[0x4449])

/**
		@brief Address: 444a
		@li "Info object for heating is active"
		@b Default: 0
			@brief en_InfoHeizung Bits: 8
				@ref EN_INFOHEIZUNG_NO_SENDING_POSSIBLE_REQUEST (0): no sending, possible request
				@ref EN_INFOHEIZUNG_SEND_AT_CHANGES (1): send at changes
*/
#define EE_INFO_OBJECT_FOR_HEATING_IS_ACTIVE (userEeprom[0x444a])

/**
		@brief Address: 444b
		@li "Delay for message rain ON"
		@b Default: 20
			@brief en_OnDelay Bits: 8
				@ref EN_ONDELAY_1_SEC (1): 1 sec
				@ref EN_ONDELAY_2_SEC (2): 2 sec
				@ref EN_ONDELAY_3_SEC (3): 3 sec
				@ref EN_ONDELAY_4_SEC (4): 4 sec
				@ref EN_ONDELAY_5_SEC (5): 5 sec
				@ref EN_ONDELAY_10_SEC (10): 10 sec
				@ref EN_ONDELAY_15_SEC (15): 15 sec
				@ref EN_ONDELAY_20_SEC (20): 20 sec
				@ref EN_ONDELAY_30_SEC (30): 30 sec
				@ref EN_ONDELAY_40_SEC (40): 40 sec
				@ref EN_ONDELAY_50_SEC (50): 50 sec
				@ref EN_ONDELAY_60_SEC (60): 60 sec
*/
#define EE_DELAY_FOR_MESSAGE_RAIN_ON (userEeprom[0x444b])

/**
		@brief Address: 444c
		@li "Delay for message rain OFF"
		@b Default: 30
			@brief en_OffDelay Bits: 8
				@ref EN_OFFDELAY_10_SEC (1): 10 sec
				@ref EN_OFFDELAY_30_SEC (3): 30 sec
				@ref EN_OFFDELAY_1_MIN (6): 1 min
				@ref EN_OFFDELAY_2_MIN (12): 2 min
				@ref EN_OFFDELAY_3_MIN (18): 3 min
				@ref EN_OFFDELAY_5_MIN (30): 5 min
				@ref EN_OFFDELAY_10_MIN (60): 10 min
				@ref EN_OFFDELAY_15_MIN (90): 15 min
				@ref EN_OFFDELAY_20_MIN (120): 20 min
*/
#define EE_DELAY_FOR_MESSAGE_RAIN_OFF (userEeprom[0x444c])

#endif /* INC_RAIN_PARAMS_H */
