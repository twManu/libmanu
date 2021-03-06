#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "y4mLib.h"
#include <debug.h>

//all header must locate in first BUF_SZ
#define BUF_SZ       256
#define SIGNATURE    "YUV4MPEG2"
#define FRM          "FRAME"

cY4M::cY4M(int level)
	: m_file(NULL)
	, m_frm_nr(0)
	, m_buf(NULL)
	, m_debugLevel(level)
{
	memset(&m_param, 0, sizeof(m_param));
}


cY4M::~cY4M()
{
	if( m_file ) fclose(m_file);
	if( m_buf ) free(m_buf);
}


bool cY4M::init(char *inFile, struct_y4m_param &param)
{
	char *foundSIG=NULL;
	m_buf = (char *)calloc(1, BUF_SZ);
	if( !m_buf ) {
		ERROR("fail to allocate temp buffer\n");
		return false;
	}
	m_file = fopen(inFile, "rb");
	if( !m_file ) {
		ERROR("fail to open %s\n", inFile);
		return false;
	} else dbg(0, "open %s\n", inFile);

	//get till SIGNATURE found and the whole line in m_buf
	do {
		if( !fgets(m_buf, BUF_SZ, m_file) ) {
			ERROR("fail to locate header\n");
			return false;
		}
		dbg(2, "%s", m_buf);
		foundSIG = strstr(m_buf, SIGNATURE);
	} while( !foundSIG );

	//parsing
	for( char *tokend=NULL, *tok=foundSIG+strlen(SIGNATURE); 1; ) {
		if( !tok || '\n'==*tok || 0==*tok ) {   //a line done
			if( m_param.format && m_param.width && m_param.height ) {
				//y size
				m_param.frm_size = m_param.width * m_param.height;
				switch(m_param.format) {
				case eY4M_C422:
					m_param.frm_size <<= 1;
					break;
				case eY4M_C444:
					m_param.frm_size += m_param.frm_size<<1;
					break;
				default:
					m_param.frm_size += m_param.frm_size>>1;
					break;
				}
				//param done
				dbg(1, "frame size=%u\n", m_param.frm_size);
				param = m_param;
				return true;
			}
			//get a new line
			if( !fgets(m_buf, BUF_SZ, m_file) ) {
				ERROR("fail to parse header\n");
				return false;
			}
			dbg(2, "%s", m_buf);
			tokend=NULL;
			tok=m_buf;
		}
		if( 0x20==*tok ) {
			++tok;
			continue;
		}
		switch( *tok++ ) {
		case 'W':
			m_param.width = strtol(tok, &tokend, 10);
			tok = tokend;
			dbg(1, "W=%d\n", m_param.width);
			break;
		case 'H':
			m_param.height = strtol(tok, &tokend, 10);
			tok = tokend;
			dbg(1, "H=%d\n", m_param.height);
			break;
		case 'C':
			if( 0==strncmp("422", tok, 3) ) {
				m_param.format = eY4M_C422;
				dbg(1, "C422\n");
			} else if( 0==strncmp("444", tok, 3) ) {
				m_param.format = eY4M_C444;
				dbg(1, "C444\n");
			} else if( 0==strncmp("420jpeg", tok, 7) ) {
				m_param.format = eY4M_C420JPEG;
				dbg(1, "C420jpeg\n");
			}else if( 0==strncmp("420paldv", tok, 8) ) {
				m_param.format = eY4M_C420PALDV;
				dbg(1, "C420paldv\n");
			} else if( 0==strncmp("420", tok, 3) ) {
				m_param.format = eY4M_C420;
				dbg(1, "C420\n");
			} else {
				ERROR("unknown format\n");
			}
			tok = strchr(tok, 0x20);
			break;
		default:
			break;
		}
	}

	return false;
}


int cY4M::getFrame(char *outFrame)
{
	char *foundFrm = NULL;

	if( !m_param.frm_size ) return 0;
	do {
		if( !fgets(m_buf, BUF_SZ, m_file) ) {
			if( !feof(m_file) )
				ERROR("fail to locate frame\n");
			return 0;
		}
		dbg(2, "%s", m_buf);
		foundFrm = strstr(m_buf, FRM);
	} while( !foundFrm );

	if( outFrame ) {
		if( 1!=fread(outFrame, m_param.frm_size, 1, m_file) )
			return 0;
	} else {
		for( int rest=m_param.frm_size; rest; ) {
			int thisRead = rest>BUF_SZ ? BUF_SZ : rest;
			if( 1!=fread(m_buf, thisRead, 1, m_file) )
				return 0;
			rest -= thisRead;
		}
	}
	if( !m_frm_nr ) dbg(1, "got frame ");
	dbg(1, "%d ", m_frm_nr+1);
	return ++m_frm_nr;
}
