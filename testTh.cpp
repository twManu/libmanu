#include <stdio.h>
#include <stdlib.h>
#include "rtpStreamTask.h"
#include "FileStream.h"


int main(int argc, char *argv[])
{
	FileStream *streamV, *streamA;
	RateCtrl *rcVideo, *rcAudio;
	rtpStreamTask *rtpTaskV, *rtpTaskA;
	BaseThread *baseTask;
	int ret = -1;
	//video
	streamV = new FileStream("../3gpp/stream2.dump", FileStream::BM_RDONLY);
	if( !streamV ) {
		ERROR("Fail to create stream\n");
		goto err;
	}
	if( !streamV->open() ) {
		ERROR("fail to open stream\n");
		goto delVStream;
	}
	rcVideo = new RateCtrl();
	if( !rcVideo ) {
		ERROR("Fail to create rateCtrl\n");
		goto delVStream;
	}
	rtpTaskV = new rtpStreamTask(streamV, rcVideo, 15*1000); //15ms sleep
	if( !rtpTaskV ) {
		ERROR("Fail to create task\n");
		goto delVrc;
	}
	if( !rtpTaskV->Init("127.0.0.1", 5568, "video", 90000) ) {
		ERROR("Fail to create task\n");
		goto delVTask;
	}
	//audio
	streamA = new FileStream("../3gpp/stream1.dump", FileStream::BM_RDONLY);
	if( !streamA ) {
		ERROR("Fail to create streamA\n");
		goto delVTask;
	}
	if( !streamA->open() ) {
		ERROR("fail to open streamA\n");
		goto delAStream;
	}
	rcAudio = new RateCtrl();
	if( !rcAudio ) {
		ERROR("Fail to create rateCtrl\n");
		goto delAStream;
	}
	rtpTaskA = new rtpStreamTask(streamA, rcAudio, 15*1000); //15ms sleep
	if( !rtpTaskA ) {
		ERROR("Fail to create taskA\n");
		goto delArc;
	}
	if( !rtpTaskA->Init("127.0.0.1", 5566, "audio", 44100) ) {
		ERROR("Fail to create task\n");
		goto delATask;
	}	
	rcVideo->timeReset();
	*rcAudio = *rcVideo;
	baseTask = rtpTaskV;
	//if( !rtpTaskA->Start() ) goto delVTask;
	baseTask=rtpTaskA;
	if( baseTask->Start() ) {
		while( baseTask->isRunning() ) sleep(5);	//till terminate
		ret = 0;
	}


delATask:
	delete rtpTaskA;
delArc:
	delete rcAudio;
delAStream:
	delete streamA;
delVTask:
	delete rtpTaskV;
delVrc:
	delete rcVideo;
delVStream:
	delete streamV;
err:
	return ret;
}
