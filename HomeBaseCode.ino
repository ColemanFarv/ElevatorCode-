/******************************************************************************
FILENAME:     HomeBaseCode.ino
COURSE:       MREN 178
PROJECT:      Elevatron
STUDENTS:     Quinton Rodgers 20328951, Coleman Farvolden 20335398


DATE:         April 3rd, 2023

ATTRIBUTIONS: https://www.geeksforgeeks.org/bubble-sort/ 
              https://howtomechatronics.com/tutorials/arduino/arduino-and-hc-12-long-range-wireless-communication-module/
******************************************************************************/
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const int pinRS = 8;  //lcd
const int pinEN = 9;
const int pinD4 = 4;
const int pinD5 = 5;
const int pinD6 = 6;
const int pinD7 = 7;
const int setPin = 2;  //HC12

char myID = 'd';
int elevator = 0;
const int NumElevators = 3;  //This can be changed but the if check on line 90 needs to be lengthened
const int totalFloors = 10;  //more than 10 floors will crash the arduino memory

typedef struct {  //struct for intaking info
  bool validInfo = true;
  char ownID = '\0';
  int floor = -1;
  char targetID = '\0';
} dataInfo;

//This is a struct for the elevator
struct node {     //struct for elevator
  int floor = 0;  //floor its currently on
  int state = 0;  //Is elevator currently going up or down or stationary. 0 is stationary, 1 is up and -1 is down
  int open = 0;   //1 elevator is open 0 is closed
  int counter = -1; //This is the counter of the floors going to in the array
  int floorOrder[10]; //This is the amount of floor destinations that the elevator can store
} * **Building;

dataInfo recievedData;

char bufferIn[64];  //incoming and outgoing buffer and counters
char bufferOut[64];
int currentPlaceIn = 0;
int currentPlaceOut = 0;

//init peripherals used
SoftwareSerial HC12(10, 11);
LiquidCrystal lcd(pinRS, pinEN, pinD4, pinD5, pinD6, pinD7);

void setup() {
  HC12.begin(9600);
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  digitalWrite(setPin, HIGH);     //set HC12 to usable mode
  while (analogRead(A0) > 800) {  //wait for user to begin
    lcd.setCursor(0, 0);
    lcd.print("Press button");
    delay(75);
  }
  Building = createSystem(NumElevators, totalFloors);  //create elevator
  delay(400);
}

void loop() {
  for (; HC12.available(); ((currentPlaceIn++) % 64)) {  //read in data from HC12
    bufferIn[currentPlaceIn] = HC12.read();
    Serial.write(bufferIn[currentPlaceIn]);
    delay(5);
  }
  recievedData = getInfo();                                       //transform info
  if (recievedData.targetID == myID && recievedData.validInfo) {  //check if data is for us
    if (recievedData.ownID == 'a') {
      elevator = 0;
    } else if (recievedData.ownID == 'b') {
      elevator = 1;
    } else {
      elevator = 2;
    }
    gotoFloor(Building, recievedData.floor, elevator);  //add floor to that elevators queue
  }
  if (analogRead(A0) < 800 && analogRead(A0) >= 600) {  //call an elevator to a floor
    manualSelect();
  }
  PrintLCD();                                             //print out LCD
  MoveAll(Building, totalFloors, NumElevators);           //move all elevators
  createMessage('a', findElevatorRow(Building, 0), 'd');  //send message to a
  createMessage('b', findElevatorRow(Building, 1), 'd');  //send message to b
  createMessage('c', findElevatorRow(Building, 2), 'd');  //send message to c
  delay(2000);                                            //travel time
}

void manualSelect() {  //gets a floor to add into elevator queue

  lcd.clear();
  int floor = 0;
  int x = analogRead(A0);
  lcd.setCursor(0, 0);
  lcd.print("Select Floor");
  lcd.setCursor(0, 1);
  lcd.print(": ");
  delay(500);
  //loop until select is pressed
  while (true) {
    x = analogRead(A0);
    //loop 1-9 with overflow
    if (x > 60 && x < 200) {  //up
      floor += 1;
      floor %= totalFloors;
    } else if (x > 200 && x < 400) {  //down
      floor -= 1;
      floor += totalFloors;
      floor %= totalFloors;
    }
    if (floor == 0) {  //0 is not accepted floor
      floor = 1;
    }
    //print current selection
    lcd.setCursor(8, 1);
    lcd.print("  ");
    lcd.setCursor(8, 1);
    lcd.print(floor);
    delay(100);
    //call the function to add it to queue
    if (x < 800 && x > 600) {
      pickupAtFloor(Building, floor, totalFloors, NumElevators);
      break;  //exit while loop
    }
  }
  //display the confirmation for 1 second
  delay(100);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("to floor ");
  lcd.print(floor);
  delay(1000);
}


void PrintLCD() {
  //generic home screen
  //prints E: A   B    C
  //       F: #   #    #
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("E:");
  lcd.setCursor(0, 1);
  lcd.print("F:");
  lcd.setCursor(3, 0);
  lcd.print("A");
  lcd.setCursor(8, 0);
  lcd.print("B");
  lcd.setCursor(13, 0);
  lcd.print("C");
  lcd.setCursor(3, 1);
  lcd.print(findElevatorRow(Building, 0));
  lcd.setCursor(8, 1);
  lcd.print(findElevatorRow(Building, 1));
  lcd.setCursor(13, 1);
  lcd.print(findElevatorRow(Building, 2));
}

//creates the message package to be sent by arduino
void createMessage(char targetID, int floor, char ownID) {
  char floorTens;
  char floorUnits;
  floorTens = floor / 10 + '0';
  floorUnits = floor % 10 + '0';
  //creates a 16 char string to be sent
  String message = "0000000000000000";
  int count = 0;
  //makes message in format zzzaaa1010bbbyyy
  //z is check
  //a is target elevator
  //10 is the floor
  //b is sender
  //y is check
  for (int i = 0; i < 3; i++) {
    message[count++] = 'z';
  }
  for (int i = 0; i < 3; i++) {
    message[count++] = targetID;
  }
  for (int i = 0; i < 2; i++) {
    message[count++] = floorTens;
    message[count++] = floorUnits;
  }
  for (int i = 0; i < 3; i++) {
    message[count++] = ownID;
  }
  for (int i = 0; i < 3; i++) {
    message[count++] = 'y';
  }
  HC12.print(message);
}

dataInfo getInfo() {
  dataInfo receivedInfo;
  int startPosition = 0;
  int currentPosition = 0;
  int endPosition = 0;
  char temp;
  char receivedString[20];  //includes 4 characters extra in case extra garbage is received
  for (int i = 0; i < 64; i++) {
    //expected string will be in the form zzzaaa1111bbbyyy (target,floor,own) (16 characters)
    currentPosition = i;
    //check if the current position is a z, signalling the potential start of a message
    if (bufferIn[currentPosition] == 'z') {
      //Serial.println("Found start position at: " + String(currentPosition));
      break;
    }
  }
  //find start position.
  if (check2of3(currentPosition, bufferIn) == 'z') {
    //check if the current position in the buffer is z, if so it is the start, otherwise the next position must be start
    startPosition = currentPosition;
  } else {
    if (currentPosition != 63) {  //remove random z
      //bufferIn[currentPosition] = '\0';
      //Serial.println("CANNOT FIND 3 Zs in a row");
    }

    receivedInfo.validInfo = false;
    return receivedInfo;
  }
  endPosition = startPosition + 2;  //start checking 2 positions past the first z in the array
  //find the end position (next set of zzz, but will only check for two)
  while (check2of3(endPosition % 64, bufferIn) != 'y') {  //check2of3 checks the character at the position, then next two positions
    endPosition++;
    //make sure we haven't checked more than 20 characters(i.e. cannot find end of message)
    if (endPosition - startPosition > 19) {
      //Serial.println("Failed to find end of the message");
      receivedInfo.validInfo = false;
      return receivedInfo;
    }
  }
  //check if the character 3 positions later is a y (i.e. check2of3 found two y's at endPosition +1 and +2)
  if (bufferIn[(endPosition + 3) % 64] == 'y') {
    endPosition = endPosition + 3;
  } else if (bufferIn[(endPosition + 2) % 64] == 'y') {  //find furthest y in buffer
    endPosition = endPosition + 2;
  } else {
    endPosition = endPosition + 1;
  }
  //transfer received string to the received input array
  for (int j = startPosition; j <= endPosition; j++) {
    receivedString[j - startPosition] = bufferIn[j % 64];
    //remove the data from the buffer
    bufferIn[j % 64] = '\0';
  }
  currentPlaceIn = startPosition;
  //add null character at end of received data
  receivedString[(endPosition - startPosition) + 1] = '\0';
  Serial.println("Input received is: " + String(receivedString));

  //update the current position variable to hold the first position to check in the received string
  currentPosition = 2;  //start at 3rd position which could be an z, but also could be ID depending on errors
  //find the sender of info starting two characters after start position
  temp = check2of3(currentPosition, receivedString);
  if (validID(temp)) {
    receivedInfo.targetID = temp;
  } else {
    //check next set of 3 characters
    currentPosition++;
    temp = check2of3(currentPosition, receivedString);
    if (validID(temp)) {
      receivedInfo.targetID = temp;
    } else {
      //failed to find a valid target ID
      //Serial.println("Unknown Sender");
      receivedInfo.validInfo = false;
      return receivedInfo;
    }
  }
  //Serial.println("target is: " + String(receivedInfo.targetID));
  //search till a number is found
  while (receivedString[currentPosition] < '0' || receivedString[currentPosition] > '9') {
    currentPosition++;
    if (currentPosition > 15) {  //last position where there could still be two chars each for sender and end of message
      //Serial.println("Unable to find a valid floor number");
      receivedInfo.validInfo = false;
      return receivedInfo;
    }
  }
  //we need all 4 digits to be present, and first two match the second two, if they aren't, assume there is an error
  //check to confirm next digit is also valid
  if (receivedString[currentPosition + 1] >= '0' && receivedString[currentPosition + 1] <= '9') {
    //update floor, first number is 10s digit, second is 1s digit
    receivedInfo.floor = ((receivedString[currentPosition] - '0') * 10) + (receivedString[currentPosition + 1] - '0');
  } else {
    //Serial.println("error in floor data received");
    receivedInfo.validInfo = false;
    return receivedInfo;
  }
  //check to ensure the next for numbers are also valid
  if ((((receivedString[currentPosition + 2] - '0') * 10) + (receivedString[currentPosition + 3] - '0')) != receivedInfo.floor) {
    //Serial.println("numbers don't match up");
    receivedInfo.validInfo = false;
    return receivedInfo;
  }
  //Serial.println("Floor number is: " + String(receivedInfo.floor));
  //update the current position to be after the floors
  currentPosition = currentPosition + 4;
  temp = check2of3(currentPosition, receivedString);
  if (validID(temp)) {
    receivedInfo.ownID = temp;
  } else {
    //failed to find a valid target ID
    //Serial.println("Unknown Sender");
    receivedInfo.validInfo = false;
    return receivedInfo;
  }
  //Serial.println("sender is: " + String(receivedInfo.ownID));
  return receivedInfo;
}
//helper function for check 2 of 3 char function
bool validChar(char value) {
  switch (value) {
    case 'a':
      return true;
    case 'b':
      return true;
    case 'c':
      return true;
    case 'd':
      return true;
    case 'z':
      return true;
    case 'y':
      return true;
    default:
      return false;
  }
}
//function to check if the value is a valid target or own id
bool validID(char value) {
  switch (value) {
    case 'a':
      return true;
    case 'b':
      return true;
    case 'c':
      return true;
    case 'd':
      return true;
    default:
      return false;
  }
}
//function that takes a position and char array, and checks if 2 of the next three chars are valid chars.
//current position needs to be less than input array max length, or input array needs to be of length 64.
char check2of3(int currentPosition, char inputArray[]) {
  if (validChar(inputArray[(currentPosition) % 64])) {
    if (validChar(inputArray[(currentPosition + 1) % 64]) && inputArray[(currentPosition + 1) % 64] == inputArray[(currentPosition) % 64]) {
      return inputArray[currentPosition];
    } else if (validChar(inputArray[(currentPosition + 2) % 64]) && inputArray[(currentPosition + 2) % 64] == inputArray[(currentPosition) % 64]) {
      return inputArray[currentPosition];
    } else {
      return 'x';
    }
  } else if (validChar(inputArray[(currentPosition + 1) % 64])) {
    if (validChar(inputArray[(currentPosition + 2) % 64]) && inputArray[(currentPosition + 2) % 64] == inputArray[(currentPosition + 1) % 64]) {
      return inputArray[(currentPosition + 1) % 64];
    } else {
      return 'x';
    }
  }
  return 'x';
}

//This is a swapping function for the bubble sort
void swap(int* ap, int* bp) {
  int temp = *ap;  //set temp to value of ap
  *ap = *bp;       //set ap's value to bp value
  *bp = temp;      //set bp's value to ap's value
}

//this prints out the array of the elevators floor traveling orders
void heapify(int arr[], int N, int i, int state)  //this does the heapification, needs array of size n and integer of index i
{
  // find whether the left or right child is smaller if going up, or check if left or right node is larger if going down 

  int root = i;  //root is set to the largest, or smallest value depdending if going up or down respectively 

  int left = 2 * i + 1;  //calculate index of left node

  int right = 2 * i + 2;  //calculate index of right node

  if (state == -1) {  //if the elevator is going down make min heap
    // checks if the left node is smaller than the root
    if (left < N && arr[left] < arr[root])
      //set root to the index of left node
      root = left;

    //check if the right node is smaller than the root node
    if (right < N && arr[right] < arr[root])
      root = right;                       //if it is set the smallest index to right node
  } else if (state == 1 || state == 0) {  //if the elevator is going up, make max heap
    if (left < N && arr[left] > arr[root])
      //set root to left index
      root = left;

    //check if the right node is larger than the root
    if (right < N && arr[right] > arr[root])

      root = right;  //if it is set the largest index to right
  }

  //if the left or right node is larger (stage 1 or 0) or smaller(stage -1) swap it with the root
  if (root != i) {

    swap(&arr[i], &arr[root]);     //swap the root with the left or right node
    heapify(arr, N, root, state);  //call the heapify function again until the array is a max heap if stage is 1 or 0, or min heap if stage is -1
  }
}


void heapSort(int arr[], int N, int state) {
  //builds heap
  for (int i = N / 2 - 1; i >= 0; i--)

    heapify(arr, N, i, state);  //called to check for  min heap if stage is -1, otherwise check for max heap property

  //this for loop sorts the heap
  for (int i = N - 1; i >= 0; i--) {

    swap(&arr[0],
         &arr[i]);  //swaps the first element with the last element
    //heapify to ensure that each parent node is larger than its children if stage is 1 or 0
    //heapify to ensure that each parent node is smaller than it's children if stage is -1
    heapify(arr, i, 0, state);
  }
}

//This Function is used when you want to add a floor to the elevator struct array.
// You give the function a floor number and it adds that floor to the array of the elevator. The elevatorwill travel to this floor at somepoint.
int addFloor(struct node*** system, int fn, int array[], int state, int count, int floor, int elevator) {  //needs floor number to bed added, which elevators array, if it is going up, down or not moving, and the counter of elements in the array
  count++;
  int check = 0;
  while (check < count) {              //check to see if the floor has already been pressed
    if (fn == array[check]) return 0;  //if the value already exists return 0;
    check++;                           //add one to check variable
  }
  array[count] = fn;  //put floor number into the array
  (count)++;          //add 1 to the count of e
  heapSort(array, count, state);
  return 0;
}

//This function is given the column/elevator number and finds and returns what row/floor the elevator is currently on
int findElevatorRow(struct node*** system, int elevator) {

  int i = 0; //Initializes floor parser

  while (system[i][elevator] == NULL) { //Keeps checking different rows in column until not Null. Everything other then elevators is NULL
    i++; //increments to searh next floor
  }

  return i; //Returns floor elevator is on
}

//This function reorders floor order array for an elevator when it reaches a desired floor. The first item in the elevators array is always the next floor it is going to
void reshuffle(struct node*** system, int array[], int state, int count, int floor, int elevator) {


  for (int i = 1; i < count + 1; i++) { //Keeps function goes through the whole array in the elevator struct

    array[i - 1] = array[i]; //Deletes first element in the array and shift all elements in the array to the left
  }


  array[count] = -1; //Reduces the array size of the elevator
}

//This function checks if all elevators have no elements in the floor destination array. AKA all elevators have reached their floors
int onrequiredFloors(struct node*** system, int elevators, int floors) {
  for (int i = 0; i < elevators; i++) { //Look over all elevators/columns
    for (int j = 0; j < floors; j++) { //Look over all floors/rows
      if (system[j][i] != NULL) { //Finds elevator
        if (system[j][i]->counter > -1) return 0; //returns 0 if any elevators have not reached desired floors
      }
    }
  }
  return 1; //Returns 1 if all elevators have reached desired floors
}

//this function checks if all elevators are on desired floors

//This function creates the whole elevator system for the building
struct node*** createSystem(int elevators, int floors) {
  
   // allocate memory for the 2d array of elevator struct pointers
  struct node*** SystemElevator = (struct node***)malloc(floors * sizeof(struct node**));

   // allocate memory for each row/floor of struct elevator nodes
  for (int i = 0; i < floors; i++) {
    SystemElevator[i] = (struct node**)malloc(elevators * sizeof(struct node*));
 // allocate memory for each column/elevator of struct elevator nodes    
    for (int j = 0; j < elevators; j++) {
      SystemElevator[i][j] = (struct node*)malloc(sizeof(struct node));
      //Initializes all elevators structs on first floor
      if (i == 0) {
        SystemElevator[i][j]->floor = 0; //Initialize starting floor
        SystemElevator[i][j]->state = 0; //Initialize starting state
        SystemElevator[i][j]->open = 0;  //Initialize starting state of open or closed
        SystemElevator[i][j]->counter = -1;//Initialize floor destination array size
        //Initialize the floor destination array of elevator
        SystemElevator[i][j]->floorOrder[0] = -1;
        SystemElevator[i][j]->floorOrder[1] = -1;
        SystemElevator[i][j]->floorOrder[2] = -1;
        SystemElevator[i][j]->floorOrder[3] = -1;
        SystemElevator[i][j]->floorOrder[4] = -1;
        SystemElevator[i][j]->floorOrder[5] = -1;
        SystemElevator[i][j]->floorOrder[6] = -1;
        SystemElevator[i][j]->floorOrder[7] = -1;
        SystemElevator[i][j]->floorOrder[8] = -1;
        SystemElevator[i][j]->floorOrder[9] = -1;
      } else {
        //set all elevators not on starting floor to be null
        SystemElevator[i][j] = NULL;
      }
    }
  }
//Return the pointer to the 2d pointer struct array
  return SystemElevator;
}

//This moves all elevators towards there next floor destinetion, 
//aka the first element of their array. It moves elevators only by one floor when called
void MoveAll(struct node*** system, int totalFloors, int totalElevators) {

  int i = 0; //Starts i at zero

  while (i < totalElevators) {  //Iterates through all elevators/columns

    int currentFloor = findElevatorRow(system, i); //Finds the floor the elevator is on

    if (system[currentFloor][i] != NULL) { //Checks if found elevator is not a null node

      if (system[currentFloor][i]->counter > -1) { //Check if there is a floor destination

//If the elevators is on a floor below the deired floor. Move the elevator one floor up
        if (system[currentFloor][i]->floor < system[currentFloor][i]->floorOrder[0]) {

          system[currentFloor][i]->state = 1;
          system[currentFloor + 1][i] = system[currentFloor][i];
          system[currentFloor][i] = NULL;
          system[currentFloor + 1][i]->floor += 1;
        } 
  
  //If the elevators is on a floor above the deired floor. Move the elevator one floor down
        else if (system[currentFloor][i]->floor > system[currentFloor][i]->floorOrder[0]) {

          system[currentFloor][i]->state = -1;
          system[currentFloor - 1][i] = system[currentFloor][i];
          system[currentFloor][i] = NULL;
          system[currentFloor - 1][i]->floor -= 1;
        } 
 //If the elevators is on a floor equal to the desired floor. Remove that floor from the floor array list, find next floor in the order
        else if (system[currentFloor][i]->floor == system[currentFloor][i]->floorOrder[0]) {
          //get rid of that element from array

          reshuffle(system, system[currentFloor][i]->floorOrder, system[currentFloor][i]->state, system[currentFloor][i]->counter, currentFloor, i);
          system[currentFloor][i]->counter -= 1;
          system[currentFloor][i]->state = 0;
        }
      }
    }
    i++; //increment to next elevator
  }
}


//THis function takes in a floor and moves the closest elevator already moving in that direction towards said floor.
//Imagine you are on a floor and you want to go to a floor. This function essentially decides which elevator will pick you up. 
//And send this floor order to the elevator.
void pickupAtFloor(struct node*** system, int floorWanted, int totalFloors, int totalElevators) {
  // Calculate the distance of each elevator from the desired floor


  int distances[totalElevators]; //creats a array of distance from elevator to floor that needs a pickup



  for (int ElevatorNumber = 0; ElevatorNumber < totalElevators; ElevatorNumber++) {  //parse through all elevators and find there distance to the floor

    int floors = 0;  //set j int to start at zero

    while (system[floors][ElevatorNumber] == NULL) {  //keep going through rows until index with element is found
      floors++;                                       //increment each pass
    }

    if (floors < floorWanted && system[floors][ElevatorNumber]->state == -1) {  //check if elevator is going down and floor pickup is up, don't send floor direction to this elevator

      distances[ElevatorNumber] = 32767;  //set distance to extrmely large number

    }

    else if (floors > floorWanted && system[floors][ElevatorNumber]->state == 1) {  //check if elevator is going up and floor pickup is down, don't send floor direction to this elevator

      distances[ElevatorNumber] = 32767;  //set distance to extrmely large number

    }

    else {

      distances[ElevatorNumber] = abs(floors - floorWanted);  //find distance of elevator to floor
    }
  }

    // Find the elevator with the shortest distance to the desired floor. 
    //and this elevator will be neutreul state or already moving toward desired floor
  int minDistance = 32767;
  int minElevator = -1;
  for (int j = 0; j < totalElevators; j++) {
    if (distances[j] < minDistance) {
      minDistance = distances[j];
      minElevator = j;
    }
  }

  int floorParser = 0; //Create floor passing int

    //Keep going through floors until elevator is found
  while (system[floorParser][minElevator] == NULL) {

    floorParser++; //Increment to next floor
  }

  int elevatorFloor = floorParser; //Get the elevators floor

  int newState = system[elevatorFloor][minElevator]->state; //Find state of found elevator

//If elevator is already on desired floor, don't move the elevator
  if (system[elevatorFloor][minElevator]->floorOrder[0] == 0) {

    newState = 0;

  }

  //Make elevator go up if below desired floor
  else if (floorWanted > elevatorFloor) {
    newState = 1;
  }

//Make elevator go down if above desired floor
  else if (floorWanted < elevatorFloor) {
    newState = -1;
  }

//Give the elevator the new state
  system[elevatorFloor][minElevator]->state = newState;

   //Make sure an elevator is found
  if (minElevator != -1) {
     // Move the chosen elevator to the desired floor
        //add the floor to the array of the determined elevator
    addFloor(system, floorWanted, system[elevatorFloor][minElevator]->floorOrder, system[elevatorFloor][minElevator]->state, system[elevatorFloor][minElevator]->counter, elevatorFloor, minElevator);
    //Adds 1 to the amount of floor directions in the array
    system[elevatorFloor][minElevator]->counter += 1;
  }
}

//Imagine you are in an elevator and want to get dropped off at a floor. 
//This function essentially adds the drop off floor to the elevator array
void gotoFloor(struct node*** system, int floorWanted, int elevatorColumn) {

  int e = 0;

  while (system[e][elevatorColumn] == NULL) {

    e++;
  }

  int check = 0;
  //check to see if the floor has already been pressed
  while (check <= (system[e][elevatorColumn]->counter)) {  
    if (floorWanted == system[e][elevatorColumn]->floorOrder[check]) {
      return;
    }         //if the value already exists return 0;
    check++;  //add one to check variable
  }

//Add desired floor to floor destination array of elevator
  if (floorWanted >= 0) {
    addFloor(system, floorWanted, system[e][elevatorColumn]->floorOrder, system[e][elevatorColumn]->state,
             system[e][elevatorColumn]->counter, e, elevatorColumn);
    system[e][elevatorColumn]->counter += 1;
  }
}