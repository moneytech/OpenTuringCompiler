#include "TuringCommon/StreamManager.h"

#include <sstream>

namespace TuringCommon {
	TInt StreamManager::registerStream(WriteStreamFunc outStream, ReadStreamFunc inStream) {
        TInt newStreamId = Streams.getNew();
        TuringStream *newStream = Streams.get(newStreamId);
        // can't create a stream that can't read or write
        if (outStream == NULL && inStream == NULL) {
            return -1;
        }
        newStream->WriteFunc = outStream;
        newStream->ReadFunc = inStream;
        return newStreamId;
    }
    
    void StreamManager::closeStream(TInt streamNumber) {
        Streams.remove(streamNumber);
    }
    
    bool StreamManager::setSpecialStream(TInt specialStream, TInt streamNumber, std::string *errMsg) {
        if (specialStream >= 0) {
            if(errMsg) *errMsg = "Special stream number to redirect must be negative.";
            return false;
        }
        if (!assertValidStream(streamNumber, errMsg)) return false;
        
        SpecialStreams[specialStream] = streamNumber;
        
        return true;
    }
    
    bool StreamManager::readFromStream(TInt streamNumber, TString *buffer, TInt length, std::string *errMsg) {
        streamNumber = getSpecialStream(streamNumber);
        if (!assertValidStream(streamNumber, errMsg)) return false;
        TuringStream *stream = Streams.get(streamNumber);
        if (stream->ReadFunc == NULL) {
            if(errMsg) *errMsg = "Tried to read from a stream with no read functionality.";
            return false;
        }
        // call the function pointer
        (*(stream->ReadFunc))(streamNumber,buffer,length);
        return true;
    }
    
    bool StreamManager::writeToStream(TInt streamNumber, TString *text, std::string *errMsg) {
        streamNumber = getSpecialStream(streamNumber);
        if (!assertValidStream(streamNumber, errMsg)) return false;
        TuringStream *stream = Streams.get(streamNumber);
        if (stream->WriteFunc == NULL) {
            if(errMsg) *errMsg = "Tried to write to a stream with no write functionality.";
            return false;
        }
        // call the function pointer
        (*(stream->WriteFunc))(streamNumber,text);
        return true;
    }
    
    bool StreamManager::assertValidStream(TInt streamNumber, std::string *errMsg) {
        if (!Streams.exists(streamNumber)) {
            if(errMsg) {
                std::ostringstream os;
                os << "Stream number " << streamNumber << " does not exist.";
                *errMsg = os.str();
            }
            return false;
        }
        return true;
    }
    
    TInt StreamManager::getSpecialStream(TInt streamNum) {
        if (SpecialStreams.find(streamNum) == SpecialStreams.end()) {
            return streamNum;
        }
        return SpecialStreams[streamNum];
    }
}