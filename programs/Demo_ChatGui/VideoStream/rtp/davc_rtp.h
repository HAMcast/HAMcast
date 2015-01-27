/*!
*************************************************************************************
* \file 
*     davc_rtp.h
* \brief
*     H264 RTP Packetizer/Depacketizer
* \author
*     Valeri George (valeri_george@web.de)
* \copyright
*     2008 Daviko GmbH
*     The copyright of this software source code is the property of Daviko GmbH.
*     The program(s) may be used and/or copied only with the written permission
*     of Daviko GmbH and in accordance with the terms and conditions stipulated 
*     in the agreement/contract under which the software has been supplied.
*************************************************************************************
*/
#ifndef _DAVC_RTP_H_
#define _DAVC_RTP_H_

#ifndef RTP_H264_PACKETIZATION_MODE0
#define RTP_H264_PACKETIZATION_MODE0 0 /* Single NAL unit mode */
#endif
#ifndef RTP_H264_PACKETIZATION_MODE1
#define RTP_H264_PACKETIZATION_MODE1 1 /* Non-interleaved mode */
#endif


void* H264RTP_Open      ( uint32_t i_mtu, uint32_t b_packetizer, uint32_t i_packetization_mode );
int H264RTP_Close       ( void *p_handle );
int H264RTP_GetFmtp     ( void *p_handle, uint8_t *p_au, uint32_t i_au_len, char **pp_fmtp );
int H264RTP_PacketizeAU ( void *p_handle, uint8_t *p_au, uint32_t i_au_len,
                        uint8_t **p_packets, uint32_t **p_packets_len_array );
int H264RTP_GetParameterSetFromFmtp( void *p_handle, char *psz_fmtp,
                                    uint8_t **p_param_set, uint32_t *p_param_set_len );
int H264RTP_Depacketize ( void *p_handle, uint8_t *p_packet, uint32_t i_packet, int64_t i_cts,
                          uint32_t b_mbit, uint8_t **p_au, uint32_t *p_au_len );
int H264RTP_GetTemporalLevel( const uint8_t *p_packet, int i_num_temporal_levels );


#endif //#define _DAVC_RTP_H_