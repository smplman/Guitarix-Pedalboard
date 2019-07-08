#ifndef SerialFileListing_h
#define SerialFileListing_h

#include "Arduino.h"

class SerialFileListing
{
  public:
    SerialFileListing();
    byte charSize;
    String dir = "/";

    void setSerial(Stream &streamObject);
    void poll();
    void begin();
    void sendText(String text);
    bool goFolder(String folderName);
    long count();
    long entryIdx(String name);
    String entry(long idx);
    void recieveData();
    void printList();

  private:
//    const byte numChars = 64;
    char* receivedChars;
    char* tempChars;        // temporary array for use when parsing
    char* strtokIndx;
    char* messageFromPC;
    boolean newData = false;

    long countVal = 0;
    bool fetchingCount = false;

    bool fetchingEntryIdx = false;
    long entryIdxVal = 0;

    bool fetchingEntry = false;
    String entryVal = "";

    //Stream &_streamRef;
    void recvWithStartEndMarkers();
    void parseData();
};

extern SerialFileListing SFL;

#endif
