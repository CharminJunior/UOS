// UOS.cpp
#include "UOS.h"  // ต้องแก้ด้วยชื่อไฟล์จริง

// นิยามตัวแปร global
My_print p;
#if !defined(ARDUINO_ARCH_RP2040)
  My_eerom E;
#endif

int H1 = 0;
bool Use_Serial_True = 0;
char Data_EEPROM[len_Data_Buffer];
int TData, Len_Data;

/* ----------------------------
   My_print Class Implementation
   ---------------------------- */

// constructor
My_print::My_print() {
  // ไม่ต้องทำอะไรพิเศษ
}

// return Use_Serial_True
bool My_print::init() {
  return Use_Serial_True;
}

// ฟังก์ชัน non‑template
void My_print::b(long baud) {
  Serial.begin(baud);
  unsigned long start = millis();
  while (!Serial) {
    if (millis() - start >= 200) { break; }
  }
  Use_Serial_True = 1;
}

void My_print::b(long baud, bool Serial_bit) {
  Serial.begin(baud);
  unsigned long start = millis();
  if(Serial_bit) {
    while (!Serial) {
      if (millis() - start >= 200) { break; }
    }
  }
  Use_Serial_True = 1;
}

/* ----------------------------
   I/O Helpers Implementation
   ---------------------------- */
bool DRead(const uint8_t pin, const uint8_t SetPin) {
  setPinMode(pin, SetPin);
  return digitalRead(pin);
}

bool btn(const uint8_t pin) {
  return !(bool(DRead(pin, 3)));
}

int ARead(const uint8_t ch) {
  return analogRead(ch);
}

void pwm(const uint8_t pin, const uint8_t value) {
  pinMode(pin, OUTPUT);
  analogWrite(pin, value);
}

void outD(const uint8_t pin, const bool value) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, value ? HIGH : LOW);
}

/* ----------------------------
   EEPROM Helpers Implementation
   ---------------------------- */
#if !defined(ARDUINO_ARCH_RP2040)
  bool isValidPosition(int pos, int length) {
    for (int i = pos; i < pos + length; i++) {
      char c = EEPROM.read(i);
      if (c == '@' || c == '#' || c == '&') {
        return false; // เจอตัวอักษรพิเศษ
      }
    }
    return true;
  }

  bool isNumber(String str) {
    if (str.length() == 0) return false;

    int start = 0;
    if (str[0] == '-' || str[0] == '+') { // มีเครื่องหมายบวก/ลบ
      if (str.length() == 1) return false; // มีแค่เครื่องหมาย ไม่มีเลข
      start = 1;
    }

    bool hasDecimal = false;
    for (int i = start; i < str.length(); i++) {
      if (str[i] == '.') {
        if (hasDecimal) return false; // เจอจุดทศนิยมซ้ำ
        hasDecimal = true;
      }
      else if (!isDigit(str[i])) {
        return false; // ไม่ใช่ตัวเลข
      }
    }
    return true;
  }

  int findValidPosition(String Compilation) {
    //int T3 = -1; // ตำแหน่งที่จะเขียน
    int maxPos = EEPROM.length() - Compilation.length();
    int trialCount = 0;  // ป้องกันวนไม่จบ (กรณีไม่มีตำแหน่งว่าง)
    const int maxTrials = 100; 

    while (trialCount < maxTrials) {
      int pos = random(0, maxPos + 1); // +1 เพราะช่วงบน exclusive
      if (isValidPosition(pos, Compilation.length())) {
        //T3 = pos;
        return pos;
      }
      trialCount++;
    }

    // ถ้าหมดรอบแล้วยังหาไม่ได้คืน false
    return -1;
  }

  My_eerom::My_eerom() {
    // ไม่มีอะไร
  }

  #if !defined(ESP8266) && !defined(ESP32)
    void My_eerom::begin() {
      // AVR / UNO ใช้ได้
    }
  #else
    void My_eerom::begin(size_t size) {
      #if defined(ESP8266) || defined(ESP32)
        EEPROM.begin(size);
      #endif
    }
  #endif

  int My_eerom::GEUP() {
    int count = 0;
    int size = EEPROM.length();

    for (int i = 0; i < size; i++) {
      byte val = EEPROM.read(i);
      if (val != 0xFF) {
        count++;
      }
    }
    return count; // คืนจำนวนไบต์ที่มีข้อมูลจริง
  }

  float My_eerom::GEUP_F() {
    int validCount = 0;
    int size = EEPROM.length();

    for (int i = 0; i < size; i++) {
      if (EEPROM.read(i) != 0xFF) {
        validCount++;
      }
    }

    float percent = (validCount * 100.0) / size;
    return percent; // เปอร์เซ็นต์ข้อมูลจริง
  }

  void My_eerom::clear() {
    if(!Use_Serial_True) {
      p.b(Starting_serial, true);
    } else {
      return;
    }
    if(GEUP_F() != 0) {
      int eepromSize = EEPROM.length(); // ขนาด EEPROM ของบอร์ด

      p.text("Size EEPROM = ",eepromSize,"\n");
      p.text("Perform cleaning...\n");

      int n1 = 0;

      for (int i = 0; i < eepromSize; i++) {
        EEPROM.write(i, 0xFF);  // หรือจะเขียน 0 ก็ได้
        //n1 = (i*100)/eepromSize;
        //lod(i,eepromSize);
      }
      // EEPROM.commit();
      #if defined(ESP8266) || defined(ESP32)
        EEPROM.commit();  // จำเป็นบน ESP
      #endif

      p.text("EEPROM is clear!\n");
    } else {
      p.text("EEPROM is clear!\n");
    }
  }

  void My_eerom::D(String name) {
    uint32_t X1;
    R(name, X1);

    if(H1 != -1) {
      int X2 = H1;
      for(int i=0; i <= X1; i++) {
        EEPROM.write(i+X2, 0xFF);  // หรือจะเขียน 0 ก็ได้
      }
      // EEPROM.commit();
      #if defined(ESP8266) || defined(ESP32)
      EEPROM.commit();  // จำเป็นบน ESP
      #endif
    }
  }

  void My_eerom::W(String name, Result Text) {
    if(R(name) != Text) {
      D(name);
      
      String in2;
      String Compilation;
      int T1, T2;
      String in;

      if (Text.isint()) {
        in2 = "#" + name;  // เป็นตัวเลข int
      } else if (Text.islong()) {
        in2 = "@" + name;  // เป็นตัวเลข long
      } else if (Text.isfloat()) {
        in2 = "$" + name;  // เป็นตัวเลข float
      } else if (Text.isstr()) {
        in2 = "%" + name;  // เป็นข้อความ string
      } else {
        // ไม่รู้จักชนิดข้อมูล
        // p.text("Error: Unknown data type\n");
        return;
      }

      in = Text.toString();
      Compilation = in2 + "&" + in + "&";

      // p.text("Compilation : ",Compilation,"\n");

      //T1 = random(0, EEPROM.length() - Compilation.length());
      //T1 = 1; // EEPROM.length() - Compilation.length();
      T1 = findValidPosition(Compilation);
      T2 = 1;

      // วนตามความยาว Compilation
      for (int i = 0; i < Compilation.length(); i++) {
        EEPROM.write(T1 + T2, Compilation.charAt(T2 - 1));
        T2++;
      }
      #if defined(ESP8266) || defined(ESP32)
        EEPROM.commit();  // จำเป็นบน ESP
      #endif
    }
  }

  Result My_eerom::R(String nane) {
    long addr = 0;
    String Lite = "";
    Result Vode;

    // วนหา '@', '#', '$', '%' ก่อน
    while (addr < EEPROM.length()) {
      char ch = (char)EEPROM.read(addr);
      if (ch != '#' && ch != '@' && ch != '$' && ch != '%') {
        addr++;
        continue;
      }

      H1 = addr;
      addr++; // ข้ามตัวอักษรชนิดข้อมูล

      // อ่านชื่อจนเจอ '&'1
      Lite = "";
      while (addr < EEPROM.length()) {
        char c = (char)EEPROM.read(addr);
        if (c == '&') {
          addr++;
          break;
        }
        Lite += c;
        addr++;
      }

      // ชื่อไม่ตรง → ข้ามค่าข้อมูล
      if (Lite != nane) {
        while (addr < EEPROM.length()) {
          if ((char)EEPROM.read(addr) == '&') {
            addr++;
            break;
          }
          addr++;
        }
        continue;
      }

      // ตัวแปร → อ่านค่าข้อมูลจนเจอ '&'2
      String data = "";
      while (addr < EEPROM.length()) {
        char c = (char)EEPROM.read(addr);
        if (c == '&') {
          addr++;
          break;
        }
        data += c;
        addr++;
      }

      // Vode = data; // เก็บชื่อ (ถ้าต้องการ)

      // แปลงค่าและเก็บใน Result
      if (ch == '@') {           // long
        Vode.type = Result::LONG;
        Vode.lVal = data.toInt();
      } else if (ch == '#') {    // int
        Vode.type = Result::INT;
        Vode.iVal = data.toInt();
      } else if (ch == '$') {    // float
        Vode.type = Result::FLOAT;
        Vode.fVal = data.toFloat();
      } else if (ch == '%') {    // string
        Vode.type = Result::STRING;
        Vode.sVal = data;
      }

      return Vode; // คืนค่าข้อมูล
    }

    // ไม่เจอ → คืนค่า Result แบบ type = STRING และ sVal = "-1"
    Result empty;
    empty.type = Result::STRING;
    empty.sVal = "-1";
    H1 = -1;
    return empty;
  }

  Result My_eerom::R(String nane, uint32_t &rawLength) {
    long addr = 0;
    String Lite = "";
    Result Vode;
    rawLength = 0;

    while (addr < EEPROM.length()) {
      // หา '@', '#', '$', '%' ก่อน
      char ch = (char)EEPROM.read(addr);
      if (ch != '#' && ch != '@' && ch != '$' && ch != '%') {
        addr++;
        continue;
      }
      long startAddr = addr;  // เก็บตำแหน่งเริ่มต้นของข้อมูลดิบ
      H1 = addr;
      addr++; // ข้าม '@', '#', '$', '%'

      // อ่านชื่อ (จนเจอ '&')
      Lite = "";
      while (addr < EEPROM.length()) {
        char c = (char)EEPROM.read(addr);
        if (c == '&') { addr++; break; }
        Lite += c;
        addr++;
      }

      // ถ้าไม่ตรงชื่อ ให้ข้ามข้อมูล (ค่าข้อมูล) ไปจนเจอ '&' ตัวที่สอง
      if (Lite != nane) {
        while (addr < EEPROM.length()) {
          if ((char)EEPROM.read(addr) == '&') { addr++; break; }
          addr++;
        }
        continue; // อ่านชุดถัดไป
      }

      // ชื่อตรง → อ่านค่าข้อมูล (จนเจอ '&')
      // Vode = "";
      String data = "";
      while (addr < EEPROM.length()) {
        char c = (char)EEPROM.read(addr);
        if (c == '&') { addr++; break; }
        data += c;
        addr++;
      }

      // คำนวณความยาวข้อมูลดิบ ตั้งแต่ตำแหน่ง '@', '#', '$', '%' ถึง & ตัวที่สอง
      rawLength = addr - startAddr;

      // แปลงค่าและเก็บใน Result
      if (ch == '@') {           // long
        Vode.type = Result::LONG;
        Vode.lVal = data.toInt();
      } else if (ch == '#') {    // int
        Vode.type = Result::INT;
        Vode.iVal = data.toInt();
      } else if (ch == '$') {    // float
        Vode.type = Result::FLOAT;
        Vode.fVal = data.toFloat();
      } else if (ch == '%') {    // string
        Vode.type = Result::STRING;
        Vode.sVal = data;
      }

      return Result(Vode);  // <-- คืนค่า Result แทน String
    }

    rawLength = 0;
    H1 = -1;
    return Result(String("-1")); // <-- คืนค่า Result
  }

  int My_eerom::H() {
    return H1;
  }

  uint16_t My_eerom::Data_extraction() {
    if(!Use_Serial_True) {
      p.b(Starting_serial, true);
    } else {
      return 0;
    }
    if(E.GEUP() == 0) { 
      p.text("No data in EEPROM\n");
      return 0;
    }
    Len_Data = 0;
    p.text("\nStart Data_extraction\n");
    for(int i = 0; i < EEPROM.length(); i++) {
      if(EEPROM.read(i) != 0xFF) {
        for(int j = 0; j < len_Data_Buffer; j++) {
          Data_EEPROM[j] = ' ';
        }
        Data_EEPROM[len_Data_Buffer - 1] = '\0';
        TData = 0;
        while(EEPROM.read(i) != 0xFF) {
          Data_EEPROM[TData] = EEPROM.read(i);
          i++;
          TData++;
        }
        p.text("Data Buffer : ", Data_EEPROM, "\n");
        Len_Data++;
      }
    }
    p.text("Data extraction end\n");
    return Len_Data;
  }

  uint32_t My_eerom::Search(String name) {
    R(name);
    return H1;
  }

  uint32_t My_eerom::Search(String name, uint32_t &Len_name) {
    R(name, Len_name);
    return H1;
  }
#endif

/* ----------------------------
   Pin-Mode Abstraction Implementation
   ---------------------------- */
void setPinMode(uint8_t pin, uint8_t mode) {
  const uint8_t yio[] = { 0, OUTPUT, INPUT, INPUT_PULLUP };
  if (mode >= 1 && mode <= 3) {
    pinMode(pin, yio[mode]);
  }
}

/* ----------------------------
   Serial Input Helpers Implementation
   ---------------------------- */
// อ่านจนเจอ '\n' หรือจนบัฟเฟอร์เต็ม (เหลือ 1 byte 0-terminator)
char* input(const char* prompt) {
  if(!Use_Serial_True) {
    p.b(Starting_serial, true);
  } else {
    return 0;
  }
  // จองบัฟเฟอร์ภายในฟังก์ชัน (ปรับขนาดได้ตามต้องการ)
  static char buf[64];
  size_t idx = 0;
  p.text(prompt," >>> ");
  while (true) {
    if (Serial.available() > 0) {
      char c = Serial.read();
      if (c == '\r') continue;            // ข้าม CR
      if (c == '\n' || idx >= sizeof(buf)-1) break;
      buf[idx++] = c;
    }
  }
  buf[idx] = '\0';
  p.text(buf,"\n");
  return buf;
}

char* input() {
  if(!Use_Serial_True) {
    p.b(Starting_serial, true);
  } else {
    return 0;
  }
  // จองบัฟเฟอร์ภายในฟังก์ชัน (ปรับขนาดได้ตามต้องการ)
  static char buf[64];
  size_t idx = 0;
  while (true) {
    if (Serial.available() > 0) {
      char c = Serial.read();
      if (c == '\r') continue;            // ข้าม CR
      if (c == '\n' || idx >= sizeof(buf)-1) break;
      buf[idx++] = c;
    }
  }
  buf[idx] = '\0';
  return buf;
}

double fx(const char* num) {
  return te_interp(num, 0);
}