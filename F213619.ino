#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

enum States {Synchronisation, Main}; //custom enum used in the the loop of the program
enum States currentState = Synchronisation; //custom enum used in the loop of the program


//Synchronisation state functions

void isNewLine(String message){
  //checks if message ends with a newLine
  char lastChar = message.charAt(message.length()-1);
  if (lastChar == '\n') {
    Serial.println(F("\nERROR : no 'newline' characters are accepted"));
  }
}


void isCarriageReturn(String message){
  //checks if message ends with a carriageReturn
  char lastChar = message.charAt(message.length()-1);
  if (lastChar == '\r') {
    Serial.println(F("\nERROR : no 'carriageReturn' characters are accepted"));
  }
}


//Main state functions

enum deviceType {S,O,L,T,C}; //custom enum used in the declaration of the Device class
enum deviceState {ON, OFF}; //custom enum used in the declaration of the Device class

enum MainState {Processing,Processed};  //custom enum used within the main state of the program
enum MainState thisState = Processing;  //custom enum used within the main state of the program

class Device { 
  private :
    String ID; //format : "ABC" - three characters long
    String location; //format : "Location" - can be as many characters as needed, however only 11 will be displayed on the LCD screen
    deviceType type;
    deviceState state;
    int power; //these attributes
    int temperature; //exist independently of one another
  public:
    String getID(){return ID;} //getter
    void setID(String ID){this->ID = ID;} //setter

    String getLocation(){return location;} //getter
    void setLocation(String location){this->location = location;} //setter

    String getType(){ //getter : outputs string rather than custom enum, this helps with displaying the device on the LCD screen
      switch (this->type){
        case S:
          return "S";
          break;
        case O:
          return "O";
          break;
        case L:
          return "L";
          break;
        case T:
          return "T";
          break;
        case C:
          return "C";
          break;
      }
    }
    void setType(deviceType ty){this->type = ty;} //setter

    String getState(){ //getter : outputs string rather than custom enum, this helps with displaying the device on the LCD screen
      switch (this->state){
        case ON:
          return "ON";
          break;
        case OFF:
          return "OFF";
          break;
      }
    }
    void setState(deviceState state){this->state = state;} //setter

    int getPower(){return power;} //getter 
    void setPower(int power){this->power = power;} //setter

    int getTemp(){return temperature;} //getter
    void setTemp(int temp){this->temperature = temp;} //setter

    String displayLine1(){ //returns the output string for the first line of the LCD display
      if (this->location.length()> 11){
        return (this->getID()+" "+this->getLocation().substring(0,11));
      } else {
        return (this->getID()+" "+this->getLocation());
      }
    }
    
    String displayLine2(){ //returns the output string for the second line of the LCD display
      if (this->getType() == "S" || this->getType() == "L"){ //if the device is a Speaker or a Light
        
        String powStr = String(this->getPower());

        //needs to have leading space as the device state is two characters long : 'ON'
        if (this->getState() == "ON") { //device is ON

          //returns custom strings depending on value of power (needs to have leading spaces depending on the value of power) 
          if (this->getPower() < 10){ //power is 1 digit long
            return this->getType()+"  "+this->getState()+"   "+powStr+"%";
          } else if (this->getPower() < 100){ //power is 2 digits long
            return this->getType()+"  "+this->getState()+"  "+powStr+"%";
          } else { //power is 3 digits long
            return this->getType()+"  "+this->getState()+" "+powStr+"%";
          }

        //no need to have leading space as the device state is three characters long : 'OFF'
        } else { //device is OFF
          
          //returns custom strings depending on value of power (needs to have leading spaces depending on the value of power) 
          if (this->getPower() < 10){ //power is 1 digit long
            return this->getType()+" "+this->getState()+"   "+powStr+"%";
          } else if (this->getPower() < 100){ //power is 2 digits long
            return this->getType()+" "+this->getState()+"  "+powStr+"%";
          } else { //power is 3 digits long
            return this->getType()+" "+this->getState()+" "+powStr+"%";
          }

        }

      } else if (this->getType() == "T") { //if the device is a Thermostat
        //
        String tempStr = String(this->getTemp());
        if (this->getState() == "ON") { //device is ON

          if (this->getTemp()<10){ //temperature is 1 digit long
            return this->getType()+"  "+this->getState()+"  "+tempStr+" C";
          } else { //temperature is 2 digits long
            return this->getType()+"  "+this->getState()+" "+tempStr+" C";
          }

        } else { //device is OFF

          if (this->getTemp()<10){ //temperature is 1 digit long
            return this->getType()+" "+this->getState()+"  "+tempStr+" C";
          } else { //temperature is 2 digits long
            return this->getType()+" "+this->getState()+" "+tempStr+" C";
          }

        }

      } else { //if the device is other
        //
        if (this->getState() == "ON") { //device is ON
          return this->getType()+"  "+this->getState();

        } else { //device is OFF
          return this->getType()+" "+this->getState();
        }

      }

    }

};


//initialise an array of 10 Device objects (each object has empty attributes until defined otherwise)
static Device deviceArray[10]; 



void alphabetical_sort() {
  // Bubble sort function for deviceArray to alphabetical order
  for (int i = 0; i < 9; i++) {
    // Last i elements are already sorted
    for (int j = 0; j < 10 - i - 1; j++) {

      if (deviceArray[j].getID() == "" && deviceArray[j + 1].getID() != ""){
        // Swap the elements if a Device with an empty ID is found before one with an ID.
        Device temp = deviceArray[j];
        deviceArray[j] = deviceArray[j+1];
        deviceArray[j+1] = temp;

      } else if (deviceArray[j].getID() != "" && deviceArray[j + 1].getID() != "" && deviceArray[j].getID() > deviceArray[j + 1].getID()) {
        // Swap if both ID's aren't empty and the element found is 'greater' than the next element
        Device temp = deviceArray[j];
        deviceArray[j] = deviceArray[j+1];
        deviceArray[j+1] = temp;

      }
    }
  }
}


bool addDevice(char type, String ID, String location){
  /*
    This function takes three inputs corresponding to a device's type, its ID and its location
    and creates a new object using the given values, and adds it to the deviceArray (if there is enough space)
    before sorting it using the above alphabetical sort function (this is to ensure deviceArray is always sorted correctly).
    If it is unable to add the device to deviceArray, it returns a false boolean value, otherwise it returns true
    (This will help with returning a custom error / validation message to the user when executed).
  */
  Device newDevice;

  if (type == 'S'){ //speaker
    enum deviceType stype = S;
    enum deviceState sstate = OFF;
    
    newDevice.setID(ID);
    newDevice.setLocation(location);
    newDevice.setType(stype);
    newDevice.setState(sstate);
    newDevice.setPower(0);

  } else if (type == 'O'){ //socket
    enum deviceType stype = O;
    enum deviceState sstate = OFF;
   
    newDevice.setID(ID);
    newDevice.setLocation(location);
    newDevice.setType(stype);
    newDevice.setState(sstate);

  } else if (type == 'L'){ //light
    enum deviceType ltype = L;
    enum deviceState lstate = OFF;
    
    newDevice.setID(ID);
    newDevice.setLocation(location);
    newDevice.setType(ltype);
    newDevice.setState(lstate);
    newDevice.setPower(0);
    
  } else if (type == 'T'){ //thermostat
    enum deviceType ttype = T;
    enum deviceState tstate = OFF;
    
    newDevice.setID(ID);
    newDevice.setLocation(location);
    newDevice.setType(ttype);
    newDevice.setState(tstate);
    newDevice.setTemp(0);
    
  } else if (type == 'C'){ //camera
    enum deviceType ctype = C;
    enum deviceState cstate = OFF;
    
    newDevice.setID(ID);
    newDevice.setLocation(location);
    newDevice.setType(ctype);
    newDevice.setState(cstate);

  } else {
    return false;
  }

  //adding to deviceArray

  for(int i=0; i<10; i++){
    if(deviceArray[i].getID() == ""){ 
      //object is empty : we do not empty all of its attributes but for the sake of identifying an 'unused' object we set its id to "".
      deviceArray[i] = newDevice;
      alphabetical_sort();
      return true;

    } else if (deviceArray[i].getID() == newDevice.getID()) { 
      //corresponds to same object : overwrite the data in deviceArray 
      deviceArray[i] = newDevice;
      alphabetical_sort();
      return true;

    }
  }
  //case where 10 distinct objects exist and user wants to create a new one : error - limit to 10 & save memory!
  return false;

}


void stateChange(String ID, String stateStr){
  /*
    This function takes a device ID and a state string (either "ON" or "OFF"), 
    and cycles through the deviceArray, changing the state of the device with the given ID to the new state,
    returning a validation message
  */
  bool found = false;
  for(int i=0; i<10; i++){
    if(deviceArray[i].getID() == ID){
      if (stateStr == "ON"){
        enum deviceState state = ON;
        deviceArray[i].setState(state);
        Serial.println("\nDevice "+ID+" : state successfully changed to ON");
        found = true;

      } else if (stateStr == "OFF") {
        enum deviceState state = ON;
        deviceArray[i].setState(state);
        Serial.println("\nDevice "+ID+" : state successfully changed to OFF");
        found = true;

      } else {
        Serial.println("\nERROR : S-"+ID+"-"+stateStr);
        Serial.println(F("Error! State input is not valid!\nPlease try again using either 'ON' or 'OFF'."));
        found = true;
      }
    }
  
  }
  if (found == false){
    Serial.println("\nERROR : S-"+ID+"-"+stateStr);
    Serial.println(F("Error! The device ID you have entered does not exist! Try again with one that does!"));
  }
  
}

void power_tempChange(String ID, int pow_temp){
  /*
    This function takes a device ID and a power or temperature string, 
    and cycles through the deviceArray, changing the power/temp of the device with the given ID to the new pow/state,
    returning a validation message
  */
  bool found = false;
  for(int i=0; i<10; i++){
    if(deviceArray[i].getID() == ID){
      found = true;

      String pow_tempStr = String(pow_temp);
      if (deviceArray[i].getType() == "S" || deviceArray[i].getType() == "L"){
        //case where device is a Speaker or a Light

        if (pow_temp < 0 || pow_temp > 100){
          Serial.println("\nERROR : P-"+ID+"-"+pow_temp);
          Serial.println(F("Error : power value must be between 0 and 100."));
        } else {
          deviceArray[i].setPower(pow_temp);
          Serial.println("\nDevice "+ID+" : power successfully changed to "+pow_tempStr+"%");
        }
        
      } else if (deviceArray[i].getType() == "T"){
        //case where device is a thermostat

        if (pow_temp < 9 || pow_temp > 32){
          Serial.println("\nERROR : P-"+ID+"-"+pow_temp);
          Serial.println(F("Error : temperature value must be between 9 and 32."));
        } else {
          deviceArray[i].setTemp(pow_temp);
          Serial.println("\nDevice "+ID+" : temperature successfully changed to "+pow_tempStr+"°C");
        }

      } else {
        //case where deviceID is valid but the device type is not correct.
        Serial.println("\nERROR : P-"+ID+"-"+pow_temp);
        Serial.println("Device "+ID+" is not a light or a speaker : you cannot change its power value!");
      }
      
    }
  }
  if (found == false){
    Serial.println("\nERROR : P-"+ID+"-"+pow_temp);
    Serial.println(F("Error! The device ID you have entered does not exist! Try again with one that does!"));
  }

}

void removeDevice(String ID){
  /*
    This function takes a device ID as input and cycles through the deviceArray,
    "deleting" the corresponding device by setting its ID value to the empty string
    (this works because the creation of a new device replaces the values of any existing device with no ID value).
    It then returns a validation message
  */
  bool found = false;
  for(int i=0; i<10; i++){
    if(deviceArray[i].getID() == ID){
      found = true;
      Serial.println("Device "+ID+" : successfully deleted.");
      deviceArray[i].setID("");
      alphabetical_sort();
    }
  }
  if (found == false){
    Serial.println("\nERROR : R-"+ID);
    Serial.println(F("Error! The device ID you have entered does not exist! Try again with one that does!"));
  }
}

//

String alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

bool isCorrectID(String ID){
  bool iscorrect = false;
  for (int i=0; i<ID.length(); i++){
    for (int j=0; j<alphabet.length();j++){
      if (ID.charAt(i) == alphabet.charAt(j)){
        iscorrect = true;
      }
    }
    if (!iscorrect){
      return false;
    } else {
      iscorrect = false;
    }
  }
  return true;
}


String alphanumeric = alphabet+"abcdefghijklmnopqrstuvwxyz0123456789";

bool isCorrectLocation(String location){
  bool iscorrect = false;
  for (int i=0; i<location.length(); i++){
    for (int j=0; j<alphanumeric.length();j++){
      if (location.charAt(i) == alphanumeric.charAt(j)){
        iscorrect = true;
      }
    }
    if (!iscorrect){
      return false;
    } else {
      iscorrect = false;
    }
  }
  return true;
}

//


void messageHandler(String message){
  /*  
    This function takes as input a string corresponding to a command entered into the Serial Monitor by the user,
    it then checks the first character of the string to decifer which action to carry out. If the message does not 
    conform to the protocol, it outputs an error message.
  */
  if (message.charAt(1) != '-'){ //ensures the second character is a hyphen 
    Serial.println("\nERROR : "+message+"\nmessage does not represent known format");
    
  } else if (message.charAt(0) == 'A'){ //the first character is an 'A' : add a device

    //ensures the message is of the correct format : A-ABC-LOCATIONSTR
    if (message.charAt(5) != '-' || message.charAt(7) != '-'){ 
      Serial.println("\nERROR : "+message+"\nmessage does not represent known format");
    } else {

      char type = message.charAt(6); //create error handling for type : (i.e. S, O, L, T, C) for Speaker, Socket, Light, Thermostat or Camera.
      String ID = message.substring(2,5);
      String location = message.substring(8);

      if (location == ""){ 
        //checks that user has inputted a location string and not left it empty - if not then error message
        Serial.println("\nERROR : "+message+"\nmessage does not represent known format");
      } else if (!isCorrectLocation(location)){
        //checks that user has inputted a correct location string : must only be characters A-Z and 0-9 included - otherwise error message
        Serial.println("\nERROR : "+message+"\nmessage does not represent known format");
      } else {  
        if (!isCorrectID(ID)){
          //checks that user has inputted a correct ID string : must only be characters A-Z included - otherwise error message
          Serial.println("\nERROR : "+message+"\nmessage does not represent known format");
        } else {
          bool worked = addDevice(type,ID,location); //uses addDevice function to add a new device
          if (worked == true){ //if the operation was successful, output a validation message, otherwise send an error message
            Serial.println(F("New device has successfully been added!"));
          } else {
            Serial.println("\nERROR : "+message);
            Serial.println(F("The device type you have entered does not exist \nOR - there's not enough space for another device. You must delete a device before adding a new one!"));
          }
        }
        
      }
      
    }

  } else if (message.charAt(0) == 'S'){ //the first character is an 'S' : changes a device's state

    if (message.charAt(5) != '-'){
      //ensures the message is of the correct format : S-ABC-ON/OFF
      Serial.println("\nERROR : "+message+"\nmessage does not represent known format");
    } else {
      String ID = message.substring(2,5);
      String state = message.substring(6);
      
      stateChange(ID,state);
    }

  } else if (message.charAt(0) == 'P'){ //the first character is a 'P' : changes a device's power

    if (message.charAt(5) != '-'){
      //ensures the message is of the correct format : P-ABC-100
      Serial.println("\nERROR : "+message+"\nmessage does not represent known format");
    } else {
      String ID = message.substring(2,5);
      int val = message.substring(6).toInt();

      power_tempChange(ID,val);
    }
    
  } else if (message.charAt(0) == 'R'){ //the first character is an 'R' : removes a device from the deviceArray
    
    String ID = message.substring(2,5);
    removeDevice(ID);
  } else {
    //case where the first character of the message does not correspond to A, S, P or R - returns error message
    Serial.println("\nERROR : "+message+"\nmessage does not represent known format");
  }
}


//LCD display functions

int pointer = 0; //initialises a global variable corresponding to the index of the device we wish to print on the LCD screen
#define YELLOW 3 //definition of colours to be used on LCD screen : makes code more readable
#define GREEN 2
#define VIOLET 0x5

//creation of the custom chars (UDCHARS extension)
byte upwardsArrow[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00000
};

byte downwardsArrow[8] = {
  0b00000,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b11111,
  0b01110,
  0b00100
};


void showCurrentDevice(){
  /*
    Prints the relevant details of the current device on the LCD screen,
    and sets the backlight to the corresponding colour.
  */


  lcd.clear();
  if (deviceArray[pointer].getID()==""){
    //case where pointer is pointing at an empty device in deviceArray
  } else if (pointer == 0){
    //if the pointer is pointing at the first device of deviceArray
    if (deviceArray[pointer+1].getID()==""){
      //case where the second device is empty : we do not display a downwards arrow
      lcd.setCursor(0, 0);
      lcd.print(" "+deviceArray[pointer].displayLine1());
      lcd.setCursor(0, 1);
      lcd.print(" "+deviceArray[pointer].displayLine2());
    } else {
      //case where the second device is not empty : we display a downwards arrow
      lcd.setCursor(0, 0);
      lcd.print(" "+deviceArray[pointer].displayLine1());
      lcd.setCursor(0, 1);
      lcd.write((byte)1);
      lcd.setCursor(1, 1);
      lcd.print(deviceArray[pointer].displayLine2());
    }
    

  } else if (pointer == 9){
    //case where the pointer is set to the last device : we display an upwards arrow but no downwards arrow
    lcd.setCursor(0, 0);
    lcd.write((byte)0);
    lcd.setCursor(1, 0);
    lcd.print(deviceArray[pointer].displayLine1());
    lcd.setCursor(0, 1);
    lcd.print(" "+deviceArray[pointer].displayLine2());

  } else {
    //case where the pointer is comprised between 1 and 8.
    if (deviceArray[pointer+1].getID()==""){
      //the following device is empty : we display an upwards arrow but no downwards arrow
      lcd.setCursor(0, 0);
      lcd.write((byte)0);
      lcd.setCursor(1, 0);
      lcd.print(deviceArray[pointer].displayLine1());
      lcd.setCursor(0, 1);
      lcd.print(" "+deviceArray[pointer].displayLine2());
    } else {
      //the following device is not empty : we display an upwards arrow and a downwards arrow
      lcd.setCursor(0, 0);
      lcd.write((byte)0);
      lcd.setCursor(1, 0);
      lcd.print(deviceArray[pointer].displayLine1());
      lcd.setCursor(0, 1);
      lcd.write((byte)1);
      lcd.setCursor(1, 1);
      lcd.print(deviceArray[pointer].displayLine2());
    }
    
  }

  //printing degree symbol if device is a Thermostat (regular degree symbol creates error on lcd screen)
  if (deviceArray[pointer].getType() == "T"){ 
    lcd.setCursor(9, 1);
    lcd.write((char)223);
  }
  
  if (deviceArray[pointer].getState()=="ON"){
    lcd.setBacklight(GREEN); //set backlight to green
  } else if (deviceArray[pointer].getState() == "OFF"){
    lcd.setBacklight(YELLOW); //set backlight to yellow
  }

}


enum LCDStates {ShowDevices, SelectButtonHeld};
enum LCDStates currentLCDState = ShowDevices;

bool stateChanged = false;
unsigned long DELAY = 1000; //set delay time to 1 second


// malloc functions : FREERAM addon

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
  #ifdef __arm__
    return &top - reinterpret_cast<char*>(sbrk(0));
  #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
  #else  // __arm__
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
  #endif  // __arm__
}

int i=0;
void showAvailableMemory(){
  byte *ptr = (byte*)malloc(i);
  if (ptr == NULL){
    //return false;
  } else {
    lcd.setCursor(0, 1);
    lcd.print(F("FREERAM : "));
    lcd.setCursor(10, 1);
    lcd.print(freeMemory());
    //Serial.println(freeMemory());
    free(ptr);
    //return true;
  }
  i++;
}

//


void ButtonHandler() {
  /*
    This function checks if any buttons have been pressed
    and changes the pointer variable accordingly (if the up button is pressed,
    the pointer decrements by one unless it is already at zero and similarly 
    increments when the down button is pressed, unless the pointer is at nine).

    This function is designed to be called within a loop to insure that the button 
    inputs are regularly being monitored and the user won't have to wait long for 
    their action to be processed.
  */

  //run state

  switch (currentLCDState){
    case 0: // showdevices state
      {
        if (stateChanged){
          showCurrentDevice();
          stateChanged = false;
        }

        break;
      }
    case 1: // select button hold state
      {
        if (stateChanged){
          lcd.clear();
          lcd.setBacklight(VIOLET);
          lcd.print(F("F213619"));
          showAvailableMemory();
          stateChanged = false;
        }
        
        break;
      }

  }

  //check buttons
  
  uint8_t buttons = lcd.readButtons();

  if (buttons & BUTTON_UP) {
    //if the up button is pressed
    if (pointer > 0){ 
      //we check the value of the pointer and show the current Device
      pointer = pointer - 1;
    } 
    //if the display state is in SelectButtonHeld - change display state to showDevices
    if (currentLCDState == SelectButtonHeld){
      currentLCDState = ShowDevices;
      stateChanged = true;
    
    //if the display state is in ShowDevices - just show the current device
    } else if (currentLCDState == ShowDevices) {
      delay(1000);
      showCurrentDevice();
      
    }
    
  }

  if (buttons & BUTTON_DOWN) {

    if (pointer < 9) {
      //we check the value of the pointer and show the current Device
      if (deviceArray[pointer+1].getID()!=""){ 
        //checks if following device isn't empty : we do not increment pointer if the corresponding device is empty
        pointer = pointer + 1;
      }
    }

    //if the display state is in SelectButtonHeld - change display state to showDevices
    if (currentLCDState == SelectButtonHeld){
      currentLCDState = ShowDevices;
      stateChanged = true;

    //if the display state is in ShowDevices - just show the current device
    } else if (currentLCDState == ShowDevices) {
      delay(1000);
      showCurrentDevice();
      
    }
  }
  
  
  //if select button is pressed, change the state to SelectButtonHeld
  if ((buttons & BUTTON_SELECT) && (currentLCDState == ShowDevices)){
    //waits one second before changing the state to SelectButtonHeld and setting stateChanged to true
    unsigned long fromPressed = millis();
    delay(DELAY);
    if (fromPressed > DELAY){
      stateChanged = true;
      currentLCDState = SelectButtonHeld;
    }

  }

  //if the button is released, the state is changed back to ShowDevices
  if (!(buttons & BUTTON_SELECT) && (currentLCDState == SelectButtonHeld)){
    currentLCDState = ShowDevices;
    stateChanged = true;
  }
  
}


void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.createChar(0,upwardsArrow);
  lcd.createChar(1,downwardsArrow);
  lcd.clear();
}


void loop() {

  switch (currentState) {
    
    case 0:
      {
        lcd.setBacklight(VIOLET);
        while (Serial.available() == 0) {
          Serial.print(F("Q"));
          delay(1000);
        }

        String T = Serial.readString();
        isNewLine(T);
        isCarriageReturn(T);

        T.trim();
        T.toUpperCase();

        if (T == "X"){
           currentState = Main; 
           Serial.println(F("\UDCHARS FREERAM"));
           lcd.clear();
           lcd.setBacklight(7);
        }
        break;
      }
    case 1:
      {
        ButtonHandler();

        if (thisState == Processing) {

          if (Serial.available()){
            String message = Serial.readString();
            message.trim();
            messageHandler(message);

            thisState = Processed;
          }
        }
        
        if (thisState == Processed) {
          //to be executed once every time a message is processed :
          if (deviceArray[0].getID() != "" && currentLCDState != SelectButtonHeld){ 
            //when the user adds the first device it shows on the lcd screen immediately, unless the select button is held
            showCurrentDevice();
          }

          thisState = Processing;
        }

        /*
        while (Serial.available() == 0) {
          showDeviceLoop();
        }
        
        String message = Serial.readString();
        message.trim();
        messageHandler(message);
        */

        break;
      }

  }
  
}