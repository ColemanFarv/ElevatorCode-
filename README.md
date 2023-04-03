# ElevatorCode-
Queens Engineering: 178, Data Structures and Algorithms Final capstone project

Description: 
Team’s 10 project is an efficient multi-elevator system which can be adapted for each unique building. 

Components Needed:

Arduino H-12 wireless transeiver, Arduino lcd display and Arduino

Overview: 
Often people wait a long time for an elevator, especially when buildings get busy. The system Team 10 designed minimizes the waiting time for the users and which can be adapted for any building. The goal was to be able have multiple elevators, that can wirelessly communicate to determine which elevator should be called. While the elevator has been told the set of floors to go to, it can also be called so it can pick up a person from a floor on its way to decrease waiting time. Once the person gets on, they can add floors in the direction the elevator is already going. 

Overall Design: 

The overall design uses one Arduino as the main control system, which wirelessly communicates to three other Arduinos that act as Elevators. Each of the 3 elevator Arduinos have the same code with only their unique identifier changed (a,b,c), whereas the main control Arduino has a much more complex code with the functions for deciding where the elevators go. The main Arduino sends out a message to each Arduino approixmately every 2 seconds with the next floor destination. The elevators will also send back if they have a request when needed. Overall the current location of the elevators are shown on each LCD display and the main control shows all 3 locations. 
