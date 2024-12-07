// ----- API -----
uint32_t dilee = 90;

void log(String message) {
    espClient.print("log " + message + "\n");
    delay(dilee);
}

int mazeWidth() {
    return getInteger("mazeWidth");
}

int mazeHeight() {
    return getInteger("mazeHeight");
}

// bool wallFront() {
//     return getBoolean("wallFront");
// }

// bool wallRight() {
//     return getBoolean("wallRight");
// }

// bool wallLeft() {
//     return getBoolean("wallLeft");
// }

// bool moveForward() {
//     return getAck("moveForward");
// }

// void turnRight() {
//     getAck("turnRight");
// }

// void turnLeft() {
//     getAck("turnLeft");
// }

void setWall(int x, int y, char direction) {
    espClient.print(
        "setWall "
        + String(x) + " "
        + String(y) + " "
        + String(direction) + "\n"
    );
    delay(dilee);
}

void clearWall(int x, int y, char direction) {
    Serial.print(
        "clearWall "
        + String(x) + " "
        + String(y) + " "
        + String(direction) + "\n"
    );
}

void setColor(int x, int y, char color) {
    espClient.print(
        "setColor "
        + String(x) + " "
        + String(y) + " "
        + String(color) + "\n"
    );
    delay(dilee);
}

void clearColor(int x, int y) {
    Serial.print(
        "clearColor "
        + String(x) + " "
        + String(y) + "\n"
    );
}

void clearAllColor() {
    Serial.print("clearAllColor\n");
}

void setText(int x, int y, String text) {
    espClient.print(
        "setText "
        + String(x) + " "
        + String(y) + " "
        + text + "\n"
    );
    delay(dilee);
}

void clearText(int x, int y) {
    Serial.print(
        "clearText "
        + String(x) + " "
        + String(y) + "\n"
    );
}

void clearAllText() {
    Serial.print("clearAllText\n");
}

bool wasReset() {
    return getBoolean("wasReset");
}

void ackReset() {
    getAck("ackReset");
}

void resetAll(){
    espClient.print(
        "resetAll\n"
    );
    delay(dilee); 
}

// ----- Helpers -----

String readline() {
    String response = "";
    while (response == "") {
        response = Serial.readStringUntil('\n');
    }
    return response;
}

String communicate(String command) {
    Serial.print(command + "\n");
    return readline();
}

bool getAck(String command) {
    String response = communicate(command);
    return response == "ack";
}

bool getBoolean(String command) {
    String response = communicate(command);
    return response == "true";
}

int getInteger(String command) {
    String response = communicate(command);
    return response.toInt();
}
