/**
  Name: SerialFileListing.cpp
  Purpose: Library to list files over serial
  IO: Serial
  @author Stephen Peery
  @version 0.5 7/14/19
  @email smp4488_AT_gmail.com
  @github https://github.com/smp4488/Guitarix-Pedalboard
  @credit Robin2 https://forum.arduino.cc/index.php?topic=396450.0
/*/

#include "SerialFileListing.h"

SerialFileListing::SerialFileListing()
{
 // SerialUSB.println("SerialFileListing");
  byte numChars = 128;
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
    char* s = strtok(NULL,":");

    // SerialUSB.println(cmd);
    // SerialUSB.println(s);

    if(strcmp(cmd, "ls") == 0)
    {
      while(s) {
        s = strtok(NULL, ",");
        ndx++;
      }
    }

    if(strcmp(cmd, "count") == 0)
    {
      countVal = atof(s);
      fetchingCount = false;
    }

    if(strcmp(cmd, "entryIdx") == 0)
    {
      entryIdxVal = atol(s);
      fetchingEntryIdx = false;
    }

    if (strcmp(cmd, "entry") == 0)
    {
      entryVal = s;
      fetchingEntry = false;
    }
}

long SerialFileListing::count()
{
  SerialUSB.print("::count::");
  SerialUSB.println(dir);

  fetchingCount = true;
  lastFetchTime = millis();
  countVal = 0;

  while (fetchingCount && (millis() - lastFetchTime <= fetchTimout)){poll();}

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
  lastFetchTime = millis();
  entryIdxVal = -1;

  while (fetchingEntryIdx && (millis() - lastFetchTime <= fetchTimout)){poll();}

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
  lastFetchTime = millis();
  entryVal = "";

  while (fetchingEntry && (millis() - lastFetchTime <= fetchTimout)){poll();}

  // SerialUSB.println("entryVal " + entryVal);

  return entryVal;
}

SerialFileListing SFL;
