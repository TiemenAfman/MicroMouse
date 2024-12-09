#include <WiFi.h>
#include <WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include "sensor.h"
#include "Stepper.h"
#include "motors.h"

// Replace with your network credentials
const char* ssid = "MicroMouseTA_AP";
const char* password = "12345678";

// Instantiate Web server on port 80
WebServer server(80);

// Socketserver op poort 1234
WiFiServer socketServer(1234);

String debugInfo = "Debugging Information:\n";

//Password OTA = Admin

String message = "started";

WiFiClient espClient;

// Rastergegevens
int gridData[5][5][2]; // raster met 2 getallen per cel


// 
bool testButton = false;
bool enblState = false;
bool debugMode = true;

uint32_t SocketDelay = 90;

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
    static const int maze_size = 5;
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
      // maze[goalX - 1][goalY - 1] = 0;
      // fifo.enqueue(goalX - 1, goalY - 1);

      maze[goalX][goalY] = 0;
      fifo.enqueue(goalX, goalY);

      // maze[goalX - 1][goalY] = 0;
      // fifo.enqueue(goalX - 1, goalY);

      // maze[goalX][goalY - 1] = 0;
      // fifo.enqueue(goalX, goalY - 1);

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



void setup() {
  // serial communication
  Serial.begin(9600);

  // hardware
  //Sensor
  pinMode(sensorPinF, INPUT); // Stel de sensor pin in als invoer
  pinMode(sensorPinL, INPUT); // Stel de sensor pin in als invoer
  pinMode(sensorPinR, INPUT); // Stel de sensor pin in als invoer
    // Stel voor stepper 1
  pinMode(motorL_step_pin, OUTPUT);
  pinMode(motorL_dir_pin, OUTPUT);
    // Stel voor stepper 1
  pinMode(motorR_step_pin, OUTPUT);
  pinMode(motorR_dir_pin, OUTPUT);
  // Stel beginrichting in (bijvoorbeeld vooruit)
  digitalWrite(motorL_dir_pin, HIGH);  
  digitalWrite(motorR_dir_pin, LOW);  
  // enable steppers
  pinMode(ENBL_PIN, OUTPUT);
  digitalWrite(ENBL_PIN, HIGH);  
  // WiFi and OTA
    // Set up the Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point started");

  // Start OTA service
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("OTA Ready");

  // Start Web Server
  server.on("/", handleRoot);

  // Voeg de toggle-handlers toe
  server.on("/toggle", handleToggle);
  server.on("/debug", handleDebugToggle);
  server.on("/enblToggle", handleEnblToggle); 

  //start de webserver
  server.begin();
  Serial.println("HTTP server started");

  // Socketserver starten
  socketServer.begin();

    // Raster initialiseren
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      gridData[i][j][0] = 0; // Initieer het eerste getal
      gridData[i][j][1] = 0; // Initieer het tweede getal
    }
  }

}

void handleRoot() {
  String html = "<html><body>";

  // Voeg een toggle button toe
  html += "<button id='toggleButton' onclick='toggle()'>Toggle</button>";

  // Voeg een JavaScript-functie toe om de knop te laten werken
  html += "<script>";
  html += "function toggle() {";
  html += "var xhr = new XMLHttpRequest();";
  html += "xhr.open('GET', '/toggle', true);";
  html += "xhr.send();";
  html += "}";
  html += "</script>";

  // Voeg een toggle button toe
  html += "<button id='debugButton' onclick='debug()'>debug</button>";

  // Voeg een JavaScript-functie toe om de knop te laten werken
  html += "<script>";
  html += "function debug() {";
  html += "var xhr = new XMLHttpRequest();";
  html += "xhr.open('GET', '/debug', true);";
  html += "xhr.send();";
  html += "}";
  html += "</script>";

  // Voeg de toggle-knop toe (voor Enable)
  html += "<button id='enblButton' onclick='toggleEnbl()' style='background-color: gray;'>Enable</button>";

  // JavaScript voor de Enable-knop
  html += "<script>";
  html += "function toggleEnbl() {";
  html += "var xhr = new XMLHttpRequest();";
  html += "xhr.open('GET', '/enblToggle', true);";
  html += "xhr.onload = function() {";
  html += "var response = JSON.parse(xhr.responseText);";  // Ontvang de JSON-respons
  html += "if (response.enblState) {";
  html += "document.getElementById('enblButton').style.backgroundColor = 'green';";  // Zet de knop op groen als Enbl aan is
  html += "} else {";
  html += "document.getElementById('enblButton').style.backgroundColor = 'gray';";   // Zet de knop terug naar grijs als Enbl uit is
  html += "}";
  html += "};";
  html += "xhr.send();";
  html += "}";
  html += "</script>";

  // Voeg het raster toe
  html += "<div style='display: grid; grid-template-columns: repeat(5, 50px); grid-gap: 5px;'>";
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      html += "<div style='width: 50px; height: 50px; text-align: center; line-height: 25px; border: 1px solid black;'>";
      html += "<span>" + String(gridData[i][j][0]) + "</span><br>"; // Eerste getal
      html += "<span>" + String(gridData[i][j][1], BIN) + "</span>"; // Tweede getal als binaire string
      html += "</div>";
    }
  }
  html += "</div>";

  // Debug info toevoegen zoals eerder
  html += "<pre>";
  html += debugInfo;
  html += "</pre></body></html>";
  

  server.send(200, "text/html", html);
}


void handleToggle() {
  // Toggelen van de boolean
  testButton = !testButton;

  // Stuur feedback naar de webpagina
  String response = enblState ? "Enbl is AAN" : "Enbl is UIT";

  server.send(200, "text/plain", response);
}

void handleDebugToggle() {
  // Toggelen van de boolean
  debugMode = !debugMode;

  // Stuur feedback naar de webpagina
  String response = debugMode ? "Debug is AAN" : "Debug is UIT";

  server.send(200, "text/plain", response);

  espClient.println(debugMode ? "Debug is AAN" : "Debug is UIT");
}

void handleEnblToggle() {
  // Toggle de enable status
  enblState = !enblState;
  
  // Zet de Enable-pin op hoog of laag
  digitalWrite(ENBL_PIN, enblState ? LOW : HIGH);  // Laag betekent de driver is ingeschakeld, hoog betekent uitgeschakeld

  // Stuur de nieuwe status terug als JSON
  String jsonResponse = "{\"enblState\":";
  jsonResponse += enblState ? "true" : "false";
  jsonResponse += "}";
  
  server.send(200, "application/json", jsonResponse);
}



void loop() {
    // Controleren op nieuwe clients
    if (!espClient || !espClient.connected()) {
      espClient = socketServer.available(); // Accepteer nieuwe verbindingen
      if (espClient) {
        Serial.println("Nieuwe client verbonden!");
        resetAll();
      }
    }

    // Als de client verbonden is
    if (espClient && espClient.connected()) {
      SocketDelay = 90;
      // Verzenden van een bericht naar de client als er iets in `message` staat
      if (message != "") {
        espClient.println(message);
        Serial.println("Bericht verzonden: " + message);
        message = ""; // Reset message na verzending
      }

      // Ontvangen van data van de client
      if (espClient.available()) {
        char c = espClient.read();
        message += c;

        debugInfo += message;

        if (message.startsWith("ena")){
          debugInfo += message;
          
          if (message == "enable") {
            digitalWrite(ENBL_PIN, HIGH);
            debugInfo += " ENBL_PIN is nu HOOG. ";
            message = "";
          }
        }
      }  
    }
    else {
      SocketDelay = 0;
    }
  
  // Handle OTA events
  ArduinoOTA.handle();

  // Handle Web server events
  server.handleClient();

if (testButton){
  testButton = !debugMode;

    debugInfo += "Sensor Value F: " + String(wallDistance(sensorPinF), 2) + " Sensor Value L: " + String(wallDistance(sensorPinL), 2) + " Sensor Value R: " + String(wallDistance(sensorPinR), 2) + "\n";
    debugInfo += "Sensor Wall F: " + String(wallFront()) + " Sensor Wall L: " + String(wallLeft()) + " Sensor Wall R: " + String(wallRight()) + "\n";
    
    if (!isGoal(micromouse.current_position[0], micromouse.current_position[1])) {
      //log("Running...");
      updateMaze2(micromouse.current_position[0],micromouse.current_position[1]);
      get_next_move();
    }
    else{
      log("finish!");
      
      // Show the flood values and walls in the maze
      for (int i = 0; i < micromouse.maze_size; i++) {
          for (int j = 0; j < micromouse.maze_size; j++) {
              setText(i, j, String(micromouse.maze[i][j]));
              if (micromouse.walls[i][j] & micromouse.NORTH) setWall(i, j, 'n');
              if (micromouse.walls[i][j] & micromouse.EAST) setWall(i, j, 'e');
              if (micromouse.walls[i][j] & micromouse.SOUTH) setWall(i, j, 's');
              if (micromouse.walls[i][j] & micromouse.WEST) setWall(i, j, 'w');
          }
      }

      //reset and start again
      micromouse.current_position[0] = 0;
      micromouse.current_position[1] = 0;
      micromouse.current_direction = "N";

      testButton = false;
    }
  }
}

// Check if the goal is reached
bool isGoal(int x, int y) {
    return (x == micromouse.goalX && y == micromouse.goalY);
}

void updateMaze2(int x, int y) {
    //log("Position: " + String(x) + ", " + String(y));

    // Set buffer for queue
    FIFOBuffer fifo(50);

    micromouse.visited[x][y] = true;
    setColor(x,y, 'b');
     // setColor(x, y, 'w');
    
    // Check walls and update the maze
    if (wallFront()) {
        log("Wall front Detected");
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
    // micromouse.maze[micromouse.maze_size/2 - 1][micromouse.maze_size/2 - 1] = 0;
    // fifo.enqueue(micromouse.maze_size/2 - 1, micromouse.maze_size/2 - 1);
    
    micromouse.maze[micromouse.maze_size/2][micromouse.maze_size/2] = 0;
    fifo.enqueue(micromouse.maze_size/2, micromouse.maze_size/2);
    
    // micromouse.maze[micromouse.maze_size/2 - 1][micromouse.maze_size/2] = 0;
    // fifo.enqueue(micromouse.maze_size/2 - 1, micromouse.maze_size/2);
    
    // micromouse.maze[micromouse.maze_size/2][micromouse.maze_size/2 - 1] = 0;
    // fifo.enqueue(micromouse.maze_size/2, micromouse.maze_size/2 - 1);
    
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
            setText(i, j, String(micromouse.maze[i][j]));
            if (micromouse.walls[i][j] & micromouse.NORTH) setWall(i, j, 'n');
            if (micromouse.walls[i][j] & micromouse.EAST) setWall(i, j, 'e');
            if (micromouse.walls[i][j] & micromouse.SOUTH) setWall(i, j, 's');
            if (micromouse.walls[i][j] & micromouse.WEST) setWall(i, j, 'w');

            gridData[i][j][0] = micromouse.maze[i][j];    // set het eerste getal (FLoodValue)
            gridData[i][j][1] = micromouse.walls[i][j];   // set het tweede getal (walls)
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
        log("Wall front Undetected");
        int nx = x + (micromouse.current_direction == "E") - (micromouse.current_direction == "W");
        int ny = y + (micromouse.current_direction == "N") - (micromouse.current_direction == "S");
        add_move(possible_moves, move_count, nx, ny);
        debugInfo += "no wall front: add move";
        //log("nx: " + String(nx) + " ny: " + String(ny));
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
        debugInfo += "no wall right: add move";
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
        debugInfo += "no wall left: add move";
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
        debugInfo += "dead end: add move";
    }

    int* next_move = find_min_move(possible_moves, move_count);
    if (next_move) {
      nx = next_move[0];
      ny = next_move[1];
      if (nx == x + 1 && ny == y) {
          // Move East
          //log("next move: east");
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
          //log("next move: west");
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
          //log("next move: north");
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
          //log("next move: south");
          //log("nx: " + String(nx) + " ny: " + String(ny) + "x: " + String(x) + " y: " + String(y));
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
      //log("move_forward");
    }
}

void move_forward(){
  moveForward(188);
  if (micromouse.current_direction == "N"){
    micromouse.current_position[0] = micromouse.current_position[0];
    micromouse.current_position[1] = micromouse.current_position[1] + 1;
    //log("move north");
  }
  else if (micromouse.current_direction == "E"){
    micromouse.current_position[0] = micromouse.current_position[0] + 1;
    micromouse.current_position[1] = micromouse.current_position[1];
    //log("move east");
  }
  else if (micromouse.current_direction == "S"){
    micromouse.current_position[0] = micromouse.current_position[0];
    micromouse.current_position[1] = micromouse.current_position[1] - 1;
    //log("move south");
  }
  else if (micromouse.current_direction == "W"){
    micromouse.current_position[0] = micromouse.current_position[0] - 1;
    micromouse.current_position[1] = micromouse.current_position[1];
    //log("move west");
  }
}

void turn_left(){
  turn(left);
  if (micromouse.current_direction == "N"){
    micromouse.current_direction = "W";
    //log("left new direction: west");
  }
  else if (micromouse.current_direction == "E"){
    micromouse.current_direction = "N";
    //log("left new direction: north");
  }
  else if (micromouse.current_direction == "S"){
    micromouse.current_direction = "E";
    //log("left new direction: east");
  }
  else if (micromouse.current_direction == "W"){
    micromouse.current_direction = "S";
    //log("left new direction: south");
  }
}
void turn_right(){
  turn(right);
  if (micromouse.current_direction == "N"){
    micromouse.current_direction = "E";
    //log("right new direction: east");
  }
  else if (micromouse.current_direction == "E"){
    micromouse.current_direction = "S";
    //log("right new direction: south");
  }
  else if (micromouse.current_direction == "S"){
    micromouse.current_direction = "W";
    //log("right new direction: west");
  }
  else if (micromouse.current_direction == "W"){
    micromouse.current_direction = "N";
    //log("right new direction: north");
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
