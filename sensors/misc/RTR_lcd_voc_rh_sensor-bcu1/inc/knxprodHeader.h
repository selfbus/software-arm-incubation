#ifndef KNXPRODEDITORHEADER_RTR_HEADER_H
#define KNXPRODEDITORHEADER_RTR_HEADER_H
/**
 * Header File for sblib (Selfbus ARM KNX Library)
 * generated with KNXprodEditor 0.45
 *
 * Format:
 * MANUFACTURER = knxMaster->Manufacturer->KnxManufacturerId = sblib manufacturer [dez]
 * DEVICETYPE = ApplicationProgram->ApplicationNumber = sblib deviceType [dez]
 * APPVERSION = ApplicationProgram->ApplicationVersion = sblib version [dez]
 *
 * Parameter: EE_...
 * einfacher Parameter: EE_PARAMETER_[Name]   [Adresse in hex] //Addr:[Adresse in hex], Size:[Größe in hex], Beschreibung in Sprache 1
 * Union Parameter: EE_UNIONPARAMETER_[Adresse in hex]   [Adresse in hex] //Addr:[Adresse in hex], Size:[Größe in hex], Beschreibung Parameter 1 in Sprache 1, Beschreibung Parameter 2 in Sprache 1, ...
 *
 * Kommunikations Objekte: COMOBJ...
 * COMOBJ_[Beschreibung]_[Funktionstext]   [Kommunikations-Objekt Nummer] //Com-Objekt Nummer: [Nummer] ,Beschreibung: [in Sprache 1], Funktion [in Sprache 1]
 *
 *
 *
 * Geräteinformationen:
 * Device: RTR mit AirQ
 * ApplicationProgram: M-004C_A-0010-20-12C1
 *
 */


#define MANUFACTURER 76 //!< Manufacturer ID
#define DEVICETYPE 0x47e //!< Device Type
#define APPVERSION 18 //!< Application Version 1.2




#define EE_PARAMETER_4500_TEMPERATUR_ZYKLISCH_SENDEN     0x4500 //!< Addr: 0x4500, Size: 0x0001, Temperatur zyklisch senden
#define EE_PARAMETER_4501_TEMPERATUR_NACH_AENDERUNG_SENDEN     0x4501 //!< Addr: 0x4501, Size: 0x0001, Temperatur nach Änderung senden
#define EE_PARAMETER_4502_LUFTQUALITAET_ZYKLISCH_SENDEN     0x4502 //!< Addr: 0x4502, Size: 0x0001, Luftqualität zyklisch senden
#define EE_PARAMETER_4503_LUFTQUALITAET_NACH_AENDERUNG_SENDEN     0x4503 //!< Addr: 0x4503, Size: 0x0001, Luftqualität nach Änderung senden
#define EE_PARAMETER_4504_LUFTFEUCHTIGKEIT_ZYKLISCH_SENDEN     0x4504 //!< Addr: 0x4504, Size: 0x0001, Luftfeuchtigkeit zyklisch senden
#define EE_PARAMETER_4505_LUFTFEUCHTIGKEIT_NACH_AENDERUNG_SENDEN     0x4505 //!< Addr: 0x4505, Size: 0x0001, Luftfeuchtigkeit nach Änderung senden
#define EE_PARAMETER_4506_EXTERNE_TEMPERATUR_ZYKLISCH_SENDEN     0x4506 //!< Addr: 0x4506, Size: 0x0001, externe Temperatur zyklisch senden
#define EE_PARAMETER_4507_EXTERNE_TEMPERATUR_NACH_AENDERUNG_SENDEN     0x4507 //!< Addr: 0x4507, Size: 0x0001, externe Temperatur nach Änderung senden
#define EE_PARAMETER_4501_SOLLTEMPERATUR_ZYKLISCH_SENDEN     0x4501 //!< Addr: 0x4501, Size: 0x0001, Solltemperatur zyklisch senden
#define EE_PARAMETER_4502_SOLLTEMPERATUR_NACH_AENDERUNG_SENDEN     0x4502 //!< Addr: 0x4502, Size: 0x0001, Solltemperatur nach Änderung senden
#define EE_PARAMETER_4502_EXTERNER_TEMPERATURSENSOR_ANGESCHLOSSEN     0x4502 //!< Addr: 0x4502, Size: 0x0001, Externer Temperatursensor angeschlossen
#define EE_PARAMETER_4503_ANSCHLUSS_EXTERNER_TEMPERATURSENSOR     0x4503 //!< Addr: 0x4503, Size: 0x0001, Anschluss externer Temperatursensor
#define EE_PARAMETER_4504_FENSTEROEFFNUNG_ANZEIGEN     0x4504 //!< Addr: 0x4504, Size: 0x0001, Fensteröffnung anzeigen
#define EE_PARAMETER_4505_LUEFTUNGSSTATUS_ANZEIGEN     0x4505 //!< Addr: 0x4505, Size: 0x0001, Lüftungsstatus anzeigen
#define EE_PARAMETER_4506_HEIZUNGSSTATUS_ANZEIGEN     0x4506 //!< Addr: 0x4506, Size: 0x0001, Heizungsstatus anzeigen
#define EE_PARAMETER_4503_SENDEINTERVALL_FAKTOR_1_60     0x4503 //!< Addr: 0x4503, Size: 0x0006, Sendeintervall Faktor [1...60]
#define EE_PARAMETER_4509_SENDEINTERVALL_EINHEIT     0x4509 //!< Addr: 0x4509, Size: 0x0002, Sendeintervall Einheit
#define EE_PARAMETER_4504_WERTAENDERUNG_GRAD_C     0x4504 //!< Addr: 0x4504, Size: 0x0010, Wertänderung [°C]
#define EE_PARAMETER_4506_SENDEINTERVALL_FAKTOR_1_60     0x4506 //!< Addr: 0x4506, Size: 0x0006, Sendeintervall Faktor [1...60]
#define EE_PARAMETER_450C_SENDEINTERVALL_EINHEIT     0x450C //!< Addr: 0x450C, Size: 0x0002, Sendeintervall Einheit
#define EE_PARAMETER_4507_WERTAENDERUNG_PPM     0x4507 //!< Addr: 0x4507, Size: 0x0010, Wertänderung [ppm]
#define EE_PARAMETER_4509_SENDEINTERVALL_FAKTOR_1_60     0x4509 //!< Addr: 0x4509, Size: 0x0006, Sendeintervall Faktor [1...60]
#define EE_PARAMETER_450F_SENDEINTERVALL_EINHEIT     0x450F //!< Addr: 0x450F, Size: 0x0002, Sendeintervall Einheit
#define EE_PARAMETER_450A_WERTAENDERUNG_PROZENT     0x450A //!< Addr: 0x450A, Size: 0x0010, Wertänderung [%]
#define EE_PARAMETER_450C_SENDEINTERVALL_FAKTOR_1_60     0x450C //!< Addr: 0x450C, Size: 0x0006, Sendeintervall Faktor [1...60]
#define EE_PARAMETER_4512_SENDEINTERVALL_EINHEIT     0x4512 //!< Addr: 0x4512, Size: 0x0002, Sendeintervall Einheit
#define EE_PARAMETER_450D_WERTAENDERUNG_GRAD_C     0x450D //!< Addr: 0x450D, Size: 0x0010, Wertänderung [°C]
#define EE_PARAMETER_450F_SENDEINTERVALL_FAKTOR_1_60     0x450F //!< Addr: 0x450F, Size: 0x0006, Sendeintervall Faktor [1...60]
#define EE_PARAMETER_4515_SENDEINTERVALL_EINHEIT     0x4515 //!< Addr: 0x4515, Size: 0x0002, Sendeintervall Einheit
#define EE_PARAMETER_4510_TASTER_ENTPRELLZEIT_MS     0x4510 //!< Addr: 0x4510, Size: 0x0008, Taster Entprellzeit [ms]


#define COMOBJ_0_TEMPERATURMESSWERT_INTERNER_SENSOR     0 //!< Com-Objekt Nummer: 0, Beschreibung: Temperaturmesswert interner Sensor, Funktion:
#define COMOBJ_1_TEMPERATURSOLLWERT     1 //!< Com-Objekt Nummer: 1, Beschreibung: Temperatursollwert, Funktion:
#define COMOBJ_2_EXTERNE_SOLLTEMPERATUR     2 //!< Com-Objekt Nummer: 2, Beschreibung: Externe Solltemperatur, Funktion:
#define COMOBJ_3_LUFTQUALITAET     3 //!< Com-Objekt Nummer: 3, Beschreibung: Luftqualität, Funktion:
#define COMOBJ_4_LUFTFEUCHTIGKEIT     4 //!< Com-Objekt Nummer: 4, Beschreibung: Luftfeuchtigkeit, Funktion:
#define COMOBJ_5_FENSTERSTATUS     5 //!< Com-Objekt Nummer: 5, Beschreibung: Fensterstatus, Funktion:
#define COMOBJ_6_LUEFTUNGSSTUFE     6 //!< Com-Objekt Nummer: 6, Beschreibung: Lüftungsstufe, Funktion:
#define COMOBJ_7_TEMPERATUR_EXTERNER_SENSOR     7 //!< Com-Objekt Nummer: 7, Beschreibung: Temperatur externer Sensor, Funktion:

#endif
