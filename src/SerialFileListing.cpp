#include "SerialFileListing.h"

SerialFileListing::SerialFileListing()
{
 // SerialUSB.println("SerialFileListing");
  byte numChars = 64;
  charSize = numChars;
  receivedChars = new char[charSize];
  tempChars = new char[charSize];
  strtokIndx = new char[charSize];
  messageFromPC = new char[charSize];
}

void SerialFileListing::setSerial(Stream &streamObject)
{
  //_streamRef = streamObject;
  //_streamRef.println("Set Stream object");
}

void SerialFileListing::sendText(String text)
{
  //_streamRef.println(text);
}

bool SerialFileListing::goFolder(String folderName)
{
  dir = folderName;
  return true;
}

void SerialFileListing::poll()
{
  recvWithStartEndMarkers();
  recieveData();
}


void SerialFileListing::recvWithStartEndMarkers()
{
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (SerialUSB.available() > 0 && newData == false) {
        rc = SerialUSB.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= charSize) {
                    ndx = charSize - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void SerialFileListing::recieveData()
{
    if (newData == true) {
        strcpy(tempChars, receivedChars);
        parseData();
        newData = false;
    }
}

void SerialFileListing::parseData()
{
    static long ndx = 0;
    char* cmd = strtok(tempChars,"::");
    Serial.println(cmd);
    char* s = strtok(NULL,":");
    // SerialUSB.println(s);

    if (String(cmd) == "ls")
    {
      while(s) {
        s = strtok(NULL, ",");
        ndx++;
      }
    }

    if (String(cmd) == "count")
    {
      countVal = atof(s);
      fetchingCount = false;
    }

    if (String(cmd) == "entryIdx")
    {
      entryIdxVal = atol(s);
      fetchingEntryIdx = false;
    }

    if (String(cmd) == "entry")
    {
      entryVal = String(s);
      fetchingEntry = false;
    }
}

long SerialFileListing::count()
{
  SerialUSB.print("::count::");
  SerialUSB.println(dir);

  fetchingCount = true;

  while (fetchingCount){poll();}

  // SerialUSB.print("count ");
  // SerialUSB.println(countVal);

  return countVal;
}

long SerialFileListing::entryIdx(String name)
{
  SerialUSB.print("::entryIdx::");
  SerialUSB.print(dir);
  SerialUSB.println("::" + name);

  fetchingEntryIdx = true;

  while (fetchingEntryIdx){poll();}

  // SerialUSB.println("entryIdxVal" + entryIdxVal);

  return entryIdxVal;
}
String SerialFileListing::entry(long idx)
{
  SerialUSB.print("::entry::");
  SerialUSB.print(dir);
  SerialUSB.print("::");
  SerialUSB.println(idx);

  fetchingEntry = true;

  while (fetchingEntry){poll();}

  // SerialUSB.println("entryVal " + entryVal);

  return entryVal;
}

SerialFileListing SFL;
