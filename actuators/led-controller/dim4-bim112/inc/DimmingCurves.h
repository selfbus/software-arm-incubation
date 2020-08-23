/*
 * DimmingCurves.h
 *
 *  Created on: 06.03.2020
 *      Author: x
 */

#ifndef DIMMINGCURVES_H_
#define DIMMINGCURVES_H_


/* -Erhält den aktuellen Dimmwert im Format 0-MAXOUTPUTVALUE und den Kanal 0-3
 * -rechnet den Wert anhand der param. Dimmkurve um
 * -ruft die entspr. Ausgaberoutine (PwmOut...) direkt auf
 * TODO Umrechnung fest auf Wertebereich 0-10000 programmiert
 */
void setOutput(int ch, int value);


#endif /* DIMMINGCURVES_H_ */
