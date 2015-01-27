/*****************************************************************************
 * 
 * File  : avmux_def.h
 *
 * Author: Alexander Haderer
 *		
 *
 * Date  : 14. Feb 1998
 *
 * Remark: Datentypen/Konstante fuer
 *		Audio Video Multiplexer Funktionen zum Zusammenstellen /
 *		Auseinandernehmen der Audio/Video Frames fuer die
 *		Uebertragungsstrecke
 *
 *****************************************************************************
 */

#ifndef AVMUX_DEF_H
#define AVMUX_DEF_H


/**********************************************************************
 * t y p e n   &   k o n s t a n t e n
 **********************************************************************



/* 
 * ID Konstanten extra data
 */
#define EXTRA_EMPTY			0
#define KEEP_OPEN			1
#define NEW_CALL			2
#define NEW_MEMBER			3
#define NEW_CONNECT			4
#define NEW_RECORD			7
#define NEW_MODERATOR		8
#define NEW_SNR 			9
#define NEW_APPSHARE_IMG 	10
#define NEW_APPSHARE_MSG 	11
#define NEW_JOINED		 	12
#define NEW_CALL_ACCEPT 	13
#define NEW_CALL_REJECT 	14
#define NEW_HANGUP		 	15
#define NEW_NAT			 	16
#define NEW_EJECT		 	17
#define NEW_AUDIO		 	18
#define NEW_VIDEO_FRAME 	19
#define NEW_VIDEO_END 		20
#define NEW_CLOSE		 	21
#define NEW_DATA		 	22
#define NEW_APP_CTRL		23
#define NEW_HAND		 	24
#define NEW_AV			 	25
#define NEW_LOOKUPLISTE	 	26
#define NEW_MAIL2IP		 	27
#define NEW_QUIT		 	28
#define NEW_INVITE			29
#define NEW_LEFT		 	30
#define NEW_P2P			 	31
#define NEW_FT_START	 	32
#define NEW_FT_PART		 	33
#define NEW_FT_STOP		 	34
#define NEW_LINK		 	35
#define NEW_STREAM		 	36
#define NEW_SIP_CALL	 	37
#define NEW_SIP_CALL_FAILED	38
#define NEW_SIP_MEMBER		39
#define NEW_SIP_VIDEO		40
#define NEW_SIP_AUDIO		41
#define NEW_SIP_INTRA		42
#define NEW_SIP_ACCEPT		43
#define NEW_SIP_REJECT		44
#define NEW_SIP_DTMF		45
#define NEW_SIP_REMOVED		46
#define NEW_LAYER1_VIDEO	47
#define NEW_LAYER2_VIDEO	48
#define NEW_UPDATE			50
#define NEW_CONTACT			51
#define NEW_SIP_EJECT		52

/*
 * Datentyp fuer extra data
 */
typedef struct {
    unsigned char id;
    unsigned int data[2];
}  ExtraDataType;

typedef ExtraDataType *ExtraDataTypePtr;


#endif 	/* AVMUX_DEF_H */
