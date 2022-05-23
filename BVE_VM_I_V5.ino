#include <Adafruit_MCP4725.h>
#include <EEPROM.h>
#include <Wire.h>
//更新履歴
//V5.1 電流計モードを絶対値表示に対応させた
//↓デバッグのコメント(//)を解除するとシリアルモニタでデバッグできます
//#define DEBUG

Adafruit_MCP4725 dac;
Adafruit_MCP4725 dac2;

String strbve = "";
uint16_t bve_speed = 0;
bool bve_door = 0;

//以下速度計補正値
int16_t spd_adj_010 = 150;
int16_t spd_adj_020 = 400;
int16_t spd_adj_030 = 680;
int16_t spd_adj_040 = 1010;
int16_t spd_adj_050 = 1330;
int16_t spd_adj_060 = 1650;
int16_t spd_adj_070 = 2000;
int16_t spd_adj_080 = 2340;
int16_t spd_adj_090 = 2680;
int16_t spd_adj_100 = 3020;
int16_t spd_adj_110 = 3340;
int16_t spd_adj_120 = 3650;
int16_t spd_adj_130 = 4000;
int16_t spd_adj_140 = 4095;
int16_t spd_adj_150 = 4095;
int16_t spd_adj_160 = 4095;

int16_t spd_limit = 120;
//以上速度計補正値

int16_t bve_current = 0;

int16_t curr_limit = 750;

//回生モード
bool curr_kaisei = true; //true:有効　false:無効
//計器モード
bool curr_mode = true; //true:電圧計 false:電流計

void setup() {

  pinMode(8, OUTPUT); //BVE_Door
  digitalWrite(8, 0);

  Serial.begin(115200);
  Serial1.begin(115200);
  dac.begin(0x60);
  dac2.begin(0x61);

  /*
    if (EEPROM.get(0, POT_N) < 0)EEPROM.put(0, 0);
    if (EEPROM.get(2, POT_EB) < 0)EEPROM.put(2, 512);
    if (EEPROM.get(4, notch_brk_num) < 0)EEPROM.put(4, 8);
    if (EEPROM.get(6, notch_brk_angl_max) < 0)EEPROM.put(6, 80);
    if (EEPROM.get(8, brk_eb_angl) < 0)EEPROM.put(8, 150);
    if (EEPROM.get(10, brk_full_angl) < 0)EEPROM.put(10, 165);
  */
  if (EEPROM.get(12, spd_adj_010) < 0)EEPROM.put(12, 150);
  if (EEPROM.get(14, spd_adj_020) < 0)EEPROM.put(14, 400);
  if (EEPROM.get(16, spd_adj_030) < 0)EEPROM.put(16, 680);
  if (EEPROM.get(18, spd_adj_040) < 0)EEPROM.put(18, 1010);
  if (EEPROM.get(20, spd_adj_050) < 0)EEPROM.put(20, 1330);
  if (EEPROM.get(22, spd_adj_060) < 0)EEPROM.put(22, 1650);
  if (EEPROM.get(24, spd_adj_070) < 0)EEPROM.put(24, 2000);
  if (EEPROM.get(26, spd_adj_080) < 0)EEPROM.put(26, 2340);
  if (EEPROM.get(28, spd_adj_090) < 0)EEPROM.put(28, 2680);
  if (EEPROM.get(30, spd_adj_100) < 0)EEPROM.put(30, 3020);
  if (EEPROM.get(32, spd_adj_110) < 0)EEPROM.put(32, 3340);
  if (EEPROM.get(34, spd_adj_120) < 0)EEPROM.put(34, 3650);
  if (EEPROM.get(36, spd_adj_130) < 0)EEPROM.put(36, 4000);
  if (EEPROM.get(38, spd_adj_140) < 0)EEPROM.put(38, 4095);
  if (EEPROM.get(40, spd_adj_150) < 0)EEPROM.put(40, 4095);
  if (EEPROM.get(42, spd_adj_160) < 0)EEPROM.put(42, 4095);
  if (EEPROM.get(44, spd_limit) < 0)EEPROM.put(44, 120);
  if (EEPROM.get(46, curr_kaisei) == -1)EEPROM.put(46, 1);
  if (EEPROM.get(48, curr_mode) == -1)EEPROM.put(48, 1);
  if (EEPROM.get(50, curr_limit) == -1)EEPROM.put(50, 750);
}

void loop() {
  //  Serial.println(bve_speed);
  // LOW = pressed, HIGH = not pressed
  //シリアルモニタが止まるのを防止するおまじない
  //BVEモードの時のみシリアル入力を受け付ける
  if (Serial.available()) {
    strbve = Serial.readStringUntil('\r');
    //シリアル設定モード
    if (strbve.startsWith("SET")) {
      //設定モード
      //速度計調整10km/h
      int8_t i = strbve.indexOf("SPD_010:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 8, i + 12).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_adj_010 = num;
          Serial.print("SET OK:SPD_010=");
          Serial.println(spd_adj_010);
          EEPROM.put(12, spd_adj_010);
          bve_speed = 100;
        }
      }
      //速度計調整20km/h
      i = strbve.indexOf("SPD_020:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 8, i + 12).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_adj_020 = num;
          Serial.print("SET OK:SPD_020=");
          Serial.println(spd_adj_020);
          EEPROM.put(14, spd_adj_020);
          bve_speed = 200;
        }
      }
      //速度計調整30km/h
      i = strbve.indexOf("SPD_030:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 8, i + 12).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_adj_030 = num;
          Serial.print("SET OK:SPD_030=");
          Serial.println(spd_adj_030);
          EEPROM.put(16, spd_adj_030);
          bve_speed = 300;
        }
      }
      //速度計調整40km/h
      i = strbve.indexOf("SPD_040:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 8, i + 12).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_adj_040 = num;
          Serial.print("SET OK:SPD_040=");
          Serial.println(spd_adj_040);
          EEPROM.put(18, spd_adj_040);
          bve_speed = 400;
        }
      }
      //速度計調整50km/h
      i = strbve.indexOf("SPD_050:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 8, i + 12).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_adj_050 = num;
          Serial.print("SET OK:SPD_050=");
          Serial.println(spd_adj_050);
          EEPROM.put(20, spd_adj_050);
          bve_speed = 500;
        }
      }
      //速度計調整60km/h
      i = strbve.indexOf("SPD_060:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 8, i + 12).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_adj_060 = num;
          Serial.print("SET OK:SPD_060=");
          Serial.println(spd_adj_060);
          EEPROM.put(22, spd_adj_060);
          bve_speed = 600;
        }
      }
      //速度計調整70km/h
      i = strbve.indexOf("SPD_070:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 8, i + 12).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_adj_070 = num;
          Serial.print("SET OK:SPD_070=");
          Serial.println(spd_adj_070);
          EEPROM.put(24, spd_adj_070);
          bve_speed = 700;
        }
      }
      //速度計調整80km/h
      i = strbve.indexOf("SPD_080:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 8, i + 12).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_adj_080 = num;
          Serial.print("SET OK:SPD_080=");
          Serial.println(spd_adj_080);
          EEPROM.put(26, spd_adj_080);
          bve_speed = 800;
        }
      }
      //速度計調整90km/h
      i = strbve.indexOf("SPD_090:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 8, i + 12).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_adj_090 = num;
          Serial.print("SET OK:SPD_090=");
          Serial.println(spd_adj_090);
          EEPROM.put(28, spd_adj_090);
          bve_speed = 900;
        }
      }
      //速度計調整100km/h
      i = strbve.indexOf("SPD_100:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 8, i + 12).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_adj_100 = num;
          Serial.print("SET OK:SPD_100=");
          Serial.println(spd_adj_100);
          EEPROM.put(30, spd_adj_100);
          bve_speed = 1000;
        }
      }
      //速度計調整110km/h
      i = strbve.indexOf("SPD_110:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 8, i + 12).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_adj_110 = num;
          Serial.print("SET OK:SPD_110=");
          Serial.println(spd_adj_110);
          EEPROM.put(32, spd_adj_110);
          bve_speed = 1100;
        }
      }

      //速度計調整120km/h
      i = strbve.indexOf("SPD_120:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 8, i + 12).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_adj_120 = num;
          Serial.print("SET OK:SPD_120=");
          Serial.println(spd_adj_120);
          EEPROM.put(34, spd_adj_120);
          bve_speed = 1200;
        }
      }
      //速度計調整130km/h
      i = strbve.indexOf("SPD_130:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 8, i + 12).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_adj_130 = num;
          Serial.print("SET OK:SPD_130=");
          Serial.println(spd_adj_130);
          EEPROM.put(36, spd_adj_130);
          bve_speed = 1300;
        }
      }
      //速度計調整140km/h
      i = strbve.indexOf("SPD_140:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 8, i + 12).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_adj_140 = num;
          Serial.print("SET OK:SPD_140=");
          Serial.println(spd_adj_140);
          EEPROM.put(38, spd_adj_140);
          bve_speed = 1400;
        }
      }
      //速度計調整150km/h
      i = strbve.indexOf("SPD_150:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 8, i + 12).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_adj_150 = num;
          Serial.print("SET OK:SPD_150=");
          Serial.println(spd_adj_150);
          EEPROM.put(40, spd_adj_150);
          bve_speed = 1500;
        }
      }
      //速度計調整160km/h
      i = strbve.indexOf("SPD_160:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 8, i + 12).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_adj_160 = num;
          Serial.print("SET OK:SPD_160=");
          Serial.println(spd_adj_160);
          EEPROM.put(42, spd_adj_160);
          bve_speed = 1600;
        }
      }
      //最高速度設定
      i = strbve.indexOf("SPD_LIMIT:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 10, i + 13).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_limit = num;
          Serial.print("SET OK:SPD_LIMIT=");
          Serial.println(spd_limit);
          EEPROM.put(44, spd_limit);
          bve_speed = spd_limit * 10;
        }
      }

      //速度計設定読み出し
      i = strbve.indexOf("SPD_READ:");
      if (i > 0) {
        Serial.println("SET READ:SPD");
        Serial.print("010=");
        Serial.println(spd_adj_010);
        Serial.print("020=");
        Serial.println(spd_adj_020);
        Serial.print("030=");
        Serial.println(spd_adj_030);
        Serial.print("040=");
        Serial.println(spd_adj_040);
        Serial.print("050=");
        Serial.println(spd_adj_050);
        Serial.print("060=");
        Serial.println(spd_adj_060);
        Serial.print("070=");
        Serial.println(spd_adj_070);
        Serial.print("080=");
        Serial.println(spd_adj_080);
        Serial.print("090=");
        Serial.println(spd_adj_090);
        Serial.print("100=");
        Serial.println(spd_adj_100);
        Serial.print("110=");
        Serial.println(spd_adj_110);
        Serial.print("120=");
        Serial.println(spd_adj_120);
        Serial.print("130=");
        Serial.println(spd_adj_130);
        Serial.print("140=");
        Serial.println(spd_adj_140);
        Serial.print("150=");
        Serial.println(spd_adj_150);
        Serial.print("160=");
        Serial.println(spd_adj_160);
        Serial.print("Limit=");
        Serial.println(spd_limit);
      }

      //回生モード
      i = strbve.indexOf("CURR_KAISEI:");
      if (i > 0) {
        int8_t on = strbve.indexOf("ON");
        int8_t off = strbve.indexOf("OFF");
        if ( on < 0 && off < 0 ) {
          Serial.println("SET NG");
        } else {
          if ( on > 0 ) {
            curr_kaisei = 1;
            Serial.println("SET OK:CURR_KAISEI:ON");
          } else {
            curr_kaisei = 0;
            Serial.println("SET OK:CURR_KAISEI:OFF");
          }
          EEPROM.put(46, curr_kaisei);
        }
      }
      //計器モード
      i = strbve.indexOf("CURR_MODE:");
      if (i > 0) {
        int8_t V = strbve.indexOf("V");
        int8_t I = strbve.indexOf("I");
        if (V < 0 && I < 0) {
          Serial.println("SET NG");
        } else {
          if (V > 0) {
            curr_mode = 1;
            Serial.println("SET OK:CURR_MODE:V");
          } else {
            curr_mode = 0;
            Serial.println("SET OK:CURR_MODE:I");
          }
          EEPROM.put(48, curr_mode);
        }
      }

      //最高速度設定
      i = strbve.indexOf("CURR_LIMIT:");
      if (i > 0) {
        uint16_t num = strbve.substring(i + 10, i + 13).toInt();
        if (num == 0) {
          Serial.println("SET NG");
        } else {
          spd_limit = num;
          Serial.print("SET OK:CURR_LIMIT=");
          Serial.println(curr_limit);
          EEPROM.put(50, curr_limit);
        }
      }
    } else {
      //通常モード
      //速度抽出
      bve_speed = strbve.substring(0, 4).toInt();
      //ドア状態抽出
      bve_door = strbve.substring(5, 6).toInt();
      //電流抽出
      bve_current = strbve.substring(7, 12).toInt();
    }

    //速度計のリミットを適用
    if (bve_speed > ( spd_limit * 10 )) {
      bve_speed = ( spd_limit * 10 );
    }

    //速度計補正
    if (bve_speed < 100) {
      dac.setVoltage(map(bve_speed, 0 , 100 , 0, spd_adj_010), false);
    } else if (bve_speed < 200) {
      dac.setVoltage(map(bve_speed, 100 , 200 , spd_adj_010, spd_adj_020), false);
    } else if (bve_speed < 300) {
      dac.setVoltage(map(bve_speed, 200 , 300 , spd_adj_020, spd_adj_030), false);
    } else if (bve_speed < 400) {
      dac.setVoltage(map(bve_speed, 300 , 400 , spd_adj_030, spd_adj_040), false);
    } else if (bve_speed < 500) {
      dac.setVoltage(map(bve_speed, 400 , 500 , spd_adj_040, spd_adj_050), false);
    } else if (bve_speed < 600) {
      dac.setVoltage(map(bve_speed, 500 , 600 , spd_adj_050, spd_adj_060), false);
    } else if (bve_speed < 700) {
      dac.setVoltage(map(bve_speed, 600 , 700 , spd_adj_060, spd_adj_070), false);
    } else if (bve_speed < 800) {
      dac.setVoltage(map(bve_speed, 700 , 800 , spd_adj_070, spd_adj_080), false);
    } else if (bve_speed < 900) {
      dac.setVoltage(map(bve_speed, 800 , 900 , spd_adj_080, spd_adj_090), false);
    } else if (bve_speed < 1000) {
      dac.setVoltage(map(bve_speed, 900 , 1000 , spd_adj_090, spd_adj_100), false);
    } else if (bve_speed < 1100) {
      dac.setVoltage(map(bve_speed, 1000 , 1100 , spd_adj_100, spd_adj_110), false);
    } else if (bve_speed < 1200) {
      dac.setVoltage(map(bve_speed, 1100 , 1200 , spd_adj_110, spd_adj_120), false);
    } else if (bve_speed < 1300) {
      dac.setVoltage(map(bve_speed, 1200 , 1300 , spd_adj_120, spd_adj_130), false);
    } else if (bve_speed < 1400) {
      dac.setVoltage(map(bve_speed, 1300 , 1400 , spd_adj_130, spd_adj_140), false);
    } else if (bve_speed < 1500) {
      dac.setVoltage(map(bve_speed, 1400 , 1500 , spd_adj_140, spd_adj_150), false);
    } else {
      dac.setVoltage(map(bve_speed, 1500 , 1600 , spd_adj_160, 4095), false);
    }

    //電流計
    if (!curr_kaisei && (bve_current < 0) ) {
      bve_current = 0;
    }
    if (curr_mode) {
      int v = 1500 - (int)(bve_current * 0.5);
      dac2.setVoltage(map(v, 0 , curr_limit , 0, 4095), false);
    } else {
      int current = abs(bve_current);
      if(current > curr_limit){
        current = curr_limit;
      }
      dac2.setVoltage(map(current, 0 , curr_limit , 0, 4095), false);
    }

    //戸閉灯指示
    digitalWrite(8, !bve_door);

    //Serial1デバッグ用
    Serial1.print("BVE Speed:");
    Serial1.println(bve_speed);
    Serial1.print(" Door:");
    Serial1.println(bve_door);

  }

  /*//読み残しは捨てる
    if (Serial.available() && modeBVE) {
    Serial.readStringUntil('\n');
    }*/

  delay(10);

}
