#ifndef FRAME_CONVERTER_H
#define FRAME_CONVERTER_H

class frame_converter
{
	public :
		frame_converter(int x, int y);
		~frame_converter();
		bool convertFrame(int w, int h,unsigned char *input,unsigned char *output);
		int aimX, aimY;
			
	private :
		unsigned char *puffer;
};
#endif