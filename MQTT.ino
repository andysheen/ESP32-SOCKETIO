void callback(char* topic, byte* message, unsigned int length) {

  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }

  const size_t capacity = 2 * JSON_OBJECT_SIZE(2) + 60;
  DynamicJsonDocument incomingDoc(capacity);
  deserializeJson(incomingDoc, messageTemp);
  const char* recMacAddress = incomingDoc["macAddress"];
  const char* data_project = incomingDoc["data"]["project"];

  Serial.print("I got a message from ");
  Serial.println(recMacAddress);
  Serial.print("Which is of type ");
  Serial.println(data_project);

  String mac = recMacAddress;

  if (mac !=  getRemoteMacAddress(0)) {
    if (String(topic) == getRemoteMacAddress(1).c_str()) {
      if (String(data_project) == "lighttouch") {
        long data_hue = incomingDoc["data"]["hue"];
        Serial.print("Light touch! Hue: ");
        Serial.println(data_hue);
        // TODO - Run light touch
        hue[REMOTELED] = (uint8_t)data_hue;
        ledChanged[REMOTELED] = true;
        //added to enable reset of fading mid fade
        isFadingRGB[REMOTELED] = false;
        fadeRGB(REMOTELED);
        startLongFade(REMOTELED);
      }
      else if (String(data_project) == "test") {
        blinkDevice();
      }
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(WiFi.macAddress().c_str())) {
      Serial.println("connected");
      blinkOnConnect();
      // Subscribe
      client.subscribe(getRemoteMacAddress(1).c_str(), 1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void checkMQTT() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void sendMsgMQTT() {

  Serial.println("colour send");
  const size_t capacity = 3 * JSON_OBJECT_SIZE(2);
  DynamicJsonDocument doc(capacity);
  doc["macAddress"] = getRemoteMacAddress(0);
  JsonObject data = doc.createNestedObject("data");
  data["project"] = "lighttouch";
  data["hue"] = String((int)getUserHue());
  String sender;
  serializeJson(doc, sender);
  //  socketIO.emit("msg", sender.c_str());
  client.publish(getRemoteMacAddress(1).c_str(), sender.c_str());
}
