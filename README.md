**ElevatorCode**

*Queens Engineering: 178, Data Structures and Algorithms Final Capstone Project*

**Description:**

This project is an efficient multi-elevator system designed to minimize waiting times for users in busy buildings. It can be adapted for each unique building's requirements.

**Components Needed:**

- Arduino H-12 wireless transceiver
- Arduino LCD display
- Arduino

**Overview:**

Often, people experience long wait times for elevators in busy buildings. The system developed by Team 10 aims to minimize these waiting times by efficiently managing multiple elevators. The system allows elevators to wirelessly communicate to determine which elevator should be called. Additionally, elevators can be called to pick up passengers on their way to a set of floors, further reducing waiting times. Passengers can also add floors in the direction the elevator is already traveling once they are on board.

**Overall Design:**

The system comprises one Arduino serving as the main control system, which communicates wirelessly with three other Arduinos acting as elevators. Each elevator Arduino runs the same code, with only a unique identifier (a, b, c) changed. The main control Arduino has a more complex code responsible for determining elevator destinations. Approximately every 2 seconds, the main Arduino sends a message to each elevator Arduino with the next floor destination. The elevators also send back requests when needed. The current location of each elevator is displayed on LCD displays, with the main control showing all three locations.

**Code:**

- The code for non-home bases will be uploaded shortly.
- `projectWiring.png` illustrates the electrical connections.
- `homebasecode.ino` contains the code that will run on the main control Arduino.

**Video Demo:**

A video demonstration of the project working can be viewed [here](https://queensuca-my.sharepoint.com/:v:/g/personal/21cf40_queensu_ca/ETZJQcOKksJInBri4_0KAmIBNZLjcPoI-rsat9ttv0f8xg?e=CUsbvM).

Feel free to contact the project team for further information.
