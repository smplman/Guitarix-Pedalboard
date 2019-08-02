/**
  Name: SerialFileListing.h
  Purpose: Library to list files over serial
  IO: Serial
  @author Stephen Peery
  @version 0.5 7/14/19
  @email smp4488_AT_gmail.com
  @github https://github.com/smp4488/Guitarix-Pedalboard
  @credit Robin2 https://forum.arduino.cc/index.php?topic=396450.0
/*/

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

    const unsigned long fetchTimout = 1000; // 1 second timeout
    unsigned long lastFetchTime;

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
