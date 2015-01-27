/*!
***********************************************************************
* \file   
*     VideoCoDecLib.h
* \brief
*     interface to the codec library
* \author
*     Mark Palkow
*     Valeri George
* \copyright
*     2003-2005 Daviko GmbH
*     The copyright of this software source code is the property of Daviko GmbH.
*     The program(s) may be used and/or copied only with the written permission
*     of Daviko GmbH and in accordance with the terms and conditions stipulated 
*     in the agreement/contract under which the software has been supplied.
***********************************************************************
*/

#ifndef _VIDEOCODECLIB_H_
#define _VIDEOCODECLIB_H_

/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* \prototypes
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/

#ifdef __cplusplus
extern "C"
{
#endif

  //YUV Formats
#define DAVC_YUV_420		0
#define DAVC_YUV_444		1
  
#define ERR_NALU       -1
#define NALU_UNDEFINED  0
#ifndef SOP
#define SOP             1         //!< Start Of Picture
#endif
#ifndef SOS
#define SOS             2         //!< Start Of Slice
#endif
#ifndef EOS
#define EOS             4         //!< End Of Slice
#endif
#ifndef EOP
#define EOP             8         //!< End Of Picture
#endif
#ifndef SPS
#define SPS             16
#endif
#ifndef PPS
#define PPS             32
#endif
  

// /*!
// * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// * \types
// * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// */
// typedef struct
// {
//   CoDec *c;
//   int   w,h;
//   int   avr_interlen;
//   int   eps_value;
// } CoderHandle;

/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function GetVersion()
*
* \brief    returns version number of codec
*
* \param    none
*
* \note     none
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
float GetDavcVersion();
char* GetDavcVersionString( int handle );

void DAVC_GetCodecInfo( char *p_info );
char* DAVC_GetCodecInfoFull( int handle );


// void DAVC_DrawMV( unsigned char *p_luma,   
//                  unsigned char *p_chromaU,
//                  unsigned char *p_chromaV,
//                  int width,              
//                  int height,             
//                  int ***motion_vectors,  
//                  mb_mode_t *p_mb_mode,
//                  unsigned int i_draw_mv );

void DAVC_DrawCodecMV( int handle,
                        unsigned int i_draw_mv );


/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function		InitEncode()
*
* \brief		init an encoder instance
*
* \param		w:	(in) width
*				h:	(in) height
*
* \return		a handle to the encoder on success, NULL on error
*
* \note			to be called first
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
int InitEncode( int w,
               int h,
               int idr_mode,
               int num_ref_frames,
               int me_analyze_level,
               int search_range,
               int subpel_refine_level,
               int min_num_skip_intra_refresh,
               int yuv_format,
               int rdopt,
               int intra_refresh_rate,
               int constrained_intra_pred,
               int intra16x16,
               int intra_period_in_gops,
               int temporal_levels,
               int *num_frame_insert,
               int non_ref_tl,
               int max_skip_refresh_qp_diff,
               int skip_refresh_rate );

void SetTemporalLevelQpOffsets( int handle, const int *qp_offsets );

int DAVC_OpenDTraceLog( int handle, char *psz_filename );
void DAVC_CloseDTraceLog( int handle );

int DAVCEnc_GetParameterSet( int handle, unsigned char *p_param_buf, unsigned int i_param_buf );

/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function		Encode()
*
* \brief		encode one frame
*
* \param		handle:		(in) a handle returned by a previous call to InitEncode()
*						yuv:			(in) pointer to frame to be encoded
*						qp:				(in) quantizer value
*						imode:    (in) if 1 a full frame will be encoded (INTRA I); 
*													 if 0 a INTER (P) frame will be encoded
*						slice_mode(in) slice mode (0=off 1=fixed #mb in slice 2=fixed #unsigned chars in slice) 
*           slice_arg (in) for slice modes 1 and 2
*						hadamard	(in) allow to use hadamard transform for prediction control
*						inter_cost_threshold (in) below this cost value of inter prediction, intra prediction is skipped
*						code:     (in/out) pointer to memory with encoded slice parts
*						slice_part_length: (in/out) pointer(double) to memory with pointers to encoded slice parts lengths
*						total_slices_len:   (in/out) pointer to variable contains total length of encoded slice(s)
*						yuv_out		(out)pointer to the reconstructed frame						
*
* \return		0 on success, <0 on error
*
* \note			to be called after InitEncode
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
int Encode(int handle,
					 unsigned char *yuv,
					 int qp,
           int b_reset_gop,
           int *pb_intra,
					 int slice_mode,
					 int slice_size,
					 int deblock_filter_disable,
			  	 int inter_cost_threshold,
					 unsigned char *code,
					 int **slice_part_length,
           int *total_slices_len,
					 unsigned char **yuv_out);
					 

/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function		ExitEncoder()
*
* \brief		close this encoder instance
*
* \param		handle:		(in) a handle returned by a previous call to InitEncode()
*
* \note			to be called last
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
void ExitEncoder(int handle);


/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function		InitDecode()
*
* \brief		open a new (empty) decoder instance
*
* \return		a handle to the decoder on success, NULL on error
*
* \note			to be called first
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
int InitDecode(int w, int h);


/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function		Decode()
*
* \brief		decode one frame
*
* \param		handle:	(in) a handle returned by a previous call to InitDecode()
*				code:       (in) pointer to memory of the the codeed frame
*				length:     (in) unsigned chars in the code memory
*				yuv:        (out) pointer to frame memory there the frame will be decoded to
*				w:          (out) width of the decoded frame
*				h:          (out) height of the decoded frame
*
* \return		0 or 1 on success; 
*				0 means frame was an INTER; 1 means frame was a INTRA
*				<0 on error
*
* \note			to be called after InitDecode
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
int Decode(int handle, unsigned char* code, int length, unsigned char** yuv, int *w, int *h, int *yuv_format, int *intra);


/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function		ExitDecoder()
*
* \brief		close this decoder instance
*
* \param		handle:		(in) a handle returned by a previous call to InitDecode()
*
* \note			to be called last
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
void ExitDecoder(int handle);


/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function		EncoInitQparam()
*
* \brief		initialize start encoder QP 
*
* \param		avr_interlen:		(in) average framelength in unsigned chars
*           width       :   (in) frame width
*           height      :   (in) frame height
*               
* \return       none			
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
void EncoInitQparam( int avr_interlen, int width, int height );

/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function		EncoEncodeSlice()
*
* \brief		calclulate new encoder QP
*
* \param		old_qparam      :     (in) last(old) QP
*           len             :     (in) last length of the stream unit
*           old_intra_flag  :     (in) intra frame coding flag
*
* \return		new QP for next frame
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
int EncoGetNewQparam( 	int 	old_qparam, int	len, int	old_intra_flag	);


/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function		GetStatisticalData()
*
* \brief		provides statistical data
*
* \param		stat:		(in/out) pointer to the statistic structure
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
void GetStatisticalData(int handle, void **stat);

#if 0
/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function   GetStatisticalData()
*
* \brief    provides encoded data (slices) with info about them
*           the user must provide buffer with slice data and big enough buffer for output data
*
* \param    pbOutput:       (out) output info data with binary data
*           pcbOutputLen:   (out) lenght of output data
*           pbBinData:      (in ) binary data (slices)
*           piSlicePartLen  (in ) infos about slice length
*           iNumSlicesToPack(in ) number of slices to pack, valid until the last slice is found
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
void GetPackedSlices( unsigned char *pbOutput, long *pcbOutputLen, unsigned char *pbBinData, int *piSlicePartLen, int iNumSlicesToPack );

/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function InitSliceExtractor()
*
* \brief    inits extractor of slices
*
* \param    pbBuf:       (in) buffer with packed slices
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
bool InitSliceExtractor( int handle, unsigned char *pbBuf, int iCodeLen );

/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function   ExtractSlice()
*
* \brief    provides encoded data (slices) with info about them
*           the user must provide buffer with slice data and big enough buffer for output data
*
* \param    pbSlice:         (out) slice binary data
*           iSlicePartLen:   (out) lenght of data
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
bool ExtractSlice( int handle, unsigned char **pSlice, int *iSliceLen );

/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function   SynchronizeToNextSlice()
*
* \brief    in case of errors searching of the start point of next slice in buffer
*
* \param    none
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
bool SynchronizeToNextSlice( int handle );

/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function  GoToNextSlice()
*
* \brief    provides the start point of next slice in buffer
*
* \param    none
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
void GoToNextSlice( int handle );
#endif

#ifdef __cplusplus
}
#endif

#endif //_VIDEOCODECLIB_H_
