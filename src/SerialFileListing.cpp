#include "SerialFileListing.h"

SerialFileListing::SerialFileListing()
{
 // Serial.println("SerialFileListing");
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

    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

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
    char* cmd = strtok(tempChars,":");
    char* s = strtok(NULL,",");

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
  Serial.print(":count:");
  Serial.println(dir);

  fetchingCount = true;

  while (fetchingCount){poll();}

  return countVal;
}

long SerialFileListing::entryIdx(String name)
{
  Serial.print(":entryIdx:");
  Serial.print(dir);
  Serial.println(":" + name);

  fetchingEntryIdx = true;

  while (fetchingEntryIdx){poll();}

  return entryIdxVal;
}
String SerialFileListing::entry(long idx)
{
  Serial.print(":entry:");
  Serial.print(dir);
  Serial.print(":");
  Serial.println(idx);

  fetchingEntry = true;

  while (fetchingEntry){poll();}

  // Serial.println(entryVal);

  return entryVal;
}

SerialFileListing SFL;
