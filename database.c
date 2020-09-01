#include <my_global.h>
#include <mysql.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

// config params

const char *ipAddress = "localhost";
const char *userName = "root";
const char *password = "f5949ff32b111038876d6f6a3a79e67c";
const char *databaseName = "taxiGPS";

const char *tableName = "TaxiInfo";
const char *createTableString = "CREATE TABLE TaxiInfo(IMEI TEXT,PowerStatus TEXT, Latitude TEXT, LatitudeDir TEXT,\
									Longitude TEXT, LongitudeDir TEXT,Speed TEXT,HeadingInDegree TEXT,\
									GPSDate TEXT,GPSTime TEXT,\
									SignalStrength TEXT, MCC TEXT, MNC TEXT, LAC TEXT, Cellid TEXT, TripStatus TEXT,VehicleNum TEXT,RTCTime TEXT, \
									RTCDate TEXT,DayNightTariff TEXT,TripTotalTime TEXT,TripDistance TEXT, \
									TripWaitingTime TEXT,Fare TEXT, Tripid TEXT, PanicStauts TEXT,TamperStatus TEXT,GPSdatetime TEXT)";

#define NUM_DATA			27
#define INDIVIDUAL_DATA_LEN	100

// public variables
static MYSQL *con;
static FILE *fp = NULL;

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  //mysql_close(con);
  //exit(1);        
}

//$loc,861074026778715,1,1906.8319,N,07252.8319,E,1,130.81,4,210715,151245.000,71,3864,31,1,MH-22BD 1234,20:13:54,21/05/15,1,00:08:46,2.8,00:06:37,49.00
#define NUM_PACKETS	40

bool insertRecord(char *receivedPacket)
{
	char buffer[3000];
	char packetBuffer[NUM_PACKETS][1000];
	int count = 0;
	int packetCount = 0;
	int i;
	char packetResult[NUM_DATA][INDIVIDUAL_DATA_LEN];
	char *pt;
	char *packetBuff;
	
	while ((pt = strsep(&receivedPacket, "\r\n")) != NULL)
	{
    	if(strlen(pt) > 0)
    	{
			strncpy(packetBuffer[packetCount],pt,1000);	
    		packetCount++;
    	}
	}
	
	for(i = 0; i < packetCount; i++)
	{
		count = 0;
		packetBuff = packetBuffer[i];
		while ((pt = strsep(&packetBuff, ",")) != NULL)
		{
		    if(count < NUM_DATA)
			{
				strncpy(packetResult[count],pt,INDIVIDUAL_DATA_LEN);
		    	count++;
			}
    	}    
		if(count >= NUM_DATA)
		{
		   char GPSdatetime[1000]; 
		   sprintf(GPSdatetime,"%s %s",packetResult[8],packetResult[9]);
			if(!strcmp(packetResult[0],"$loc"))
			{    	
				 if(!strcmp(packetResult[20],"\0"))
				{    	
				sprintf(buffer,"INSERT INTO %s (IMEI,PowerStatus,Latitude,LatitudeDir,Longitude,LongitudeDir,Speed,HeadingInDegree,GPSDate,GPSTime,SignalStrength,MCC,MNC,LAC,Cellid,RTCTime,RTCDate,TamperStatus,GPSdatetime) VALUES('%s','0','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",tableName,packetResult[1],packetResult[2],packetResult[3],\
											packetResult[4],packetResult[5],packetResult[6],packetResult[7],\
											packetResult[8],packetResult[9],packetResult[10],packetResult[11],packetResult[12],\
											packetResult[13],packetResult[14],packetResult[17],packetResult[18],packetResult[26],GPSdatetime);
				printf("%s\n",buffer);
				}
				else
				{
				sprintf(buffer,"INSERT INTO %s (IMEI,PowerStatus,Latitude,LatitudeDir,Longitude,LongitudeDir,Speed,HeadingInDegree,GPSDate,GPSTime,SignalStrength,MCC,MNC,LAC,Cellid,TripStatus,VehicleNum,RTCTime,RTCDate,DayNightTariff,TripTotalTime,TripDistance,TripWaitingTime,Fare,Tripid,PanicStatus,TamperStatus,GPSdatetime) VALUES('%s','0','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",tableName,packetResult[1],packetResult[2],packetResult[3],\
												packetResult[4],packetResult[5],packetResult[6],packetResult[7],\
											packetResult[8],packetResult[9],packetResult[10],packetResult[11],packetResult[12],\
											packetResult[13],packetResult[14],packetResult[15],packetResult[16],packetResult[17],\
											packetResult[18],packetResult[19],packetResult[20],packetResult[21],packetResult[22],packetResult[23],packetResult[24],packetResult[25],packetResult[26],GPSdatetime);
				printf("%s\n",buffer);
				}
				if(mysql_query(con, buffer)){
			  		finish_with_error(con);
		  		}
				else
				{
				return  true;
				}
			}
			else if(!strcmp(packetResult[0],"$bat"))
			{    	
				sprintf(buffer,"INSERT INTO %s (IMEI,PowerStatus,Latitude,LatitudeDir,Longitude,LongitudeDir,Speed,HeadingInDegree,GPSDate,GPSTime,SignalStrength,MCC,MNC,LAC,Cellid,RTCTime,RTCDate,TamperStatus,GPSdatetime) VALUES('%s','1','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",tableName,packetResult[1],packetResult[2],packetResult[3],\
											packetResult[4],packetResult[5],packetResult[6],packetResult[7],\
											packetResult[8],packetResult[9],packetResult[10],packetResult[11],packetResult[12],\
											packetResult[13],packetResult[14],packetResult[17],packetResult[18],packetResult[26],GPSdatetime);
				printf("%s\n",buffer);
				if(mysql_query(con, buffer)){
			  		finish_with_error(con);
		  		}
				else
				{
				 return true;
				}
			}
			else
			{
				printf("Invalid start string\n");
			}
		}
		else
		{
			printf("Less Number of Data \n");
		}
	}
	return false;
}

void startMySQLConnection()
{
	con = mysql_init(NULL);
	if (mysql_real_connect(con, ipAddress,userName,password,databaseName, 0, NULL, 0) == NULL) 
	{
	  finish_with_error(con);
	}    
}

/*
int main(int argc, char **argv)
{
  con = mysql_init(NULL);
  
  if (con == NULL) 
  {
	  fprintf(stderr, "%s\n", mysql_error(con));
	  exit(1);
  }  

  if (mysql_real_connect(con, "localhost", "root", "adpcmm", 
          "testdb", 0, NULL, 0) == NULL) 
  {
      finish_with_error(con);
  }    
	if(argc > 1)
	{
		insertRecord(argv[1]);	
	}
	else
	{
		printf("No input arguments\n");
	}
  mysql_close(con);
  exit(0);
}
*/

/*

$loc,861074026778715,1,1906.8319,N,07252.8319,E,1,130.81,4,210715,151245.000,71,3864,31,1,MH-22BD 1234,20:13:54,21/05/15,1,00:08:46,2.8,00:06:37,49.00
$loc,861074026778715,1,1906.8319,N,07252.8319,E,1,130.81,4,210715,151245.000,71,3864,31,1,MH-22BD 1234,20:13:54,21/05/15,1,00:08:46,2.8,00:06:37,49.00
$loc,861074026778715,1,1906.8319,N,07252.8319,E,1,130.81,4,210715,151245.000,71,3864,31,1,MH-22BD 1234,20:13:54,21/05/15,1,00:08:46,2.8,00:06:37,49.00
$loc,861074026778715,1,1906.8319,N,07252.8319,E,1,130.81,4,210715,151245.000,71,3864,31,1,MH-22BD 1234,20:13:54,21/05/15,1,00:08:46,2.8,00:06:37,49.00
$loc,861074026778715,1,1906.8319,N,07252.8319,E,1,130.81,4,210715,151245.000,71,3864,31,1,MH-22BD 1234,20:13:54,21/05/15,1,00:08:46,2.8,00:06:37,49.00
$loc,861074026778715,1,1906.8319,N,07252.8319,E,1,130.81,4,210715,151245.000,71,3864,31,1,MH-22BD 1234,20:13:54,21/05/15,1,00:08:46,2.8,00:06:37,49.00
$loc,861074026778715,1,1906.8319,N,07252.8319,E,1,130.81,4,210715,151245.000,71,3864,31,1,MH-22BD 1234,20:13:54,21/05/15,1,00:08:46,2.8,00:06:37,49.00
$loc,861074026778715,1,1906.8319,N,07252.8319,E,1,130.81,4,210715,151245.000,71,3864,31,1,MH-22BD 1234,20:13:54,21/05/15,1,00:08:46,2.8,00:06:37,49.00
$loc,861074026778715,1,1906.8319,N,07252.8319,E,1,130.81,4,210715,151245.000,71,3864,31,1,MH-22BD 1234,20:13:54,21/05/15,1,00:08:46,2.8,00:06:37,49.00




*/

/*
CREATE TABLE TaxiInfo(IMEI TEXT, GPSfix TEXT, Latitude TEXT, LatitudeDir TEXT,Longitude TEXT, LongitudeDir TEXT,Speed TEXT,HeadingInDegree TEXT,NumSatellite TEXT,GPSDate TEXT,GPSTime TEXT,BatteryCapacity TEXT,BatteryVoltage TEXT,SignalStrength TEXT,TripStatus TEXT,VehicleNum TEXT,RTCTime TEXT,RTCDate TEXT,DayNightTariff TEXT,TripTotalTime TEXT,TripDistance TEXT,TripWaitingTime TEXT,Fare TEXT);
*/
