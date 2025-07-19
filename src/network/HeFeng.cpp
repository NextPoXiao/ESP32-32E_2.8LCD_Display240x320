
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "HeFeng.h"


HeFeng::HeFeng() {
}

void HeFeng::fans(HeFengCurrentData *data, String id) { 
  std::unique_ptr<WiFiClientSecure>client(new WiFiClientSecure);
  client->setInsecure();
  HTTPClient https;
  String url = "https://api.bilibili.com/x/relation/stat?vmid=" + id;
  Serial.print("[HTTPS] begin...bilibili\r\n");
  if (https.begin(*client, url)) {  // HTTPS
    // start connection and send HTTP header
    int httpCode = https.GET();
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTPS] GET... code: %d\r\n", httpCode);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.println(payload);
        DynamicJsonDocument  jsonBuffer(2048);
        deserializeJson(jsonBuffer, payload);
        JsonObject root = jsonBuffer.as<JsonObject>();

        String follower = root["data"]["follower"];
        data->follower = follower;
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\r\n", https.errorToString(httpCode).c_str());
      data->follower = "-1";
    }

    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\r\n");
    data->follower = "-1";
  }
}

void HeFeng::doUpdateCurr(HeFengCurrentData *data, String key, String location) { 

  std::unique_ptr<WiFiClientSecure>client(new WiFiClientSecure);
  client->setInsecure();
  HTTPClient https;
  String url = "https://free-api.heweather.net/s6/weather/now?lang=en&location=" + location + "&key=" + key;
  Serial.print("[HTTPS] begin...now\r\n");
  if (https.begin(*client, url)) {  // HTTPS
    // start connection and send HTTP header
    int httpCode = https.GET();
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTPS] GET... code: %d\r\n", httpCode);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.println(payload);
        DynamicJsonDocument  jsonBuffer(2048);
        deserializeJson(jsonBuffer, payload);
        JsonObject root = jsonBuffer.as<JsonObject>();

        String tmp = root["HeWeather6"][0]["now"]["tmp"];
        data->tmp = tmp;
        String fl = root["HeWeather6"][0]["now"]["fl"];
        data->fl = fl;
        String hum = root["HeWeather6"][0]["now"]["hum"];
        data->hum = hum;
        String wind_sc = root["HeWeather6"][0]["now"]["wind_sc"];
        data->wind_sc = wind_sc;
        String cond_code = root["HeWeather6"][0]["now"]["cond_code"];
        String cond_txt = root["HeWeather6"][0]["now"]["cond_txt"];
        data->cond_txt = cond_txt;
        data->icon_code = getIconCode(cond_code);

      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\r\n", https.errorToString(httpCode).c_str());
      data->tmp = "-1";
      data->fl = "-1";
      data->hum = "-1";
      data->wind_sc = "-1";
      data->cond_txt = "no network";
      data->icon_code = 39;
    }

    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\r\n");
    data->tmp = "-1";
    data->fl = "-1";
    data->hum = "-1";
    data->wind_sc = "-1";
    data->cond_txt = "no network";
    data->icon_code = 39;
  }

}

void HeFeng::doUpdateFore(HeFengForeData *data, String key, String location) { 

  std::unique_ptr<WiFiClientSecure>client(new WiFiClientSecure);
  client -> setInsecure();
  HTTPClient https;
  String url = "https://free-api.heweather.net/s6/weather/forecast?lang=en&location=" + location + "&key=" + key;
  Serial.print("[HTTPS] begin...forecast\r\n");
  if (https.begin(*client, url)) {  // HTTPS
    // start connection and send HTTP header
    int httpCode = https.GET();
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTPS] GET... code: %d\r\n", httpCode);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.println(payload);
        DynamicJsonDocument  jsonBuffer(8192);
        deserializeJson(jsonBuffer, payload);
        JsonObject root = jsonBuffer.as<JsonObject>();
        int i;
        for (i = 0; i < 3; i++) {
          String tmp_min = root["HeWeather6"][0]["daily_forecast"][i]["tmp_min"];
          data[i].tmp_min = tmp_min;
          String tmp_max = root["HeWeather6"][0]["daily_forecast"][i]["tmp_max"];
          data[i].tmp_max = tmp_max;
          String datestr = root["HeWeather6"][0]["daily_forecast"][i]["date"];
          data[i].datestr = datestr.substring(5, datestr.length());
          String cond_code = root["HeWeather6"][0]["daily_forecast"][i]["cond_code_d"];
          data[i].icon_code = getIconCode(cond_code);
        }
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\r\n", https.errorToString(httpCode).c_str());
      int i;
      for (i = 0; i < 3; i++) {
        data[i].tmp_min = "-1";
        data[i].tmp_max = "-1";
        data[i].datestr = "-1";
        data[i].icon_code = 39;
      }
    }
    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\r\n");
    int i;
    for (i = 0; i < 3; i++) {
      data[i].tmp_min = "-1";
      data[i].tmp_max = "-1";
      data[i].datestr = "-1";
      data[i].icon_code = 39;
    }
  }
}

int HeFeng::getIconCode(String cond_code) {
      if (cond_code == "100" || cond_code == "9006") {
        return 0;
      }
      if (cond_code == "999") {
        return 39;
      }
      if (cond_code == "104") {
        return 9;
      }
      if (cond_code == "500") {
        return 30;
      }
      if (cond_code == "503" || cond_code == "504" || cond_code == "507" || cond_code == "508") {
        return 28;
      }
      if (cond_code == "499" || cond_code == "901") {
        return 21;
      }
      if (cond_code == "103") {
        return 5;
      }
      if (cond_code == "502" || cond_code == "511" || cond_code == "512" || cond_code == "513") {
        return 31;
      }
      if (cond_code == "501" || cond_code == "509" || cond_code == "510" || cond_code == "514" || cond_code == "515") {
        return 30;
      }
      if (cond_code == "102") {
        return 4;
      }
      if (cond_code == "213") {
        return 4;
      }
      if (cond_code == "302" || cond_code == "303") {
        return 11;
      }
      if (cond_code == "305" || cond_code == "308" || cond_code == "309" || cond_code == "314" || cond_code == "399") {
        return 13;
      }
      if (cond_code == "306" || cond_code == "307" || cond_code == "310" || cond_code == "311" || cond_code == "312" || cond_code == "315" || cond_code == "316" || cond_code == "317" || cond_code == "318") {
        return 15;
      }
      if (cond_code == "200" || cond_code == "201" || cond_code == "202" || cond_code == "203" || cond_code == "204" || cond_code == "205" || cond_code == "206" || cond_code == "207" || cond_code == "208" || cond_code == "209" || cond_code == "210" || cond_code == "211" || cond_code == "212") {
        return 4;
      }
      if (cond_code == "300" || cond_code == "301") {
        return 10;
      }
      if (cond_code == "400" || cond_code == "408") {
        return 22;
      }
      if (cond_code == "407") {
        return 21;
      }
      if (cond_code == "401" || cond_code == "402" || cond_code == "403" || cond_code == "409" || cond_code == "410") {
        return 24;
      }
      if (cond_code == "304" || cond_code == "313" || cond_code == "404" || cond_code == "405" || cond_code == "406") {
        return 20;
      }
      if (cond_code == "101") {
        return 4;
      }
      return 39;
}


