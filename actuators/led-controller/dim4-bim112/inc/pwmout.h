/*
 * pwmout.h
 *
 *  Created on: 20.02.2020
 *      Author: Uwe S.
 */

#ifndef PWMOUT_H_
#define PWMOUT_H_

#define PWM_MAX_1000  16000	//  Timer Überlauf für 1kHz PWM Frequenz
#define PWM_MAX_600   26665	//  Timer Überlauf für 600Hz PWM Frequenz
#define PRESCALER 2			//  für PWM Timer



class pwmout{
public:
	void begin(int ch);
	void setpwm(int value);  // 0-10000 = 0% - 100%
protected:
	int channel;
	int pwmmax;
	bool isactive = false;
};

#endif /* PWMOUT_H_ */
