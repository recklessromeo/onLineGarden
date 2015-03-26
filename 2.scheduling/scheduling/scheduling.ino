/****************************************************************************************/
/*                SCHEDULE A JOB                                                        */
/*                                                                                      */
/* This sketch load all the scheduling that a resource had to perform.                  */
/* Each minute we check if the job had to start.                                        */ 
/* We use a Json object like this:                                                      */
/*  -> {"first":{"hour":9,"minute":20,"name":"first","time":60}, ...} <-                */
/*  hour: hour that the job had to start                                                */
/*  minute: the miunte that the job had to start                                        */
/*  time: duration (in second) that the job had to go on                                */
/*                                                                                      */
/* Each day it reload the planned scheduling in order to retrieve all possible updates. */
/* For this reason, please do not insert any schedule at hour: 00:00.                   */
/*                                                                                      */
/* The scheduling are retrieved from a Real time DataBase (Firebase) by Json objcet     */
/*                                                                                      */
/* This example code is in the public domain.                                           */
/* Athor: P. Giorgi                                                                     */
/****************************************************************************************/

/*****************************************/
/* FIX: start scheduling when it's ready */
/*****************************************/


#include <Process.h>
#include <ArduinoJson.h>
  
// Variables
String URL = "https://";
String firebaseURL = "luminous-heat-4517.firebaseio.com"; /* Substitute with your firebase App*/
int numOfScheduling = 0;
int hours, minutes, seconds;
String hourString;
String minString;
String secString;
String lastMinute = "MM";
long* hour = 0;
long* minute = 0;
long* time = 0;

void setup() {
  // Initialize Bridge
  Bridge.begin();

  // Initialize Serial
  Serial.begin(9600);
  delay(10000);
  URL.concat(firebaseURL);
  URL.concat("/scheduling.json");

  // Inizialize Scheduling
  refreshScheduling();

}

void refreshScheduling() {
/***************************************************/
/* 1. curl to identify the lenght of Json response */
/***************************************************/
  Process p;
  p.begin("curl");
  p.addParameter("-k");
  p.addParameter(URL);
  p.run();
  
  int i = 0;
  int jsonLength = 0;
  /* Find the lenght of Json object */
  while (p.available()>0) {
    char c = p.read();
    jsonLength++;
  }
  
  /* Create an array exact for the request */
  char json[jsonLength];

/*****************************************/
/* 2. curl to retreive the Json response */
/*****************************************/
  Process p1;
  p1.begin("curl");
  p1.addParameter("-k");
  p1.addParameter(URL);
  p1.run();
  
  while (p1.available()>0) {
    char c = p1.read();
    json[i] = c;
    i++;
  }

/**************************/
/* 3. print Json response */
/**************************/
  Serial.println("--> Start read <--");
  int a = 0;
  while (a < jsonLength) {
    Serial.print(json[a]);
    a++;
  }
  Serial.println();
  Serial.println("--> End read <--");
  Serial.flush();
  
/********************************/
/* 4. analyse the json response */
/********************************/
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  
  /* json is like -->{"prima":{"hour":9,"minute":20,"name":"prima","time":60},"seconda":{"hour":10,"minute":30,"name":"seconda","time":30}} */
  
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  
  //Reset the number of scheduling
  numOfScheduling = 0;
  for (JsonObject::iterator item = root.begin(); item !=root.end(); ++item)
  {
    numOfScheduling++;
  }
  Serial.print("Nunber of scheduling: ");
  Serial.println(numOfScheduling);
  if (hour != 0) {
    delete [] hour;
    delete [] minute;
    delete [] time;
  }
  hour = new long [numOfScheduling];
  minute = new long [numOfScheduling];
  time = new long [numOfScheduling];
  int z = 0;
  for (JsonObject::iterator item = root.begin(); item !=root.end(); ++item)
  {
    Serial.print("Schedule name: ");
    Serial.print(item->key);
    Serial.println("-> ");
    JsonObject& schedule = root[item->key].asObject();
    hour[z] = schedule["hour"];
    minute[z] = schedule["minute"];
    time[z] = schedule["time"];
    z++;
  }
  Serial.println(" Wait 10sec.");
  delay(10000);
  Serial.println(" Finish Wait.");
}

void loop() {
  Process date;
  int hours, minutes, seconds;
  int lastSecond = -1;
  date.begin("date");
  date.addParameter("+%T");
  date.run();
  if(date.available()>0) {
    String timeString = date.readString();
    int firstColon = timeString.indexOf(":");
    int secondColon= timeString.lastIndexOf(":");
    String hourString = timeString.substring(0, firstColon); 
    String minString = timeString.substring(firstColon+1, secondColon);
    String secString = timeString.substring(secondColon+1);
    Serial.print("Actual hour: ");
    Serial.print(hourString);
    Serial.print(":");
    Serial.println(minString);
    if(minString.equals(lastMinute)) {
    /* Check if the control of scheduling has been already performed for this hour (HH:MM) */
      Serial.print("It's the same minute (MM-> ");
      Serial.print(lastMinute);
      Serial.println("). Wait 10 sec.");
      delay(10000);
    } else {
      if(hourString.equals("00") && minString.equals("00") ) {
        /* If it's midnight, the list of scheduling had to be recalculated */
        Serial.println("It's midnight. Recalculare the Scheduling!");
        refreshScheduling();
      } else {
        /* Check if a scheduling had to start.
        /* If more than one schedule start, the longer wins. So the duration (sec) is simply calulated with the longest one */
        Serial.println("Waiting a scheduling.");
        delay(10000);
      }
    }
    lastMinute = minString;
  } else {
    Serial.println("WARNING! Hour is not available.");
  }
}
