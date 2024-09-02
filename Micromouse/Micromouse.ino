#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include "sendMessage.h"  
#include "sensor.h"
#include "Stepper.h"

// Replace with your network credentials
const char* ssid = "Ziggo6FF6DC6";
const char* password = "ncV4dNppuvfj";

// Replace with your MQTT Broker's IP address or hostname
const char* mqtt_server = "192.168.178.80";
const int mqtt_port = 1883;

// pins
const int stepPin = D2; // Step pin verbonden met D2
const int dirPin = D3;  // Dir pin verbonden met D3
// Pin-definities multiplexing
const int S0 = D5;
const int S1 = D6;
const int S2 = D7;


//Password OTA = Admin

String message = "Log Started";

WiFiClient espClient;
PubSubClient mqttClient(espClient);  // Create a PubSubClient object with the WiFiClient

class FIFOBuffer {
  private:
    int capacity;
    int *bufferX;
    int *bufferY;
    int front;
    int rear;
    int count;

  public:
    // Constructor
    FIFOBuffer(int cap) {
      capacity = cap;
      bufferX = new int[capacity];
      bufferY = new int[capacity];
      front = 0;
      rear = -1;
      count = 0;
    }

    // Destructor
    ~FIFOBuffer() {
      delete[] bufferX;
      delete[] bufferY;
    }

    // Enqueue item
    void enqueue(int x, int y) {
      if (is_full()) {
        Serial.println("Buffer is full");
        return;
      }
      rear = (rear + 1) % capacity;
      bufferX[rear] = x;
      bufferY[rear] = y;
      count++;
    }

    // Dequeue item
    void dequeue(int &x, int &y) {
      if (is_empty()) {
        Serial.println("Buffer is empty");
        return;
      }
      x = bufferX[front];
      y = bufferY[front];
      front = (front + 1) % capacity;
      count--;
    }

    // Check if buffer is empty
    bool is_empty() {
      return count == 0;
    }

    // Check if buffer is full
    bool is_full() {
      return count == capacity;
    }

    // Get the size of the buffer
    int size() {
      return count;
    }
};

class MicroMouse {
  public:
    static const int maze_size = 16;
    int maze[maze_size][maze_size];
    int walls[maze_size][maze_size];
    bool visited[maze_size][maze_size];
    int current_position[2];
    String current_direction;
    int goal = maze_size/2;

    // Wall constants
    static const int NORTH = 0b0001;
    static const int EAST  = 0b0010;
    static const int SOUTH = 0b0100;
    static const int WEST  = 0b1000;

    // Define the goal coordinates
    int goalX = maze_size/2;
    int goalY = maze_size/2;

    MicroMouse() {
      // Initialize the maze array with -1
      for (int x = 0; x < maze_size; x++) {
        for (int y = 0; y < maze_size; y++) {
          maze[x][y] = -1;
          if (x == 0) walls[x][y] |= WEST;
          if (y == maze_size - 1) walls[x][y] |= NORTH;
          if (x == maze_size - 1) walls[x][y] |= EAST;
          if (y == 0) walls[x][y] |= SOUTH;
        }
      }

      // Set initial position
      current_position[0] = 0; // X-coordinate
      current_position[1] = 0; // Y-coordinate
      current_direction = "N"; // Initial direction is North

      // Set buffer for queue
      FIFOBuffer fifo(50);

      // Set goal to 0 and add to buffer
      maze[goalX - 1][goalY - 1] = 0;
      fifo.enqueue(goalX - 1, goalY - 1);

      maze[goalX][goalY] = 0;
      fifo.enqueue(goalX, goalY);

      maze[goalX - 1][goalY] = 0;
      fifo.enqueue(goalX - 1, goalY);

      maze[goalX][goalY - 1] = 0;
      fifo.enqueue(goalX, goalY - 1);

      // Dequeue items from buffer
      while (fifo.size() > 0) {
        int x, y;
        fifo.dequeue(x, y);

        // Set point below
        if (y > 0 && maze[x][y - 1] == -1) {
          maze[x][y - 1] = maze[x][y] + 1;
          fifo.enqueue(x, y - 1);
        }

        // Set point above
        if (y < maze_size - 1 && maze[x][y + 1] == -1) {
          maze[x][y + 1] = maze[x][y] + 1;
          fifo.enqueue(x, y + 1);
        }

        // Set point left
        if (x > 0 && maze[x - 1][y] == -1) {
          maze[x - 1][y] = maze[x][y] + 1;
          fifo.enqueue(x - 1, y);
        }

        // Set point right
        if (x < maze_size - 1 && maze[x + 1][y] == -1) {
          maze[x + 1][y] = maze[x][y] + 1;
          fifo.enqueue(x + 1, y);
        }
      }
    }
};

MicroMouse micromouse;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ESP8266Client")) {
      Serial.println("connected");
      mqttClient.subscribe("test/topic"); // Subscribe to a topic if needed
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  // serial communication
  Serial.begin(9600);

  // hardware
  //Sensor
  pinMode(sensorPin, INPUT); // Stel de sensor pin in als invoer
    // Stel voor stepper 1
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  // Stel beginrichting in (bijvoorbeeld vooruit)
  digitalWrite(dirPin, LOW);  
  // Stel pinmodi in voor multiplexer
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);

  // Stel multiplexer in
  digitalWrite(S0, LOW);
  digitalWrite(S1, LOW);
  digitalWrite(S2, LOW);
// test

  // WiFi and OTA
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    //log("."); // Uncomment if you have a log function
  }

  ArduinoOTA.setHostname("esp_micromouse");

  ArduinoOTA.onStart([]() {
    //log("Start"); // Uncomment if you have a log function
  });
  ArduinoOTA.onEnd([]() {
    //log("\nEnd"); // Uncomment if you have a log function
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    //log("Progress: %u%%\r" + (progress / (total / 100))); // Uncomment if you have a log function
  });
  ArduinoOTA.onError([](ota_error_t error) {
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Connected to WiFi");
  Serial.print(WiFi.localIP());

  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(callback);
}

void loop() {
  // if (!mqttClient.connected()) {
  //   reconnect();
  // }
  //mqttClient.loop();

  ArduinoOTA.handle();

  // Check and send messages for the first sensor
  //checkAndSendMessage(mqttClient, sensorPin, "MicroMouse/SensorFront", lastValue);

  bool isWall = wallFront_();  // Roep de functie aan om te controleren of er een muur voor de sensor is
  //publishValue(mqttClient, "MicroMouse/IsWallFront", String(isWall));
  float distance = wallDistance();  // Meet de 
  Serial.println(distance);

  if (false){
    if (isWall) {
      message = "Muur gedetecteerd!";
    } else {
      message = "Geen muur gedetecteerd";

      
      
      // Bereken de snelheid van de stappenmotor op basis van de afstand
      // Hoe dichter bij de muur, hoe langzamer de motor draait
      // Pas deze formule aan naar wens
      int speed = map(distance, 3, 15, 1000, 100); // Snelheid in microseconden, afhankelijk van afstand
      
      // Limiteer de snelheid om te voorkomen dat deze te snel of te langzaam wordt
      speed = constrain(speed, 100, 1000);

      // Stappenmotor aansturen
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(speed); // Pas de snelheid aan
      digitalWrite(stepPin, LOW);
      delayMicroseconds(speed); // Pas de snelheid aan

    }
  }

  delay(100);

  // if ( !isGoal(micromouse.current_position[0], micromouse.current_position[1])) {
  //     log("Running...");
  //     updateMaze2(micromouse.current_position[0],micromouse.current_position[1]);
  //     get_next_move();
  //   }
  // else{
  //       log("finish!");
    
  //   // Show the flood values and walls in the maze
  //   for (int i = 0; i < micromouse.maze_size; i++) {
  //       for (int j = 0; j < micromouse.maze_size; j++) {
  //           setText(i, j, String(micromouse.maze[i][j]));
  //           if (micromouse.walls[i][j] & micromouse.NORTH) setWall(i, j, 'n');
  //           if (micromouse.walls[i][j] & micromouse.EAST) setWall(i, j, 'e');
  //           if (micromouse.walls[i][j] & micromouse.SOUTH) setWall(i, j, 's');
  //           if (micromouse.walls[i][j] & micromouse.WEST) setWall(i, j, 'w');
  //       }
  //   }

  //   // //reset and start again
  //   // micromouse.current_position[0] = 0;
  //   // micromouse.current_position[1] = 0;
  //   // micromouse.current_direction = "N";
  //   // ackReset();
  // }

  // if (wasReset()){
  //   //reset and start again
  //   micromouse.current_position[0] = 0;
  //   micromouse.current_position[1] = 0;
  //   micromouse.current_direction = "N";
  //   ackReset();
  // }

  // Wacht even
  // for (int i = 0; i < 1; i++) {
  //   delay(1);
  //   ArduinoOTA.handle();
  // }    

  
//publishValue(mqttClient, "MicroMouse/Log", message);
}

// Check if the goal is reached
bool isGoal(int x, int y) {
    return (x == micromouse.goalX && y == micromouse.goalY);
}

void updateMaze2(int x, int y) {
    log("Position: " + String(x) + ", " + String(y));

    // Set buffer for queue
    FIFOBuffer fifo(50);

    micromouse.visited[x][y] = true;
    setColor(x,y, 'w');
     // setColor(x, y, 'w');
    
    // Check walls and update the maze
    if (wallFront()) {
        if (micromouse.current_direction == "N" && y < micromouse.maze_size - 1) {
            micromouse.walls[x][y] |= micromouse.NORTH;
            micromouse.walls[x][y + 1] |= micromouse.SOUTH;
        } else if (micromouse.current_direction == "E" && x < micromouse.maze_size - 1) {
            micromouse.walls[x][y] |= micromouse.EAST;
            micromouse.walls[x + 1][y] |= micromouse.WEST;
        } else if (micromouse.current_direction == "S") {
            micromouse.walls[x][y] |= micromouse.SOUTH;
            micromouse.walls[x][y - 1] |= micromouse.NORTH;
        } else if (micromouse.current_direction == "W") {
            micromouse.walls[x][y] |= micromouse.WEST;
            micromouse.walls[x - 1][y] |= micromouse.EAST;
        }
    }
    
    if (wallLeft()) {
        if (micromouse.current_direction == "N") {
            micromouse.walls[x][y] |= micromouse.WEST;
            micromouse.walls[x - 1][y] |= micromouse.EAST;
        } else if (micromouse.current_direction == "E" && y < micromouse.maze_size - 1) {
            micromouse.walls[x][y] |= micromouse.NORTH;
            micromouse.walls[x][y + 1] |= micromouse.SOUTH;
        } else if (micromouse.current_direction == "S" && x < micromouse.maze_size - 1) {
            micromouse.walls[x][y] |= micromouse.EAST;
            micromouse.walls[x + 1][y] |= micromouse.WEST;
        } else if (micromouse.current_direction == "W") {
            micromouse.walls[x][y] |= micromouse.SOUTH;
            micromouse.walls[x][y - 1] |= micromouse.NORTH;
        }
    }
    
    if (wallRight()) {
        if (micromouse.current_direction == "N" && x < micromouse.maze_size - 1) {
            micromouse.walls[x][y] |= micromouse.EAST;
            micromouse.walls[x + 1][y] |= micromouse.WEST;
        } else if (micromouse.current_direction == "E") {
            micromouse.walls[x][y] |= micromouse.SOUTH;
            micromouse.walls[x][y - 1] |= micromouse.NORTH;
        } else if (micromouse.current_direction == "S") {
            micromouse.walls[x][y] |= micromouse.WEST;
            micromouse.walls[x - 1][y] |= micromouse.EAST;
        } else if (micromouse.current_direction == "W" && y < micromouse.maze_size - 1) {
            micromouse.walls[x][y] |= micromouse.NORTH;
            micromouse.walls[x][y + 1] |= micromouse.SOUTH;
        }
    }
    
    // Clear maze
    for (int i = 0; i < micromouse.maze_size; i++) {
        for (int j = 0; j < micromouse.maze_size; j++) {
            micromouse.maze[i][j] = -1;
        }
    }
    
    // Set goal points and add to buffer
    micromouse.maze[micromouse.maze_size/2 - 1][micromouse.maze_size/2 - 1] = 0;
    fifo.enqueue(micromouse.maze_size/2 - 1, micromouse.maze_size/2 - 1);
    
    micromouse.maze[micromouse.maze_size/2][micromouse.maze_size/2] = 0;
    fifo.enqueue(micromouse.maze_size/2, micromouse.maze_size/2);
    
    micromouse.maze[micromouse.maze_size/2 - 1][micromouse.maze_size/2] = 0;
    fifo.enqueue(micromouse.maze_size/2 - 1, micromouse.maze_size/2);
    
    micromouse.maze[micromouse.maze_size/2][micromouse.maze_size/2 - 1] = 0;
    fifo.enqueue(micromouse.maze_size/2, micromouse.maze_size/2 - 1);
    
    // Dequeue items from buffer
    while (fifo.size() > 0) {
      // Dequeue items from buffer
      while (fifo.size() > 0) {
        int x, y;
        fifo.dequeue(x, y);

        // Set point below
        if (y > 0 && micromouse.maze[x][y - 1] == -1 && !(micromouse.walls[x][y] & micromouse.SOUTH)) {
          micromouse.maze[x][y - 1] = micromouse.maze[x][y] + 1;
          fifo.enqueue(x, y - 1);
        }

        // Set point above
        if (y < micromouse.maze_size - 1 && micromouse.maze[x][y + 1] == -1 && !(micromouse.walls[x][y] & micromouse.NORTH)) {
          micromouse.maze[x][y + 1] = micromouse.maze[x][y] + 1;
          fifo.enqueue(x, y + 1);
        }

        // Set point left
        if (x > 0 && micromouse.maze[x - 1][y] == -1 && !(micromouse.walls[x][y] & micromouse.WEST)) {
          micromouse.maze[x - 1][y] = micromouse.maze[x][y] + 1;
          fifo.enqueue(x - 1, y);
        }

        // Set point right
        if (x < micromouse.maze_size - 1 && micromouse.maze[x + 1][y] == -1 && !(micromouse.walls[x][y] & micromouse.EAST)) {
          micromouse.maze[x + 1][y] = micromouse.maze[x][y] + 1;
          fifo.enqueue(x + 1, y);
        }
      }
    }
    
    // Show the flood values and walls in the maze
    for (int i = 0; i < micromouse.maze_size; i++) {
        for (int j = 0; j < micromouse.maze_size; j++) {
            // setText(i, j, String(micromouse.maze[i][j]));
            // if (micromouse.walls[i][j] & micromouse.NORTH) setWall(i, j, 'n');
            // if (micromouse.walls[i][j] & micromouse.EAST) setWall(i, j, 'e');
            // if (micromouse.walls[i][j] & micromouse.SOUTH) setWall(i, j, 's');
            // if (micromouse.walls[i][j] & micromouse.WEST) setWall(i, j, 'w');
        }
    }
}

void get_next_move() {
    int x = micromouse.current_position[0];
    int y = micromouse.current_position[1];
    
    int nx;
    int ny;
    
    int possible_moves[micromouse.maze_size][micromouse.maze_size]; // Adjust size as needed
    int move_count = 0;

    if (!wallFront()) {
        int nx = x + (micromouse.current_direction == "E") - (micromouse.current_direction == "W");
        int ny = y + (micromouse.current_direction == "N") - (micromouse.current_direction == "S");
        add_move(possible_moves, move_count, nx, ny);
        log("no wall front: add move");
        log("nx: " + String(nx) + " ny: " + String(ny));
    }

    if (!wallRight()) {
        int nx, ny;
        if (micromouse.current_direction == "N") {
            nx = x + 1; ny = y;
        } else if (micromouse.current_direction == "E") {
            nx = x; ny = y - 1;
        } else if (micromouse.current_direction == "S") {
            nx = x - 1; ny = y;
        } else if (micromouse.current_direction == "W") {
            nx = x; ny = y + 1;
        }
        add_move(possible_moves, move_count, nx, ny);
        log("no wall right: add move");
    }

    if (!wallLeft()) {
        int nx, ny;
        if (micromouse.current_direction == "N") {
            nx = x - 1; ny = y;
        } else if (micromouse.current_direction == "E") {
            nx = x; ny = y + 1;
        } else if (micromouse.current_direction == "S") {
            nx = x + 1; ny = y;
        } else if (micromouse.current_direction == "W") {
            nx = x; ny = y - 1;
        }
        add_move(possible_moves, move_count, nx, ny);
        log("no wall left: add move");
    }

    if (wallLeft() && wallRight() && wallFront()) {
        int nx, ny;
        if (micromouse.current_direction == "N") {
            nx = x; ny = y - 1;
        } else if (micromouse.current_direction == "E") {
            nx = x - 1; ny = y;
        } else if (micromouse.current_direction == "S") {
            nx = x; ny = y + 1;
        } else if (micromouse.current_direction == "W") {
            nx = x + 1; ny = y;
        }
        add_move(possible_moves, move_count, nx, ny);
        log("dead end: add move");
    }

    int* next_move = find_min_move(possible_moves, move_count);
    if (next_move) {
      nx = next_move[0];
      ny = next_move[1];
      if (nx == x + 1 && ny == y) {
          // Move East
          log("next move: east");
          while (micromouse.current_direction != "E") {
              if (micromouse.current_direction == "N") {
                  turn_right();
              } else if (micromouse.current_direction == "S") {
                  turn_left();
              } else if (micromouse.current_direction == "W") {
                  turn_left();
              }
          }
      } else if (nx == x - 1 && ny == y) {
          // Move West
          log("next move: west");
          while (micromouse.current_direction != "W") {
              if (micromouse.current_direction == "S") {
                  turn_right();
              } else if (micromouse.current_direction == "N") {
                  turn_left();
              } else if (micromouse.current_direction == "E") {
                  turn_left();
              }
          }
      } else if (nx == x && ny == y + 1) {
          // Move North
          log("next move: north");
          while (micromouse.current_direction != "N") {
              if (micromouse.current_direction == "W") {
                  turn_right();
              } else if (micromouse.current_direction == "E") {
                  turn_left();
              } else if (micromouse.current_direction == "S") {
                  turn_left();
              }
          }
      } else if (nx == x && ny == y - 1) {
          // Move South
          log("next move: south");
          log("nx: " + String(nx) + " ny: " + String(ny) + "x: " + String(x) + " y: " + String(y));
          while (micromouse.current_direction != "S") {
              if (micromouse.current_direction == "W") {
                  turn_left();
              } else if (micromouse.current_direction == "E") {
                  turn_right();
              } else if (micromouse.current_direction == "N") {
                  turn_right();
              }
          }
      }
      move_forward();
      log("move_forward");
    }
}

void move_forward(){
  moveForward();
  if (micromouse.current_direction == "N"){
    micromouse.current_position[0] = micromouse.current_position[0];
    micromouse.current_position[1] = micromouse.current_position[1] + 1;
    log("move north");
  }
  else if (micromouse.current_direction == "E"){
    micromouse.current_position[0] = micromouse.current_position[0] + 1;
    micromouse.current_position[1] = micromouse.current_position[1];
    log("move east");
  }
  else if (micromouse.current_direction == "S"){
    micromouse.current_position[0] = micromouse.current_position[0];
    micromouse.current_position[1] = micromouse.current_position[1] - 1;
    log("move south");
  }
  else if (micromouse.current_direction == "W"){
    micromouse.current_position[0] = micromouse.current_position[0] - 1;
    micromouse.current_position[1] = micromouse.current_position[1];
    log("move west");
  }
}

void turn_left(){
  turnLeft();
  if (micromouse.current_direction == "N"){
    micromouse.current_direction = "W";
    log("left new direction: west");
  }
  else if (micromouse.current_direction == "E"){
    micromouse.current_direction = "N";
    log("left new direction: north");
  }
  else if (micromouse.current_direction == "S"){
    micromouse.current_direction = "E";
    log("left new direction: east");
  }
  else if (micromouse.current_direction == "W"){
    micromouse.current_direction = "S";
    log("left new direction: south");
  }
}
void turn_right(){
  turnRight();
  if (micromouse.current_direction == "N"){
    micromouse.current_direction = "E";
    log("right new direction: east");
  }
  else if (micromouse.current_direction == "E"){
    micromouse.current_direction = "S";
    log("right new direction: south");
  }
  else if (micromouse.current_direction == "S"){
    micromouse.current_direction = "W";
    log("right new direction: west");
  }
  else if (micromouse.current_direction == "W"){
    micromouse.current_direction = "N";
    log("right new direction: north");
  }
  
}

void add_move(int moves[][micromouse.maze_size], int &move_count, int nx, int ny) {
    if (nx >= 0 && ny >= 0 && nx < micromouse.maze_size && ny < micromouse.maze_size) { // Adjust bounds as necessary
        moves[move_count][0] = nx;
        moves[move_count][1] = ny;
        move_count++;
    }
}

int* find_min_move(int moves[][micromouse.maze_size], int move_count) {
    if (move_count == 0) return nullptr;

    int* best_move = moves[0];
    int min_value = micromouse.maze[moves[0][0]][moves[0][1]];

    for (int i = 1; i < move_count; i++) {
        int value = micromouse.maze[moves[i][0]][moves[i][1]];
        if (value < min_value) {
            min_value = value;
            best_move = moves[i];
        }
    }

    return best_move;
}
