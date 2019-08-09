#ifndef __CY4M_H__
#define __CY4M_H__

#include <stdio.h>
#include <stdlib.h>

typedef enum enum_y4m_format {
	  eY4M_C420JPEG = 1
	, eY4M_C420PALDV
	, eY4M_C420
	, eY4M_C422
	, eY4M_C444
} enum_y4m_format;


typedef struct struct_y4m_param {
	short            width;
	short            height;
	enum_y4m_format  format;
	unsigned int     frm_size;
} struct_y4m_param;


typedef enum enum_y4m_state {
	  eY4M_WAIT_HEADER
	, eY4M_WAIT_FORMAT
} enum_y4m_state;


class cY4M
{
public:
	cY4M();
	~cY4M();
	bool init(char *inFile, struct_y4m_param &param);
	//return 1 based frame #, if outFrame provided, data copied
	int getFrame(char *outFrame);
protected:
	FILE                        *m_file;
	struct_y4m_param            m_param;
	int                         m_frm_nr;
	char                        *m_buf;
};


#endif	//__CY4M_H__
