/*
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
*/

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
  //client.publish(getRemoteMacAddress(1).c_str(), sender.c_str());

  uint16_t packetIdPub2 = mqttClient.publish(getRemoteMacAddress(1).c_str(), 1, true, sender.c_str());
  Serial.print("Publishing at QoS 1, packetId: ");
  Serial.println(packetIdPub2);


}


void onMqttConnect(bool sessionPresent) {
  uint16_t packetIdSub = mqttClient.subscribe(getRemoteMacAddress(0).c_str(), 1);
  Serial.print("Subscribing at QoS 1, packetId: ");
  Serial.println(packetIdSub);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);

  String messageTemp;

  for (int i = 0; i < len; i++) {
    Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
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

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setUpMQTTCallbacks() {
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
}
