#include "HanReader.h"

HanReader::HanReader()
{
  
}

void HanReader::setup(HardwareSerial *hanPort, unsigned long baudrate, SerialConfig config, Stream *debugPort)
{
	// Initialize H/W serial port for MBus communication
	if (hanPort != NULL)
	{
    // debug->print("Setting input baudrate to ");
    // debug->println(baudrate);
		hanPort->begin(baudrate, config);
		while (!hanPort) {}
	}
	
	han = hanPort;
	bytesRead = 0;
	debug = debugPort;
	if (debug) debug->println("MBUS serial setup complete");
}

void HanReader::setup(HardwareSerial *hanPort)
{
	setup(hanPort, 2400, SERIAL_8E1, NULL);
}

void HanReader::setup(HardwareSerial *hanPort, Stream *debugPort)
{
	setup(hanPort, 2400, SERIAL_8E1, debugPort);
}

void HanReader::Clear() {
	reader.Clear();
}
/*
 * HanReader::read()
 */
bool HanReader::read()
{
  // if (array_index < 384)   // 64 384 273
  // debug->println("*");
  if (han->available())
  {
    // debug->println("!");
    byte newByte = han->read();
    /* Test setup
      byte newByte = rx_data[array_index];
      array_index++;
    */
    // debug->print("Sending character:" ); debug->print(array_index); debug->print(" => "); debug->println(newByte,HEX);
    // debug->print(newByte, HEX);debug->print(" ");
    if (reader.Read(newByte))
    {
      element_counter = 0;
      // debug->println("Received a TRUE from DlmsReader");
      bytesRead = reader.GetRawData(buffer, &element_counter);
      // debug->print("Ready to handle: "); debug->print(bytesRead); debug->print(" Elements: "); debug->println(element_counter);
      readOut[0] = element_counter;
      parseBlock(element_counter);
      /*
      // debug->println("List elements in array");

      for (int i = 0; i < 16; i++)
      {
        debug->print(i); debug->print(" - "); debug->println(readOut[i]);
      }
      debug->println();
      */
      // list = (List)kaifa.GetListID(buffer, 0, bytesRead);
      return true;
    }
  }
  return false;
}

/*
 * parseBlock
 * Loop through the buffer fetched by reader.GetRawData
 */
int HanReader::parseBlock(int element_counter)
{
  int i;
  int index = 0;
  long  valL;
  byte  valS[40];
  int hash = 0;
  int element_type;
  int element_length;
  
    for(i = 0; i < element_counter; i++)
    {
      // Get OBIS code
      index += 4;
      hash = buffer[index] + buffer[index+1] + 4*buffer[index+2] + 2*buffer[index+3] + buffer[index+4];
      index += 6;
      /*
      debug->print("Ready to parse value: "); debug->print(buffer[index]); debug->print(" Offset: "); debug->println(buffer[index]);
      debug->print(" Hash: "); debug->println(buffer[hash]);
      */
      element_type = int(buffer[index]);
      element_length = 0;
      switch (int(buffer[index])) {
        case  6:  // UINT_32
          valL = buffer[index+1];
          valL = valL << 8 | buffer[index+2];
          valL = valL << 8 | buffer[index+3];
          valL = valL << 8 | buffer[index+4];
          index += 11;
          break;
        case 9:   // Octet-String
          element_length = int(buffer[index+1]);
          for (int j=0; j <int(buffer[index+1]); j++)
            valS[j] = buffer[index+2+j];
          index += buffer[index+1] + 2;
          break;
        case 10:  //  String
          element_length = int(buffer[index+1]);
          for (int j=0; j <int(buffer[index+1]); j++)
            valS[j] = buffer[index+2+j];
          index += buffer[index+1] + 2;
          break;
        case 16:  // INT_16
          valL = buffer[index+1];
          valL = valL << 8 | buffer[index+2];
          index += 9;
          break;
        case 18:  //  UINT_16
          valL = buffer[index+1];
          valL = valL << 8 | buffer[index+2];
          index += 9;
          break;
        default:
          debug->println("How come I got here ???");
      }
      // debug->print("Hash: "); debug->println(hash);
      switch (element_type) {
        case 6:
        case 16:
        case 18:
          // Save 
          /*
          debug->print("Value of integer: "); debug->println(valL);
          debug->print("Position in array is ");
          debug->println(getPosition(hash));
          */
          readOut[getPosition(hash)] = valL;
          break;
        case 9:
        case 10:
          // Ignore String return....
          /*
          for (int j=0; j < element_length; j++)
            debug->print(valS[j], HEX); debug->print(" ");
          debug->println();
          */
          break;
        default:
          debug->println("Not going here!!!");
      }
  }
}

int HanReader::getListSize()
{
	return listSize;
}

time_t HanReader::getPackageTime()
{
	int packageTimePosition = dataHeader 
		+ (compensateFor09HeaderBug ? 1 : 0);

	return getTime(buffer, packageTimePosition, bytesRead);
}

time_t HanReader::getTime(int objectId)
{
	return getTime(objectId, buffer, 0, bytesRead);
}

int HanReader::getInt(int objectId)
{
	return getInt(objectId, buffer, 0, bytesRead);
}

int HanReader::getPosition(int hash)
{
  switch (hash) {
    case 19:
      return 1;
    case 23:
      return 2;
    case 27:
      return 3;
    case 31:
      return 4;
    case 139:
      return 5;
    case 299:
      return 6;
    case 143:
      return 7;
    case 223:
      return 8;
    case 303:
      return 9;
    case 21:
      return 10;
    case 25:
      return 11;
    case 29:
      return 12;
    case 33:
      return 13;
    case 219:
      return 14;
    default:
      return 15;
  }
}

int HanReader::getInt(int start, int size)
{

	int value = 0;
	int bytes = 0;
	int mask = 0x01;		// Use if negative value (MSB = 1)

	for (int i = start + size -1; i >= start; i--)
	{
		mask = mask << 8;
		value = value << 8 | buffer[i];
	}

	if (buffer[start+size-1] >= 0x80) {	// Negative value
		return value - mask;
	}

	return value;

}

String HanReader::getString(int objectId)
{
	return getString(objectId, buffer, 0, bytesRead);
}

String HanReader::getString(int start, int Length)
{
	String value = String("");
	for (int i = start; i < start + Length; i++)
	{
		value += String((char)buffer[i]);
	}
	return value;
}


int HanReader::findValuePosition(int dataPosition, byte *buffer, int start, int length)
{
	// The first byte after the header gives the length 
	// of the extended header information (variable)
	int headerSize = dataHeader + (compensateFor09HeaderBug ? 1 : 0);
	int firstData = headerSize + buffer[headerSize] + 1;

	for (int i = start + firstData; i<length; i++)
	{
		if (dataPosition-- == 0)
			return i;
		else if (buffer[i] == 0x0A) // OBIS code value
			i += buffer[i + 1] + 1;
		else if (buffer[i] == 0x09) // string value
			i += buffer[i + 1] + 1;
		else if (buffer[i] == 0x02) // byte value (1 byte)
			i += 1;
		else if (buffer[i] == 0x12) // integer value (2 bytes)
			i += 2;
		else if (buffer[i] == 0x06) // integer value (4 bytes)
			i += 4;
		else
		{
			if (debug)
			{
				debug->print("Unknown data type found: 0x");
				debug->println(buffer[i], HEX);
			}
			return 0; // unknown data type found
		}
	}

	if (debug)
	{
		debug->print("Passed the end of the data. Length was: ");
		debug->println(length);
	}

	return 0;
}


time_t HanReader::getTime(int dataPosition, byte *buffer, int start, int length)
{
	// TODO: check if the time is represented always as a 12 byte string (0x09 0x0C)
	int timeStart = findValuePosition(dataPosition, buffer, start, length);
	timeStart += 1;
	return getTime(buffer, start + timeStart, length - timeStart);
}

time_t HanReader::getTime(byte *buffer, int start, int length)
{
	int pos = start;
	int dataLength = buffer[pos++];

	if (dataLength == 0x0C)
	{
		int year = buffer[pos] << 8 |
			buffer[pos + 1];

		int month = buffer[pos + 2];
		int day = buffer[pos + 3];
		int hour = buffer[pos + 5];
		int minute = buffer[pos + 6];
		int second = buffer[pos + 7];

		return toUnixTime(year, month, day, hour, minute, second);
	}
	else
	{
		// Date format not supported
		return (time_t)0L;
	}
}

long HanReader::getLong(int position)
{
  /*
  debug->print("getLong called with value "); debug->print(position);
  debug->print(" Returning: "); debug->println(readOut[position]);
  */
  return readOut[position];
}

int HanReader::getInt(int dataPosition, byte *buffer, int start, int length)
{
	int valuePosition = findValuePosition(dataPosition, buffer, start, length);

	if (valuePosition > 0)
	{
		int value = 0;
		int bytes = 0;
		switch (buffer[valuePosition++])
		{
			case 0x12: 
				bytes = 2;
				break;
			case 0x06:
				bytes = 4;
				break;
			case 0x02:
				bytes = 1;
				break;
		}

		for (int i = valuePosition; i < valuePosition + bytes; i++)
		{
			value = value << 8 | buffer[i];
		}

		return value;
	}
	return 0;
}

String HanReader::getString(int dataPosition, byte *buffer, int start, int length)
{
	int valuePosition = findValuePosition(dataPosition, buffer, start, length);
	if (valuePosition > 0)
	{
		String value = String("");
		for (int i = valuePosition + 2; i < valuePosition + buffer[valuePosition + 1] + 2; i++)
		{
			value += String((char)buffer[i]);
		}
		return value;
	}
	return String("");
}

time_t HanReader::toUnixTime(int year, int month, int day, int hour, int minute, int second)
{
	byte daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	long secondsPerMinute = 60;
	long secondsPerHour = secondsPerMinute * 60;
	long secondsPerDay = secondsPerHour * 24;

	long time = (year - 1970) * secondsPerDay * 365L;

	for (int yearCounter = 1970; yearCounter<year; yearCounter++)
		if ((yearCounter % 4 == 0) && ((yearCounter % 100 != 0) || (yearCounter % 400 == 0)))
			time += secondsPerDay;

	if (month > 2 && (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)))
		time += secondsPerDay;

	for (int monthCounter = 1; monthCounter<month; monthCounter++)
		time += daysInMonth[monthCounter - 1] * secondsPerDay;

	time += (day - 1) * secondsPerDay;
	time += hour * secondsPerHour;
	time += minute * secondsPerMinute;
	time += second;

	return (time_t)time;
}
