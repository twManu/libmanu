#ifndef	__INTERPOLATE_HPP__
#define	__INTERPOLATE_HPP__

#include <string.h>
#include <math.h>
#include <stdio.h>
#define	DEF_DEBUG
#include "../debug.h"

typedef unsigned int u_int;


template <class objT> class interpolate {
protected:
	objT                           m_last_L;
	objT                           m_last_R;
public:
	enum {
		  CHANNEL_L = 1
		, CHANNEL_R
	};
	interpolate() {
		resetLastData();		
	}
	~interpolate() {}
	void resetLastData(void) {
		m_last_L = m_last_R = 0;
	}
	//single channel
	bool resample(u_int inNum, objT *inData, u_int outNum, objT *outData, u_int channel) {
		int i, index_front;
		float ratio, weight_front, weight_rear, distance;
		objT last_post_data;

		if( 0==inNum || 0==outNum ) return false;

		if( inNum==outNum ) {
			memcpy(outData, inData, outNum * sizeof(objT));
		} else {
			last_post_data = (CHANNEL_L==channel) ? m_last_L : m_last_R;
			ratio = (float) inNum / (float) outNum;
			for (i = 0; i <(int) outNum; i++) {
				distance = ratio * i;
				weight_rear = distance - floor(distance);
				weight_front = 1.0 - weight_rear;
				index_front = ((int) distance) - 1;
				ERROR("dist=%f, wr/wf=%f/%f, index_front=%d, ",\
					distance, weight_rear, weight_front, index_front);
				if (index_front < 0) {
					outData[i] = last_post_data * weight_front
						+ inData[index_front + 1] * weight_rear;
					ERROR("%d vs %d = %d\n",\
						last_post_data, inData[index_front + 1], outData[i]);
				} else {
					outData[i] = inData[index_front] * weight_front
						+ inData[index_front + 1] * weight_rear;
					ERROR("%d vs %d = %d\n",\
						inData[index_front], inData[index_front + 1], outData[i]);
				}
			}
		}

		if( CHANNEL_L==channel ) m_last_L = inData[inNum - 1];
		else m_last_R = inData[inNum - 1];

		return true;
	}
	//dual channel
	bool resample(u_int inNum, objT *inData, u_int outNum, objT *outData) {
		int i;
		float ratio, weight_front, weight_rear, distance;
		int index_front;

		if( 0==inNum || 0==outNum ) return false;
		else if( inNum==outNum ) {
			memcpy(outData, inData, outNum * sizeof(objT));
		} else {
			ratio = (float) inNum / (float) outNum;
			for (i = 0; i <(int) outNum; i++) {
				distance = ratio * i;
				weight_rear = distance - floor(distance);
				weight_front = 1.0 - weight_rear;
				index_front = ((int) distance) - 1;
				ERROR("dist=%f, wr/wf=%f/%f, index_front=%d, ",\
					distance, weight_rear, weight_front, index_front);
				if (index_front < 0) {
					outData[2*i] = m_last_L * weight_front
						+ inData[2*(index_front+1)] * weight_rear;
					ERROR("L: %d vs %d = %d, ",\
						m_last_L, inData[2*(index_front+1)], outData[2*i-2]);
					outData[2*i+1] = m_last_R * weight_front
						+ inData[2*(index_front+1)+1] * weight_rear;
					ERROR("R: %d vs %d = %d\n",\
						m_last_R, inData[2*(index_front+1)+1], outData[2*i-1]);
				} else {
					outData[2*i] = inData[2*index_front] * weight_front
						+ inData[2*(index_front+1)] * weight_rear;
					ERROR("L: %d vs %d = %d, ",\
						inData[2*index_front], inData[2*(index_front+1)], outData[2*i-2]);
					outData[2*i+1] = inData[2*index_front + 1] * weight_front
						+ inData[2*(index_front+1)+1] * weight_rear;
					ERROR("L: %d vs %d = %d\n",\
						inData[2*index_front + 1], inData[2*(index_front+1)+1], outData[2*i-1]);
				}
			}
		}

		m_last_L = inData[2*inNum - 2];
		m_last_R = inData[2*inNum - 1];
		return true;
	}
};

#endif	//__INTERPOLATE_HPP__