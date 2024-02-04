// aptLoader.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <sstream>
#include <utility>
#include <ctime>

//#include "GeomagnetismHeader.h"
#include "GeoMagLib.h"

CGeoMagnetic* geoMagnetic = nullptr;

FILE* apt;

errno_t err;

FILE* ofileAPT;
FILE* ofileRWY;
FILE* ofileRMK;
FILE* ofileATT;
FILE* ofileARS;

#define RECLEN 1533

char buffer[RECLEN];
char recordType[4];

char timeOut[10];
time_t now;
tm* ltm;

char tilde = '~';
char newline[] = "\r\n";

static std::string facilityId;
static std::string f;

char hemiLat;
char hemiLon;

char ll[25];

double decimalLat;
double decimalLon;

std::string s;

int count = 0;

std::string result;

std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
	str.erase(0, str.find_first_not_of(chars));
	
	return str;
}

std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
	str.erase(str.find_last_not_of(chars) + 1);
	
	return str;
}

std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
	return ltrim(rtrim(str, chars), chars);
}

std::vector<std::string> split(std::string const & s, char delim)
{
	std::vector<std::string> result;
	
	std::istringstream iss(s);

	for (std::string token; std::getline(iss, token, delim); )
	{
		result.push_back(std::move(token));
	}

	return result;
}

void MakeField(std::string& str, int p, int l)
{
	str.clear();
	
	str.assign(&buffer[p], l);
	
	trim(str);
}

void WriteRecordTilde(std::string& str, FILE* ofile)
{
	fwrite(str.data(), str.length(), 1, ofile);

	fwrite(&tilde, 1, 1, ofile);
}

void WriteRecordNewline(std::string& str, FILE* ofile)
{
	fwrite(str.data(), str.length(), 1, ofile);

	fwrite(&newline, 2, 1, ofile);
}

void WriteAirport()
{
	WriteRecordTilde(facilityId, ofileAPT);

	//icao
	MakeField(f, 1210, 7);
	
	if (f.length() != 0)
	{
		WriteRecordTilde(f, ofileAPT);
	}
	else
	{
		WriteRecordTilde(facilityId, ofileAPT);
	}

	//type
	MakeField(f, 14, 13);
	WriteRecordTilde(f, ofileAPT);

	//state
	MakeField(f, 91, 2);
	WriteRecordTilde(f, ofileAPT);

	//county
	MakeField(f, 70, 21);
	WriteRecordTilde(f, ofileAPT);

	//city
	MakeField(f, 93, 40);
	WriteRecordTilde(f, ofileAPT);

	//name
	MakeField(f, 133, 50);
	WriteRecordTilde(f, ofileAPT);

	//ownerType
	MakeField(f, 183, 2);
	WriteRecordTilde(f, ofileAPT);

	//facilityUse
	MakeField(f, 185, 2);
	WriteRecordTilde(f, ofileAPT);

	//ownersName
	MakeField(f, 187, 35);
	WriteRecordTilde(f, ofileAPT);

	//ownersAddr
	MakeField(f, 222, 72);
	WriteRecordTilde(f, ofileAPT);

	//ownersCityStateZip
	MakeField(f, 294, 45);
	WriteRecordTilde(f, ofileAPT);

	//ownersPhone
	MakeField(f, 339, 16);
	WriteRecordTilde(f, ofileAPT);

	//managersName
	MakeField(f, 355, 35);
	WriteRecordTilde(f, ofileAPT);

	//managersAddr
	MakeField(f, 390, 72);
	WriteRecordTilde(f, ofileAPT);

	//managersCityStateZip
	MakeField(f, 462, 45);
	WriteRecordTilde(f, ofileAPT);

	//managersPhone
	MakeField(f, 507, 16);
	WriteRecordTilde(f, ofileAPT);

	//latitude
	MakeField(f, 523, 15);
	WriteRecordTilde(f, ofileAPT);

	//longitude
	MakeField(f, 550, 15);
	WriteRecordTilde(f, ofileAPT);

	//elevation
	MakeField(f, 578, 7);
	WriteRecordTilde(f, ofileAPT);

	// Mag Var
	MakeField(f, 523, 15);
	if (strcmp(f.c_str(), "") == 0)
	{
		hemiLat = f.at(f.length() - 1);
	}
	hemiLat = f.at(f.length() - 1);
	f.replace(f.length() - 1, 1, "");
	auto partsLat = split(f, '-');

	decimalLat = atof(partsLat[0].data()) + (atof(partsLat[1].data()) / 60) + (atof(partsLat[2].data()) / 3600);
	if (hemiLat == 'S' || hemiLat == 'W')
	{
		decimalLat *= -1;
	}

	MakeField(f, 550, 15);
	hemiLon = f.at(f.length() - 1);
	f.replace(f.length() - 1, 1, "");
	auto partsLon = split(f, '-');

	decimalLon = atof(partsLon[0].data()) + (atof(partsLon[1].data()) / 60) + (atof(partsLon[2].data()) / 3600);
	if (hemiLon == 'S' || hemiLon == 'W')
	{
		decimalLon *= -1;
	}

	geoMagnetic->CalculateFieldElements(decimalLat, decimalLon, 1200.0, 'F');

	memset(ll, 0x00, 25);
	sprintf(ll, "%14.2f", geoMagnetic->GeoMagneticElements->Decl);

	f.clear();
	f.append(ll);
	trim(f);

	WriteRecordTilde(f, ofileAPT);
	
	//sectional
	MakeField(f, 597, 30);
	WriteRecordTilde(f, ofileAPT);

	//acreage
	MakeField(f, 632, 5);
	WriteRecordTilde(f, ofileAPT);

	//artccId
	MakeField(f, 637, 4);
	WriteRecordTilde(f, ofileAPT);

	//artccName
	MakeField(f, 644, 30);
	WriteRecordTilde(f, ofileAPT);

	//fss
	MakeField(f, 712, 4);
	WriteRecordTilde(f, ofileAPT);

	//fssName
	MakeField(f, 716, 30);
	WriteRecordTilde(f, ofileAPT);

	//fssTollFreeNbr
	MakeField(f, 746, 16);
	WriteRecordTilde(f, ofileAPT);

	//fssPilotNbr
	MakeField(f, 762, 16);
	WriteRecordTilde(f, ofileAPT);

	//notamFacility
	MakeField(f, 828, 4);
	WriteRecordTilde(f, ofileAPT);

	//notamServices
	MakeField(f, 832, 1);
	WriteRecordTilde(f, ofileAPT);

	//status
	MakeField(f, 840, 2);
	WriteRecordTilde(f, ofileAPT);

	//fuelTypes
	s.clear();
	
	count = 0;
	
	for (int i = 0; i < 7; i++)
	{
		MakeField(f, 900 + (i * 5), 5);
		
		if (strcmp(f.data(), " ") > 0)
		{
			if (count > 0)
			{
				s.append(",");
				
				count = 0;
			}
			
			s.append(f.data());
			
			count++;
		}
	}
	
	MakeField(f, 900 + (7 * 5), 5);
	
	if (strcmp(f.data(), " ") > 0)
	{
		s.append(",");
		
		s.append(f.data());
	}

	WriteRecordTilde(s, ofileAPT);

	//airframeRepair
	MakeField(f, 940, 5);
	WriteRecordTilde(f, ofileAPT);

	//powerplantRepair
	MakeField(f, 945, 5);
	WriteRecordTilde(f, ofileAPT);

	//bottledOxygen
	MakeField(f, 950, 8);
	WriteRecordTilde(f, ofileAPT);

	//bulkOxygen
	MakeField(f, 958, 8);
	WriteRecordTilde(f, ofileAPT);

	//controlTower
	MakeField(f, 980, 1);
	WriteRecordTilde(f, ofileAPT);

	//unicom
	MakeField(f, 981, 7);
	WriteRecordTilde(f, ofileAPT);

	//ctaf
	MakeField(f, 988, 7);
	WriteRecordTilde(f, ofileAPT);

	//segCircle
	MakeField(f, 995, 4);
	WriteRecordTilde(f, ofileAPT);

	//beaconColor
	MakeField(f, 999, 3);
	WriteRecordTilde(f, ofileAPT);

	//landingFee
	MakeField(f, 1002, 1);
	WriteRecordTilde(f, ofileAPT);

	//transientStorage
	MakeField(f, 1124, 12);
	WriteRecordTilde(f, ofileAPT);

	//otherServices
	MakeField(f, 1136, 71);
	WriteRecordTilde(f, ofileAPT);

	//windIndicator
	MakeField(f, 1207, 3);
	WriteRecordTilde(f, ofileAPT);

	//Minimum Operational Network
	MakeField(f, 1217, 1);
	WriteRecordNewline(f, ofileAPT);
}

void WriteRunway()
{
	WriteRecordTilde(facilityId, ofileRWY);

	//runwayId
	MakeField(f, 16, 7);
	WriteRecordTilde(f, ofileRWY);

	//runwayLength
	MakeField(f, 23, 5);
	WriteRecordTilde(f, ofileRWY);

	//runwayWidth
	MakeField(f, 28, 4);
	WriteRecordTilde(f, ofileRWY);

	//runwaySurface
	MakeField(f, 32, 12);
	WriteRecordTilde(f, ofileRWY);

	//runwaySurfaceTreatment
	MakeField(f, 44, 5);
	WriteRecordTilde(f, ofileRWY);

	//runwayPavementClass
	MakeField(f, 49, 11);
	
	for (size_t i = 0; i < f.length(); i++)
	{
		if (f[i] == ' ')
		{
			f.erase(i, 1);
		}
	}
	
	WriteRecordTilde(f, ofileRWY);

	//runwayEdgeLighting
	MakeField(f, 60, 5);
	WriteRecordTilde(f, ofileRWY);

	//runwayIlsType
	MakeField(f, 71, 10);
	WriteRecordTilde(f, ofileRWY);

	//righthandTraffic
	MakeField(f, 81, 1);
	WriteRecordTilde(f, ofileRWY);

	//runwayMarkingCondition
	MakeField(f, 87, 1);
	WriteRecordTilde(f, ofileRWY);

	//runwayLatitude
	MakeField(f, 88, 15);
	WriteRecordTilde(f, ofileRWY);

	//runwayLongitude
	MakeField(f, 115, 15);
	WriteRecordTilde(f, ofileRWY);

	//runwayElevation
	MakeField(f, 142, 7);
	WriteRecordTilde(f, ofileRWY);

	//thresholdHeight
	MakeField(f, 149, 3);
	WriteRecordTilde(f, ofileRWY);

	//glidePath
	MakeField(f, 152, 4);
	WriteRecordTilde(f, ofileRWY);

	//displacedThreshold
	MakeField(f, 217, 4);
	WriteRecordTilde(f, ofileRWY);

	//touchDownElevation
	MakeField(f, 221, 7);
	WriteRecordTilde(f, ofileRWY);

	//glideSlopeIndicator
	MakeField(f, 228, 5);
	WriteRecordTilde(f, ofileRWY);

	//runwayVisualRange
	MakeField(f, 233, 3);
	WriteRecordTilde(f, ofileRWY);

	//runwayVisualValue
	MakeField(f, 236, 1);
	WriteRecordTilde(f, ofileRWY);

	//approachLightingSystem
	MakeField(f, 237, 8);
	WriteRecordTilde(f, ofileRWY);

	//reil
	MakeField(f, 245, 1);
	WriteRecordTilde(f, ofileRWY);

	//centerline
	MakeField(f, 246, 1);
	WriteRecordTilde(f, ofileRWY);

	//touchdown
	MakeField(f, 247, 1);
	WriteRecordTilde(f, ofileRWY);

	//recipElevation
	MakeField(f, 364, 7);
	WriteRecordTilde(f, ofileRWY);

	//tora
	MakeField(f, 698, 5);
	WriteRecordTilde(f, ofileRWY);

	//toda
	MakeField(f, 703, 5);
	WriteRecordTilde(f, ofileRWY);

	//asda
	MakeField(f, 708, 5);
	WriteRecordTilde(f, ofileRWY);

	//lda
	MakeField(f, 713, 5);
	WriteRecordTilde(f, ofileRWY);

	//lahsoDistance
	MakeField(f, 718, 5);
	WriteRecordTilde(f, ofileRWY);

	//lahsoRunway
	MakeField(f, 723, 7);
	WriteRecordTilde(f, ofileRWY);

	//lahsoEntity
	MakeField(f, 730, 40);
	WriteRecordTilde(f, ofileRWY);

	//singleWeight
	MakeField(f, 535, 6);
	WriteRecordTilde(f, ofileRWY);

	//dualWeight
	MakeField(f, 541, 6);
	WriteRecordTilde(f, ofileRWY);

	//tandemWeight
	MakeField(f, 547, 6);
	WriteRecordTilde(f, ofileRWY);

	//doubleTandemWeight
	MakeField(f, 553, 6);
	WriteRecordNewline(f, ofileRWY);
}

void WriteRemark()
{
	WriteRecordTilde(facilityId, ofileRMK);

	//remarkElementName
	MakeField(f, 16, 13);
	WriteRecordTilde(f, ofileRMK);

	//remarkText
	MakeField(f, 29, 1500);
	
	result.clear();
	
	for (int i = 0; i < (int)f.length(); i++)
	{
		if ((byte)f.at(i) == 126)
		{
			result.append("-");
		}
		else if (((byte)f.at(i) < 32) || ((byte)f.at(i) > 126))
		{
		}
		else
		{
			result.append(1, f.at(i));
		}
	}

	WriteRecordNewline(result, ofileRMK);
}

void WriteAttended()
{
	WriteRecordTilde(facilityId, ofileATT);

	//attendance
	MakeField(f, 18, 108);
	WriteRecordNewline(f, ofileATT);
}

void WriteArresting()
{
	WriteRecordTilde(facilityId, ofileARS);

	//arrestingEnd
	MakeField(f, 23, 3);
	WriteRecordTilde(f, ofileARS);

	//arrestingType
	MakeField(f, 26, 9);
	WriteRecordNewline(f, ofileARS);
}

int main(int argc, char* argv[])
{
	err = fopen_s(&apt, "APT.txt", "rb");

	if (err)
	{
		printf("%s\n", strerror(err));
		
		return 0;
	}

	fread_s(buffer, RECLEN, RECLEN, 1, apt);

	if (!feof(apt))
	{
		now = time(0);
		ltm = localtime(&now);
		sprintf(timeOut, "%4d.%03d", ltm->tm_year + 1900, ltm->tm_yday);

		geoMagnetic = new CGeoMagnetic(atof(timeOut), 'M');

		err = fopen_s(&ofileAPT, "aptAirport.txt", "wb");
		err = fopen_s(&ofileRWY, "aptRunway.txt", "wb");
		err = fopen_s(&ofileRMK, "aptRemark.txt", "wb");
		err = fopen_s(&ofileATT, "aptAttended.txt", "wb");
		err = fopen_s(&ofileARS, "aptArresting.txt", "wb");

		while (!feof(apt))
		{
			memset(recordType, 0x00, 4);
			memcpy(recordType, &buffer[0], 3);

			if (strcmp(recordType, "APT") == 0)
			{
				MakeField(facilityId, 27, 4);
				WriteAirport();
			}

			if (strcmp(recordType, "RWY") == 0)
			{
				WriteRunway();
			}

			if (strcmp(recordType, "RMK") == 0)
			{
				WriteRemark();
			}

			if (strcmp(recordType, "ATT") == 0)
			{
				WriteAttended();
			}

			if (strcmp(recordType, "ARS") == 0)
			{
				WriteArresting();
			}

			fread_s(buffer, RECLEN, RECLEN, 1, apt);
		}

		fclose(ofileAPT);
		fclose(ofileRWY);
		fclose(ofileRMK);
		fclose(ofileATT);
		fclose(ofileARS);

		delete geoMagnetic;
	}

	fclose(apt);
}
