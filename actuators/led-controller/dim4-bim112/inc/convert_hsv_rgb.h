/*
 * convert_hsv_rgb.h
 *
 *  Created on: 24.02.2020
 *      Author: X
 */

#ifndef CONVERT_HSV_RGB_H_
#define CONVERT_HSV_RGB_H_


// Die aktuellen RGB/HSV Werte werden normalerweise aus den aktuellen Ausgangswerten berechnet. Dies ist für den Farbwert nicht immer möglich.
// In diesem Fall wird der letzte gespeicherte Farbwert aud dieser Variable genutzt
unsigned char storedHueValue = 0;


void rgb2hsv(const unsigned char &src_r, const unsigned char &src_g, const unsigned char &src_b, unsigned char &dst_h, unsigned char &dst_s, unsigned char &dst_v);
void hsv2rgb(const unsigned char &src_h, const unsigned char &src_s, const unsigned char &src_v, unsigned char &dst_r, unsigned char &dst_g, unsigned char &dst_b);


#endif /* CONVERT_HSV_RGB_H_ */
