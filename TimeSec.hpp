#ifndef	__TIMESEC_H__
#define	__TIMESEC_H__

using namespace std;
#include <stdio.h>
#include <string>
#include <string.h>

/*
 * This class facilitates two models of time measurement.
 *	1. event driven: no idea when the next action will be taken in advance
 *                       we provide method diffXXXTime for query time between start or previous called.
 *	2. timer driven: to take action periodically.
 *                       we provide method checkTime to reflect the timer... it's polling
 * 
 * Time precision is second and can output "ddD hh:mm:ss"
 * usage:
 * 	const -> reset() -> diffPrevTime() -> diffPrevTime() -> diffStartTime() -> diffPrevTime()
 *	const(period) -> reset(-1) -> checkTimer() -> checkTimer()
 */
class TimeSec {
protected:
	unsigned int        m_startT;        //reset time
	unsigned int        m_prevT;         //previous query time
	unsigned int        m_periodT;       //periodT
#define	MIN_IN_SEC           60
#define HOUR_IN_SEC          (60*MIN_IN_SEC)
#define DAY_IN_SEC           (24*HOUR_IN_SEC)
	/*
	 * format the difference in second to "ddD hh:mm:ss"
	 */
	void strFromDiffSec(string &str, unsigned int diffSec) {
		static unsigned int tbreak[4] = { DAY_IN_SEC, HOUR_IN_SEC, MIN_IN_SEC, 0 };
		char tmp[16];
		int i;
		unsigned int val;
		
		str = "";
		for( i=0 ; i<4; ++i ) {
			if( diffSec>=tbreak[i] ) {
				if( tbreak[i] ) {
					val = diffSec/tbreak[i];
					diffSec %= tbreak[i];
				} else val = diffSec;      //compare of 0 is the last case
				if( 0==i ) sprintf(tmp, "%dD ", val);
				else sprintf(tmp, "%02d", val);
				str += tmp;
			} else {
				//0 and omit 'day'
				if( i ) str += "00";
			}
			if( 1==i || 2==i ) str += ":";
		}
	}

	virtual unsigned int getCurTime() = 0;

public:
	TimeSec(unsigned int periodSec=0)
		: m_startT(0)
		, m_prevT(0)
		, m_periodT(periodSec) {}
	~TimeSec() {}

	/*
	 * set start time
	 * In  : lastQuery - the time for last query
	 *		0 :  previous query undefined, the getTime return 0 in first call
	 *              -1 : as start time
	 *              otherwise : init query time defined by user
	 */
	void reset(int lastQuery=0) {
		m_startT = getCurTime();
		if( -1==lastQuery ) m_prevT = m_startT;
		else m_prevT = lastQuery;
	}

	/*
	 * calculate time in sec between curTime and previous called.
	 * If curTime is 0, the subroutine itself should query system time.
	 * In  : curTime - 0 : the implementation should query system time itself
	 *           otherwise : the time in sec to calculate with
	 * Out : m_prevT - updated
	 *       timeStr - the time string to return in "ddD hh:mm:ss".
	 *                 "ddD " disappear if not over a day
	 * Ret : diff in sec returned
	 */
	unsigned int diffPrevTime(string &timeStr, unsigned int curTime=0) {
		if( 0==curTime ) curTime = getCurTime();
		if( 0==m_prevT ) {
			m_prevT = curTime;                 //update
			timeStr = "";
			curTime = 0;                       //to diff
		} else {
			unsigned int preTime = m_prevT;    //preserve
			m_prevT = curTime;                 //update
			curTime -= preTime;                //to diff
			strFromDiffSec(timeStr, curTime);
		}

		return curTime;
	}
	
	/*
	 * calculate time in sec between curTime and previous reset.
	 * If curTime is 0, the subroutine itself should query system time.
	 * In  : curTime - 0 : the implementation should query system time itself
	 *           otherwise : the time in sec to calculate with
	 * Out : timeStr - the time string to return in "ddD hh:mm:ss".
	 *                 "ddD " disappear if not over a day
	 * Ret : diff in sec returned
	 */
	unsigned int diffStartTime(string &timeStr, unsigned int curTime=0) {
		if( 0==curTime ) curTime = getCurTime();
		curTime -= m_startT;                       //to diff
		strFromDiffSec(timeStr, curTime);

		return curTime;
	}
	
	// check if timer is up. if that happens, m_prevT is also updated
	bool checkTimer(unsigned int curTime=0) {
		if( 0==curTime ) curTime = getCurTime();
		if( curTime-m_prevT >= m_periodT ) {
			m_prevT = curTime;
			return true;
		}
		return false;
	}
};

#endif	//__TIMESEC_H__