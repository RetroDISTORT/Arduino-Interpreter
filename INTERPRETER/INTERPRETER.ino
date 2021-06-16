#include <avr/wdt.h>
#include <avr/sleep.h>
#include "Wire.h"
//#include <SdFat.h>
#include <SPI.h>
//SdFat SD;

void Analizer(String comIn, bool opt[]); //searches for symbols(#$+-*/^) and/or key words ("if"). in order to avoid steps
String Run(); //EXECUTES THE LINE STEP BY STEP
String Actuator(String comIn, bool vmode, bool opt[]); //searches for if(), else(), while(), for()STRINGS
String Resolver(String comIn, bool opt[]); //SOLVE ARITHMETIC ECUATIONS
String Capture(String comIn, int pos[]); //CAPTURES VARIABLES IN STRING
String Replace(String comIn, String newString, int P1, int P2); //REPLACE A PART OF A STRING FOR ANOTHER
String EXECUTE(String com, String spar[], String & comIn, bool reset, int rep[]); //EXECUTES THE com WITH THE PARAMETERS
String GET(String com, String spar[], String comIn); //GETS THE VARIABLE WITH THE PARAMETERS
String SD_APP(); //EXECUTES "TEST" APP IN THE SD
void Streaming(); //Streams Analog values to display

String var[11][11];    //String Variables (11x11 matrix Total:121 [0-10][0-10]) USER VARIABLES
boolean Watchdog = 0;
boolean AnalogStream = 0;
boolean Oscilloscope[4] =   {0, 0, 0, 0}; //[A1][A2][A3][B1] OSCILLOSCOPE
int Voltmeter[2] =  {4, 4}; //[INPUT1][INPUT2]VOLTMETER CHANNEL 1


void setup() {
  wdt_disable();
  Wire.begin();
  pinMode(34, OUTPUT);     //SD
  pinMode(32, OUTPUT);     //DISPLAY
  digitalWrite(32, HIGH);  //DISPLAY ON
  pinMode(61, OUTPUT);     //LED
  pinMode(3, INPUT);       //INTERRUPT
  var[0][0] = 15;
  var[0][1] = 16;
  var[1][0] = 20;
  Wire.begin();
  Serial.begin(115200);
  //Serial1.begin(250000);
  //Serial2.begin(250000);
  Serial3.begin(115200);
  Serial.println("Enter your com.");
}

char rx_byte = 0;
String command = "";

void loop() {
  while (1) {
    if (Watchdog == 1) {
      digitalWrite(61, HIGH);
      delay(500);
      digitalWrite(61, LOW);
      sleepNow();
    }
    //Serial.println("writting!");
    if ((digitalRead(3) == 0) or (Watchdog == 1))
    {
      Watchdog = 1;
      Serial.println("HI!");
      while (digitalRead(3) == 0) {}
      delay(500);
      attachInterrupt(1, wakeUpNow, LOW);
      digitalWrite(32, LOW);
//      ADCSRA &= ~(1 << 7);
      WDTCSR = (24);//change enable and WDE - also resets
      WDTCSR = (33);//prescalers only - get rid of the WDE and WDCE bit
      WDTCSR |= (1 << 6); //enable interrupt mode
      Serial.println("SLEEP!");
      sleepNow();
    }

    if ( Serial3.available() > 0) {    // is a character available?
      rx_byte = Serial3.read();       // get the character
      if (rx_byte != '\n' and rx_byte != ';') {
        // a character of the string was received
        command += String(rx_byte);
        Serial.print("char: ");
        Serial.println(String(rx_byte));
      }
      else {
        //if(com.substring(com.length()-3,com.length())!="ÿÿÿ")com=com.substring(com.length()+1,command.length()-3);
        // end of string
        command += rx_byte;
        Run();
        command = "";                // clear the string for reuse
        Serial.println("Enter your command.");
      }
    } // end: if (Serial.available() > 0)
    if (AnalogStream == 1) {
      Streaming();
    }
  }
}

String Run() {                        //0           1          2        3             4
  bool opt[5] = {0, 0, 0, 0, 0}; // [Variable] [Arithmetic] [Logic] [Hardware] [Logic Statement]
  Analizer(command, opt); //searches for symbols(#$+-*/^) and/or key words ("if"). in order to avoid steps
  if (opt[0] == 1) {
    command = Actuator(command, 1, opt);
  }
  if (opt[1] == 1 || opt[2] == 1) {
    command = Resolver(command, opt);
  }
  if (opt[3] == 1 || opt[4] == 1) {
    command = Actuator(command, 0, opt);
  }
  //Serial.println(etimer - stimer);
  return (command);
}

String SD_APP()
{
  File myFile;
  digitalWrite(34, 1);
  String GOTO = " ";
  String COLLECTOR = " ";
  Serial.print("Initializing SD card...");

  if (!SD.begin(43)) {
    Serial.println("initialization failed!");
  }
  Serial.println("initialization done.");

  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");
    while (myFile.available()) {
      command = myFile.readStringUntil('\n'); //CAPTURE STRING UNTIL IT FINDS A '\n'
      GOTO = Run();//EXECUTE
      if (GOTO != " ") {
        myFile.seek(0);
        while (COLLECTOR != GOTO and myFile.available()) {
          myFile.readStringUntil('[');
          COLLECTOR = myFile.readStringUntil(']');
        }

        if (COLLECTOR == GOTO)
          if (myFile.available() == 0) {
            Serial.println("GOTO FAILED... ERROR");
          }
        GOTO = " ";
        COLLECTOR = " ";
      }
    }
    myFile.close();
  } else {
    Serial.println("error opening test.txt");
  }
  return (" ");
}

void Streaming()
{
  //OSCILLOSCOPE////////////////////////////////////////////////
  if (Oscilloscope[0] == 1) {
    Serial.println("add 1,0," + String(analogRead(A11)));
    Serial3.print("add 1,0," + String(analogRead(A11)));
    Serial3.write(0xff);
    Serial3.write(0xff);
    Serial3.write(0xff);
  }
  if (Oscilloscope[1] == 1) {
    Serial.println("add 1,1," + String(analogRead(A13)));
    Serial3.print("add 1,1," + String(analogRead(A13)));
    Serial3.write(0xff);
    Serial3.write(0xff);
    Serial3.write(0xff);
  }
  if (Oscilloscope[2] == 1) {
    Serial.println("add 1,2," + String(analogRead(A15)));
    Serial3.print("add 1,2," + String(analogRead(A15)));
    Serial3.write(0xff);
    Serial3.write(0xff);
    Serial3.write(0xff);
  }
  if (Oscilloscope[3] == 1) {
    Serial.println("add 1,3," + String(analogRead(A5)));
    Serial3.print("add 1,3," + String(analogRead(A5)));
    Serial3.write(0xff);
    Serial3.write(0xff);
    Serial3.write(0xff);
  }
  //VOLTMETER///////////////////////////////////////////////////
  if (Voltmeter[0] == 0) {
    Serial3.print("INPUT1.txt=" + String(analogRead(A11)));
  }
  if (Voltmeter[0] == 1) {
    Serial3.print("INPUT1.txt=" + String(analogRead(A13)));
  }
  if (Voltmeter[0] == 2) {
    Serial3.print("INPUT1.txt=" + String(analogRead(A15)));
  }
  if (Voltmeter[0] == 3) {
    Serial3.print("INPUT1.txt=" + String(analogRead(A5)));
  }
  Serial3.write(0xff);
  Serial3.write(0xff);
  Serial3.write(0xff);
  if (Voltmeter[1] == 0) {
    Serial3.print("INPUT2.txt=" + String(analogRead(A11)));
  }
  if (Voltmeter[1] == 1) {
    Serial3.print("INPUT2.txt=" + String(analogRead(A13)));
  }
  if (Voltmeter[1] == 2) {
    Serial3.print("INPUT2.txt=" + String(analogRead(A15)));
  }
  if (Voltmeter[1] == 3) {
    Serial3.print("INPUT2.txt=" + String(analogRead(A5)));
  }
  Serial3.write(0xff);
  Serial3.write(0xff);
  Serial3.write(0xff);
  delay(30);
}

void Analizer(String comIn, bool opt[]) { //searches for symbols(#$+-*/^) and/or key words ("if"). in order to avoid steps
  for (int z = 0; comIn.charAt(z - 1) != '\n' and comIn.charAt(z - 1) != ';' and z < comIn.length(); z++) { //GO THROUGH CHARACTERS UNTIL IT FINDS THE END OF THE LINE
    if (comIn.charAt(z) == '"') {
      z++;
      while (comIn.charAt(z) != '"') {
        z++;
      }
    }
    if (comIn.charAt(z) == '$')
      opt[0] = 1;
    if (comIn.charAt(z) == '+' || comIn.charAt(z) == '-' || comIn.charAt(z) == '*' || comIn.charAt(z) == '/' || comIn.charAt(z) == '^')
      opt[1] = 1;
    if (comIn.charAt(z) == '=' || comIn.charAt(z) == '<' || comIn.charAt(z) == '>' || comIn.charAt(z) == '!')
      opt[2] = 1;
    if (comIn.charAt(z) == '#')
      opt[3] = 1;
    if (comIn.substring(z, z + 3) == "if(")
      opt[4] = 1;
  }
}
String Actuator(String comIn, bool vmode, bool opt[]) { //vmode = variable mode
  boolean part = 0; //[PART 0 SEARCH FOR GET COMMAND] [PART 1 GET VALUES OR ENDING]
  String spar[10]; //PARAMETERS
  int selector = 0; //SELECTS WHERE TO SAVE THE PARAMETER "INDEX"
  int pos[2]; //POSITIONS START AND END OF PARAMETER
  String com; //COMMAND TYPE
  int rep[2]; //REPLACE PART OF STRING
  int mode = 0;
  if (opt[3] == 0) { //if no hardware statements where fount in optimization, then, skip the step
    mode = 1;
  }
  while (mode <= 1) {
    for (int z = pos[0] = 0; comIn.charAt(z - 1) != '\n' and comIn.charAt(z - 1) != ';' and z < comIn.length(); z++) { //GO THROUGH CHARACTERS UNTIL IT FINDS THE END OF THE LINE
      if (part == 0) {
        if ((vmode == 1 and comIn.charAt(z) == '$') || (mode == 1 && vmode == 0 and comIn.substring(z, z + 3) == "if(") || (mode == 0  and vmode == 0 and comIn.charAt(z) == '#')) { //searching for if( statemant
          com = comIn.substring(z + 1, z + 4);
          if (comIn.substring(z, z + 3) == "if(") { //if its an if statement, just skip the next two characters "if"
            z += 2;
          }
          else
          {
            z += 4;
          }
          pos[0] = z;
          rep[0] = z - 4;
          part = 1;
        }
      }
      if (part == 1) {
        /////////////////CHAR        COLLECTOR/////////////////////////
        if (comIn.charAt(z - 1) == 39) {
          spar[selector] = comIn.charAt(z);
          selector++;
          z += 2;
          while (comIn.charAt(z - 1) != ',' and comIn.charAt(z) != ')') {
            z++;
          }
          pos[0] = z; // set new pos
        }
        /////////////////STRING     COLLECTOR/////////////////////////
        if (comIn.charAt(z) == '"') {
          pos[0] = z;
          z++;
          while (comIn.charAt(z) != '"') {
            z++;
          }
          pos[1] = z;
          z++;
          spar[selector] = comIn.substring(pos[0] + 1, pos[1]);
          selector++;
          while (comIn.charAt(z - 1) != ',' and comIn.charAt(z) != ')') {
            z++;
          }
          pos[0] = z; // set new pos
        }
        /////////////////STRING END COLLECTOR/////////////////////////
        if (comIn.charAt(z) == '(') { //STARTER
          pos[0] = z + 1; //Set the cursor after this character
        }
        if (comIn.charAt(z) == ',')  { //FIND SEPARATORS
          pos[1] = z - 1; //Set the cursor before this character
          spar[selector] = Capture(comIn, pos);
          selector++;
          pos[0] = z + 1; //Set the cursor after this character
        }
        if (comIn.charAt(z) == ')') { //FINISHER
          pos[1] = z - 1; //Set the cursor before this character
          spar[selector] = Capture(comIn, pos);
          selector++;
          pos[0] = z + 1; //Set the cursor after this character
          rep[1] = z; // Sets the ending of the replace range
          part = 0; //Restarts search of new '$'
          selector = 0; //Restarts Parameter/index selector
          z = rep[0]; //Restarts cursor position
          if (com.charAt(0) == 'f') { //if its an "if" statement
            if (spar[0] >= "1") {
              if (spar[1] == "")
                return (" ");
              else
                return (spar[1]); //Return true GOTO line
            }
            else {
              if (spar[2] == "")
                return (" ");
              else
                return (spar[2]); //Return false GOTO line
            }
          }
          else {
            if (vmode == 0) {
              comIn = Replace(comIn, EXECUTE(com, spar, comIn), rep[0], rep[1]); //if its a "#" hardware statement
              if (opt[4] == 0) { //if no "if" statements where fount in optimization, then, skip the Logic Satement step
                mode = 2;
              }
            }
            else {
              comIn = Replace(comIn, GET(com, spar, comIn), rep[0], rep[1]);
            }
          }
        }
      }
    }
    mode++;
  }
  if (vmode == 0)
    return (" ");
  else
    return (comIn);
}

String Resolver(String comIn, bool opt[]) { //FINDS VARIABLES TO REPLACE FOR STRINGS
  bool long_flag = 0; // flag for long "if"
  int part = 0; //[PART 0 SEARCH FOR GET COMMAND] [PART 1 GET VALUES OR ENDING] [PART 3 EXECUTES]
  String spar[3] = {" ", " ", " "}; //PARAMETERS
  int selector = 0; //SELECTS WHERE TO SAVE THE PARAMETER "INDEX"
  int pos[2] = {0, 0}; //POSITIONS START AND END OF PARAMETER
  String com; //COMMAND TYPE
  int rep[2] = {0, 0}; //REPLACE PART OF STRING
  int mode = 0;
  if (opt[1] == 0) { //if no Arithmetic statements where fount in optimization, then, skip the step
    mode = 1;
  }
  while (mode <= 1) {
    for (int z = pos[0] = 0; comIn.charAt(z - 1) != '\n' and comIn.charAt(z - 1) != ';' and z < comIn.length(); z++) { //GO THROUGH CHARACTERS UNTIL IT FINDS THE END OF THE LINE
      if (part == 0) {
        if (comIn.charAt(z) == 39) {
          z += 3;
        }
        if (comIn.charAt(z) == '"') {
          z++;
          while (comIn.charAt(z) != '"') {
            z++;
          }
          z++;
        }
        if (mode == 0) {
          if ((comIn.charAt(z) == '+' || comIn.charAt(z) == '-' || comIn.charAt(z) == '^' || comIn.charAt(z) == '*' || comIn.charAt(z) == '/' ) && ((comIn.charAt(z - 1) >= '0' and comIn.charAt(z - 1) <= '9') || (comIn.charAt(z - 1) == '.')) && ((comIn.charAt(z + 1) >= '0' and comIn.charAt(z + 1) <= '9') || (comIn.charAt(z + 1) == '.') || (comIn.charAt(z + 1) == '-')))
            long_flag = 1;
        }
        if (mode == 1 and (comIn.charAt(z) == '>' || comIn.charAt(z) == '<' || (comIn.charAt(z) == '!' && comIn.charAt(z + 1) == '=') || (comIn.charAt(z) == '=' && comIn.charAt(z + 1) == '=') || (comIn.charAt(z) == '<' && comIn.charAt(z + 1) == '=') || (comIn.charAt(z) == '>' && comIn.charAt(z) == '=')))
          long_flag = 1;
        if (long_flag == 1)
        { //GET COMMAND
          pos[1] = z;
          for (int i = 1 ; (comIn.charAt(z - i) >= '0' and comIn.charAt(z - i) <= '9') || (comIn.charAt(z - i) == '.' || (comIn.charAt(z - i) == '-')); i++) { //RETURN CHAR BY CHAR UNTIL IT FINDS NO NUMBER OR DECIMAL POINT OR SIGHN
            pos[0] = z - i;
            rep[0] = z - i;
          }
          spar[selector] = Capture(comIn, pos);
          if (comIn.charAt(z + 1) == '>' || comIn.charAt(z + 1) == '<' || (comIn.charAt(z + 1) == '=' )) {
            com = comIn.substring(z, z + 2);
            z++;
          }
          else {
            com = comIn.charAt(z);
          }
          part = 1;
          pos[0] = z;
          selector++;
          long_flag = 0;
        }
      }
      if (part == 1) {
        pos[0] = z;
        for (int i = 1 ; (comIn.charAt(z + i) >= '0' and comIn.charAt(z + i) <= '9') || (comIn.charAt(z + i) == '.') || (comIn.charAt(z + i) == '-'); i++) { //RETURN CHAR BY CHAR UNTIL IT FINDS NO NUMBER OR DECIMAL POINT OR SIGN
          pos[1] = z + i;
          rep[1] = z + i;
        }
        spar[selector] = Capture(comIn, pos);
        part = 3;
      }
      if (part == 3) {//FINISHER COLLECTION AND REPLACEMENT
        part = 0; //Restarts search of new 'operator'
        selector = 0; //Restarts Parameter/index selector
        z = rep[0]; //Restarts cursor position
        comIn = Replace(comIn, EXECUTE(com, spar, comIn), rep[0], rep[1]);
      }
    }
    if (opt[2] == 0) { //if no hardware statements where fount in optimization, then, skip the step
      mode = 2;
    }
    mode++;
  }
  return (comIn);
}

String Capture(String comIn, int pos[]) {
  while ((comIn.charAt(pos[0]) < '0' || comIn.charAt(pos[0]) > '9') || comIn.charAt(pos[0]) == '.' || comIn.charAt(pos[0]) == '"') { //skip characters non-num remove from string 'L'<-999+
    pos[0]++;
    if (pos[0] > comIn.length()) {
      return (""); //AVOID OVERFLOW
    }
  }
  while (((comIn.charAt(pos[0] - 1) >= '0' and comIn.charAt(pos[0] - 1 <= '9')) || (comIn.charAt(pos[0] - 1) == '.') || ((comIn.charAt(pos[0] - 1) == '-' && (comIn.charAt(pos[0] - 2) <= '0' and comIn.charAt(pos[0] - 2 >= '9'))))) and (comIn.charAt(pos[0] - 1) != '=' && (comIn.charAt(pos[0] - 1) != '>') && (comIn.charAt(pos[0] - 1) != '<')) || comIn.charAt(pos[0] - 1) == '-' ) { //skip characters num add them to string 9->999+
    pos[0]--;
    if (pos[0] < 0) {
      return (""); //AVOID OVERFLOW
    }
  }
  while ((comIn.charAt(pos[1]) < '0' || comIn.charAt(pos[1]) > '9') || (comIn.charAt(pos[1]) == '.' && comIn.charAt(pos[1] - 1) != '-')) { //skip characters non-num remove from string +999->'L'
    pos[1]--;
    if (pos[1] < 0) {
      return (""); //AVOID OVERFLOW
    }
  }
  while ((comIn.charAt(pos[1] + 1) >= '0' and comIn.charAt(pos[1] + 1) <= '9')) { //skip characters num add them to string +999<-9
    pos[1]++;
    if (pos[1] > comIn.length()) {
      return (""); //AVOID OVERFLOW
    }
  }
  return (comIn.substring(pos[0], pos[1] + 1)); //PLUS 1 IS TO AVOID ERROR
}

String Replace(String comIn, String newString, int P1, int P2) {
  return (comIn.substring(0, P1) + newString + (comIn.substring(P2 + 1, comIn.length())));
}

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val / 10 * 16) + (val % 10) );
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}

String GET(String com, String spar[], String comIn) {
  byte val;
  if (com == "VAR") {
    return (String(var[spar[0].toInt()][spar[1].toInt()]));
  }
  if (com == "WSE") {
    Wire.beginTransmission(0x68);
    Wire.write(0); // set next input to start at the seconds register
    Wire.endTransmission();
    Wire.requestFrom(0x68, 1);
    val = bcdToDec( Wire.read() & 0x7f);
    Serial.print(val , DEC);
    return (String(val));
  }
  if (com == "WMI") {
    Wire.beginTransmission(0x68);
    Wire.write(1); // set next input to start at the seconds register
    Wire.endTransmission();
    Wire.requestFrom(0x68, 1);
    val = bcdToDec( Wire.read());
    return (String(val));
  }
  if (com == "WHR") {
    Wire.beginTransmission(0x68);
    Wire.write(2); // set next input to start at the seconds register
    Wire.endTransmission();
    Wire.requestFrom(0x68, 1);
    val = bcdToDec( Wire.read() & 0x3f);
    return (String(val));
  }
  if (com == "WMO") {
    Wire.beginTransmission(0x68);
    Wire.write(3); // set next input to start at the seconds register
    Wire.endTransmission();
    Wire.requestFrom(0x68, 1);
    val = bcdToDec( Wire.read());
    return (String(val));
  }
  if (com == "WYR") {
    Wire.beginTransmission(0x68);
    Wire.write(4); // set next input to start at the seconds register
    Wire.endTransmission();
    Wire.requestFrom(0x68, 1);
    val = bcdToDec( Wire.read());
    return (String(val));
  }
  if (com == "WTP") {
    Wire.beginTransmission(0x68);
    Wire.write(1); // set next input to start at the seconds register
    Wire.endTransmission();
    Wire.requestFrom(0x68, 1);
    val = bcdToDec( Wire.read());
    return (String(val));
  }
  if (com == "IRA") {
    return (String(analogRead((spar[0].toInt()))));
  }
  if (com == "SBL") {

  }
  if (com == "SDP") {

  }
  if (com == "SIR") {

  }
  if (com == "SPK") {

  }
  if (com == "SXY") {

  }
  if (com == "SSD") {

  }
  if (com == "SNT") {

  }
  if (com == "RND") {
    return (String(random(spar[0].toInt(), spar[1].toInt())));
  }
  if (com == "IRD") {
    return (String(digitalRead((spar[0].toInt()))));
  }
  return (" ");
}

String EXECUTE(String com, String spar[], String comIn) {
  int val[3];
  byte tvar[4];
  String str[3];
  //LOGIC COMMANDS///////////////////////////////////////////////
  //Serial.print("COM: ");
  //Serial.println(com);
  if (com == ">=") {
    //comNew = (comIn.substring( 0, rep[0])) + (String(spar[0].toFloat() >= spar[1].toFloat())) + (comIn.substring(rep[1], comIn.length()));
    return (String(spar[0].toFloat() >= spar[1].toFloat()));
  }
  if (com == "<=") {
    //comNew = (comIn.substring( 0, rep[0])) + (String(spar[0].toFloat() <= spar[1].toFloat())) + (comIn.substring(rep[1], comIn.length()));
    return (String(spar[0].toFloat() <= spar[1].toFloat()));
  }
  if (com == "==") {
    //comNew = (comIn.substring( 0, rep[0])) + (String(spar[0].toFloat() == spar[1].toFloat())) + (comIn.substring(rep[1], comIn.length()));
    return (String(spar[0].toFloat() == spar[1].toFloat()));
  }
  if (com == "!=") {
    //comNew = (comIn.substring( 0, rep[0])) + (String(spar[0].toFloat() != spar[1].toFloat())) + (comIn.substring(rep[1], comIn.length()));
    return (String(spar[0].toFloat() != spar[1].toFloat()));
  }
  if (com.charAt(0) == '>') {
    //comNew = (comIn.substring( 0, rep[0])) + (String(spar[0].toFloat() > spar[1].toFloat())) + (comIn.substring(rep[1], comIn.length()));
    return (String(spar[0].toFloat() > spar[1].toFloat()));
  }
  if (com.charAt(0) == '<') {
    //comNew = (comIn.substring( 0, rep[0])) + (String(spar[0].toFloat() < spar[1].toFloat())) + (comIn.substring(rep[1], comIn.length()));
    return (String(spar[0].toFloat() < spar[1].toFloat()));
  }
  //ARITHMETIC COMMANDS///////////////////////////////////////
  if (com.charAt(0) == '+') {
    return (String((spar[0].toFloat() + spar[1].toFloat())));
  }
  if (com.charAt(0) == '-') {
    return (String(spar[0].toFloat() - spar[1].toFloat()));
  }
  if (com.charAt(0) == '*') {
    return (String(spar[0].toFloat() * spar[1].toFloat()));
  }
  if (com.charAt(0) == '/') {
    return (String(spar[0].toFloat() / spar[1].toFloat()));
  }
  if (com.charAt(0) == '^') {
    Serial.println(pow(spar[0].toFloat(), spar[1].toFloat()));
    return (String(pow(spar[0].toFloat(), spar[1].toFloat())));
  }
  if (com.charAt(0) == '$') {
    return (String(sqrt(spar[0].toFloat())));
  }
  //VAR COMMANDS//////////////////////////////////////////////////////////////////
  if (com.charAt(0) == '=') {
    var[spar[0].toInt()][spar[1].toInt()] = spar[2].toInt();
    Serial.print("SET: ");
    Serial.println(spar[2].toInt());
    return (" ");
  }
  //SD COMMANDS///////////////////////////////////////////////////////////////////
  //GOTO LINE
  //HARDWARE COMMANDS/////////////////////////////////////////////////////////////
  if (com == "ISD") {
    pinMode(spar[0].toInt(), OUTPUT);
    digitalWrite(spar[0].toInt(), spar[1].toInt());
    Serial.println("<ISD>");
    Serial.print("spar[0]:");
    Serial.println(spar[0]);
    Serial.print("spar[1]:");
    Serial.println(spar[1]);
    return (" ");
  }
  if (com == "ISP") {
    analogWrite(spar[0].toInt(), spar[1].toInt());
    return (" ");
  }
  if (com == "IST") {
    tone(spar[0].toInt(), spar[1].toInt(), spar[2].toInt());
    return (" ");
  }
  if (com == "DLY") {
    delay(spar[0].toInt());
    return (" ");
  }
  if (com == "SBL") {
    digitalWrite(34, spar[0].toInt());
    return (" ");
  }
  if (com == "SDP") {
    digitalWrite(32, spar[0].toInt());
    return (" ");
  }
  if (com == "SIR") {
    digitalWrite(30, spar[0].toInt());
    return (" ");
  }
  if (com == "SPK") {
    digitalWrite(22, spar[0].toInt());
    return (" ");
  }
  if (com == "SXY") {
    digitalWrite(40, spar[0].toInt());
    return (" ");
  }
  if (com == "SWA") {
    //Send to Sleep mode
    return (" ");
  }
  if (com == "VAR") {
    return (String(var[spar[0].toInt()][spar[1].toInt()] = spar[2].toInt()));
  }
  if (com == "SSP") {//Send through Serial
    if (spar[0].toInt() == 0) {
      if (spar[1].toInt() == 0) {
        Serial.print(spar[2]);
      }
      if (spar[1].toInt() == 1) {
        //Serial.write(String(spar[2]));
      }
    }
    if (spar[0].toInt() == 1) {
      if (spar[1].toInt() == 0) {
        //Serial1.print(spar[2]);
      }
      if (spar[1].toInt() == 1) {
        //Serial1.write(spar[2]);
      }
    }
    if (spar[0].toInt() == 2) {
      if (spar[1].toInt() == 0) {
        //Serial2.print(spar[2]);
      }
      if (spar[1].toInt() == 1) {
        //Serial2.write(spar[2]);
      }
    }
    if (spar[0].toInt() == 3) {
      if (spar[1].toInt() == 0) {
        //Serial3.print(spar[2]);
      }
      if (spar[1].toInt() == 1) {
        //Serial3.write(spar[2]);
      }
    }
  }
  if (com == "SPD") {//Set Serial Speed

  }
  if (com == "DTX") {
    Serial.print("Sending:");
    Serial.println(spar[0] + spar[1] + spar[2] + spar[3] + spar[4] + spar[5] + spar[6] + spar[7] + spar[8] + spar[9]);
    Serial3.print(spar[0] + spar[1] + spar[2] + spar[3] + spar[4] + spar[5] + spar[6] + spar[7] + spar[8] + spar[9]);
    Serial3.write(0Xff);
    Serial3.write(0Xff);
    Serial3.write(0Xff);
  }
  if (com == "GUI" or com == "MSC") {
    Wire.beginTransmission(0x68);
    Wire.write(1); // set next input to start at the seconds register
    Wire.endTransmission();
    Wire.requestFrom(0x68, 6);
    tvar[0] = (bcdToDec(Wire.read()));
    tvar[1] = (bcdToDec(Wire.read() & 0x3f));
    str[0] = "";
    /////TIME/////
    if (tvar[1] < 10) {
      str[0] = str[0] + "0";
    }
    if (com == "MSC") {
      if (tvar[1] > 12) {
        str[1] = String(tvar[1] - 12) + ":";
        Serial.println("meridiem.txt=" + String('"') + "PM" + String('"'));
        Serial3.print("meridiem.txt=" + String('"') + "PM" + String('"'));
      }
      else {
        str[1] = str[0] + String(tvar[1]) + ":";
        Serial.println("meridiem.txt=" + String('"') + "AM" + String('"'));
        Serial3.print("meridiem.txt=" + String('"') + "AM" + String('"'));
      }
      Serial3.write(0Xff);
      Serial3.write(0Xff);
      Serial3.write(0Xff);
    }
    str[0] = str[0] + String(tvar[1]) + ":";
    if (tvar[0] < 10) {
      str[0] = str[0] + "0";
      str[1] = str[1] + "0";
    }
    str[0] = str[0] + String(tvar[0]);
    str[1] = str[1] + String(tvar[0]);
    Serial.println("v String v");
    Serial.println("time.txt=" + String('"') + str[0] + String('"'));
    Serial3.print("time.txt=" + String('"') + str[0] + String('"'));
    Serial3.write(0Xff);
    Serial3.write(0Xff);
    Serial3.write(0Xff);
    if (com == "MSC") {
      Serial.println("clock.txt=" + String('"') + str[1] + String('"'));
      Serial3.print("clock.txt=" + String('"') + str[1] + String('"'));
      Serial3.write(0Xff);
      Serial3.write(0Xff);
      Serial3.write(0Xff);
    }
    ////DATE///
    tvar[0] = (bcdToDec(Wire.read()));
    tvar[1] = (bcdToDec(Wire.read()));
    tvar[2] = (bcdToDec(Wire.read()));
    tvar[3] = (bcdToDec(Wire.read()));
    switch (tvar[0]) {
      case 1:
        str[0] = "Mon";
        str[1] = "Monday";
        break;
      case 2:
        str[0] = "Tue";
        str[1] = "Tuesday";
        break;
      case 3:
        str[0] = "Wed";
        str[1] = "Wednesday";
        break;
      case 4:
        str[0] = "Thu";
        str[1] = "Thursday";
        break;
      case 5:
        str[0] = "Fri";
        str[1] = "Friday";
        break;
      case 6:
        str[0] = "Sat";
        str[1] = "Saturday";
        break;
      case 7:
        str[0] = "Sun";
        str[1] = "Sunday";
        break;
    }
    Serial3.print("date.txt=" + String('"') + str[0] + " " + String(tvar[1]) + "/" + String(tvar[2]) + "/" + String(tvar[3]) + String('"'));
    Serial.println("date.txt=" + String('"') + str[0] + " " + String(tvar[1]) + "/" + String(tvar[2]) + "/" + String(tvar[3]) + String('"'));
    Serial3.write(0Xff);
    Serial3.write(0Xff);
    Serial3.write(0Xff);
    if (com == "MSC") {
      switch (tvar[2]) {
        case 1:
          str[2] = "January";
          break;
        case 2:
          str[2] = "February";
          break;
        case 3:
          str[2] = "March";
          break;
        case 4:
          str[2] = "April";
          break;
        case 5:
          str[2] = "May";
          break;
        case 6:
          str[2] = "June";
          break;
        case 7:
          str[2] = "July";
          break;
        case 8:
          str[2] = "August";
          break;
        case 9:
          str[2] = "September";
          break;
        case 10:
          str[2] = "October";
          break;
        case 11:
          str[2] = "November";
          break;
        case 12:
          str[2] = "December";
          break;
      }
      Serial3.print("clockdate.txt=" + String('"') + str[1] + ", " + str[2] + " " + String(tvar[1]) + ", " + String(tvar[2]) + ", 20" + String(tvar[3]) + String('"'));
      Serial.println("clockdate.txt=" + String('"') + str[1] + ", " + str[2] + " " + String(tvar[1]) + ", " + String(tvar[2]) + ", 20" + String(tvar[3]) + String('"'));
      Serial3.write(0Xff);
      Serial3.write(0Xff);
      Serial3.write(0Xff);
      ///GET TEMP//
      Wire.beginTransmission(0x68);
      Wire.write(0x11);
      Wire.endTransmission();
      Wire.requestFrom(0x68, 2);
      tvar[0] =  Wire.read();
      tvar[0] = (tvar[0] & B01111111); //do 2's math on Tmsb
      tvar[1] =  Wire.read();
      tvar[0] += ( (tvar[1] >> 6) * 0.25 ); //only care about bits 7 & 8
      Serial.println("temp.txt=" + String('"') + String(float(tvar[0])));
      Serial.write(176);    // degree symbol
      Serial.print("c" + String('"'));
      Serial3.print("temp.txt=" + String('"') + String(float(tvar[0])).substring(0, 2));
      Serial3.write(176);    // degree symbol
      Serial3.print("c" + String('"'));
      Serial3.write(0Xff);
      Serial3.write(0Xff);
      Serial3.write(0Xff);
    }
    return (String(" "));
  }
  if (com == "STM") {//Set Streaming Mode || spar[1]=Enable/Disable

    if (spar[0].toInt() == 0) {
      for (int i = 0; i <= 3; i++) {
        Oscilloscope[i] = 0;
      }
      for (int i = 0; i <= 1; i++) {
        for (int x = 0; x <= 3; x++) {
          Voltmeter[i] = x;
        }
      }
      AnalogStream = 0;
      Serial.println("STREAMING OFF");
    }
    else {
      AnalogStream = 1;
      Serial.println("STREAMING ON");
    }
  }
  if (com == "OSC") {//Set Oscilloscope Streaming || spar[1]=Analog Input || spar[2]=Enable/Disable
    Oscilloscope[spar[0].toInt()] = spar[1].toInt();
  }
  if (com == "VMT") {//Set Voltmeter Streaming || spar[1]=Channel || spar[2]=Analog input
    Voltmeter[spar[0].toInt()] = spar[1].toInt();
  }
  if (com == "IRT") {}
  if (com == "BLE") {}
  if (com == "IIC") {}
  if (com == "WEV") {}
  if (com == "WSE") {}
  if (com == "WMI") {}
  if (com == "WHR") {}
  if (com == "WMO") {}
  if (com == "WYR") {}
  if (com == "SDF") {}
  if (com == "SDG") {}
  //Variables commands
  if (com == "") {
    digitalWrite(40, spar[0].toInt());
    return (" ");
  }
    if (com == "APP") {
    SD_APP();
    return (" ");
  }
  return (spar[0]);
}


void sleepNow()         // here we put the arduino to sleep
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here
  sleep_enable();
  attachInterrupt(1, wakeUpNow, LOW);
  sleep_mode();
  sleep_disable();
  detachInterrupt(1);
}

void wakeUpNow()        // here the interrupt is handled after wakeup
{
  if (Watchdog == 1) {
    Serial3.print("FLAG:0x02");
    delay(500);

    if (digitalRead(3) == 0) {
      ADCSRA |= (1 << 7);
      wdt_disable();
      while (digitalRead(3) == 0) {}
      Serial.println("AWAKE!");
      delay(500);
      digitalWrite(32, HIGH);
      Watchdog = 0;
    }
  }
}


//ISR(WDT_vect) {
}
