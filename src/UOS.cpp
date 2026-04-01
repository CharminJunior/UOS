// UOS.cpp
#include "UOS.h"  // ต้องแก้ด้วยชื่อไฟล์จริง

// นิยามตัวแปร global
My_print p;
#if !defined(ARDUINO_ARCH_RP2040)
  My_eerom E;
#endif

int H1 = 0;
bool Use_Serial_True = 0;
bool RandomSeeded_True = 0;
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
    if (millis() - start >= 200) { return; }
  }
  Use_Serial_True = 1;
  if(RandomSeeded_True == 0) {
    randomSeed(analogRead(A0)); // ใช้ค่าอะนาล็อกสุ่มเพื่อความหลากหลาย
    RandomSeeded_True = 1;
  }
}

void My_print::b(long baud, bool Serial_bit) {
  Serial.begin(baud);
  unsigned long start = millis();
  if(Serial_bit) {
    while (!Serial) {
      if (millis() - start >= 200) { return; }
    }
  }
  Use_Serial_True = 1;
  if(RandomSeeded_True == 0) {
    randomSeed(analogRead(A0)); // ใช้ค่าอะนาล็อกสุ่มเพื่อความหลากหลาย
    RandomSeeded_True = 1;
  }
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

  bool isNumber(char* str) {
    if (strlen(str) == 0) return false;

    int start = 0;
    if (str[0] == '-' || str[0] == '+') { // มีเครื่องหมายบวก/ลบ
      if (strlen(str) == 1) return false; // มีแค่เครื่องหมาย ไม่มีเลข
      start = 1;
    }

    bool hasDecimal = false;
    for (int i = start; i < strlen(str); i++) {
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

  int findValidPosition(const char* Compilation) {
    if(RandomSeeded_True == 0) {
      randomSeed(analogRead(A0)); // ใช้ค่าอะนาล็อกสุ่มเพื่อความหลากหลาย
      RandomSeeded_True = 1;
    }
    // ใช้ strlen() แทน .length() เพราะ Compilation เป็น char*
    int compLen = strlen(Compilation);
    int maxPos = EEPROM.length() - compLen;
    
    if (maxPos < 0) return -1; // ป้องกันกรณีข้อความยาวกว่า EEPROM

    int trialCount = 0;  
    const int maxTrials = 100; 

    while (trialCount < maxTrials) {
      int pos = random(0, maxPos + 1); 
      // ส่งความยาวที่คำนวณได้จาก strlen ไปให้ isValidPosition
      if (isValidPosition(pos, compLen)) {
        return pos;
      }
      trialCount++;
    }

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
    uint32_t count = 0;

    for (uint32_t i = 0; i < EEPROM.length(); i++) {
      if (EEPROM.read(i) != 0xFF) {
        count++;
      }
    }
    return count; // คืนจำนวนไบต์ที่มีข้อมูลจริง
  }

  uint8_t My_eerom::GEUP_T() {
    uint32_t count = 0;

    for (uint32_t i = 0; i < EEPROM.length(); i++) {
      if (EEPROM.read(i) != 0xFF) {
        count++;
      }
    }
    return (uint8_t)((count * 100) / EEPROM.length()); // 0-100 แบบ int ของข้อมูลจริง
  }

  float My_eerom::GEUP_F() {
    uint32_t validCount = 0;

    for (uint32_t i = 0; i < EEPROM.length(); i++) {
      if (EEPROM.read(i) != 0xFF) {
        validCount++;
      }
    }

    return (validCount * 100.0) / EEPROM.length(); // เปอร์เซ็นต์ข้อมูลจริง
  }

  void My_eerom::clear() {
    if(!Use_Serial_True) {
      p.b(Starting_serial, true);
    }
    if(GEUP() != 0) {
      uint32_t eepromSize = EEPROM.length(); // ขนาด EEPROM ของบอร์ด

      p.text("Size EEPROM = ",eepromSize,"\n");
      p.text("Perform cleaning...\n");

      uint32_t n1 = 0;

      for (uint32_t i = 0; i < eepromSize; i++) {
        if(EEPROM.read(i) != 0xFF) {
          EEPROM.write(i, 0xFF);  // หรือจะเขียน 0 ก็ได้
        }
      }
      #if defined(ESP8266) || defined(ESP32)
        EEPROM.commit();  // จำเป็นบน ESP
      #endif

      
    }
    p.text("EEPROM is clear!\n", " - Total ", GEUP(), " bytes of data\n");
  }

  void My_eerom::D(char* name) {
    uint32_t X1;
    R(name, X1);

    if(H1 != -1) {
      int X2 = H1;
      for(int i=0; i <= X1; i++) {
        EEPROM.write(i+X2, 0xFF);  // หรือจะเขียน 0 ก็ได้
      }
      #if defined(ESP8266) || defined(ESP32)
      EEPROM.commit();  // จำเป็นบน ESP
      #endif
    }
  }

  void My_eerom::W(char* name, Result Text) {
      if(R(name) != Text) {
        D(name);
        
        char typeChar;
        if (Text.isint())      typeChar = '#';
        else if (Text.islong())  typeChar = '@';
        else if (Text.isfloat()) typeChar = '$';
        else if (Text.isstr())   typeChar = '%';
        else return;

        // สร้าง String สำหรับเก็บค่าที่จะเขียนลง EEPROM
        // รูปแบบ: #name&value&
        String in = Text.toString(); 
        char compilation[64]; // ปรับขนาดตามความเหมาะสม
        snprintf(compilation, sizeof(compilation), "%c%s&%s&", typeChar, name, in.c_str());

        int T1 = findValidPosition(compilation); // ต้องไปแก้ findValidPosition ให้รับ char* ด้วย
        if (T1 != -1) {
            // p.text("Writing to EEPROM at position ", T1, ": ", String(compilation), '\n');
            for (int i = 0; i < strlen(compilation); i++) {
              EEPROM.write(T1 + 1 + i, compilation[i]);
            }
            #if defined(ESP8266) || defined(ESP32)
              EEPROM.commit();
            #endif
        }
      }
  }

  Result My_eerom::R(char* nane) {
    long addr = 0;
    int eepromSize = EEPROM.length();
    Result Vode;

    while (addr < eepromSize) {
      // 1. หา '@', '#', '$', '%'
      char ch = (char)EEPROM.read(addr);
      if (ch != '#' && ch != '@' && ch != '$' && ch != '%') {
        addr++;
        continue;
      }

      H1 = addr; // เก็บตำแหน่งที่เจอ
      addr++;    // ข้ามตัวอักษรชนิดข้อมูล

      // 2. เปรียบเทียบชื่อ "ทีละตัวอักษร" โดยไม่ต้องสร้าง String หรือ Buffer ใดๆ
      bool nameMatch = true;
      int nameIdx = 0;
      
      while (addr < eepromSize) {
        char c = (char)EEPROM.read(addr);
        addr++;
        
        if (c == '&') {
          // ถ้า EEPROM เจอ '&' แล้ว แต่คำค้นหา (nane) ยังไม่จบ (\0) แปลว่าชื่อไม่ตรง
          if (nane[nameIdx] != '\0') {
            nameMatch = false; 
          }
          break; // จบการอ่านชื่อ
        }
        
        // ถ้าตัวอักษรไม่ตรงกัน ก็จำไว้ว่าไม่ตรง (แต่ยังต้องวนอ่านให้จบถึง '&')
        if (nane[nameIdx] != c) {
          nameMatch = false;
        }
        nameIdx++;
      }

      // 3. ถ้าชื่อไม่ตรง → วนข้ามค่าข้อมูลชุดนี้ไปเลย
      if (!nameMatch) {
        while (addr < eepromSize) {
          if ((char)EEPROM.read(addr) == '&') {
            addr++;
            break;
          }
          addr++;
        }
        continue; // กลับไปเริ่มเช็ค Address ถัดไป
      }

      // 4. ถ้าชื่อตรง! → ดึงข้อมูลมาใส่ Buffer ธรรมดา (หลีกเลี่ยงการบวก String)
      // จองพื้นที่ 32 bytes (เก็บเลข หรือข้อความสั้นๆ ได้สบายๆ)
      char dataBuf[32]; 
      int dIdx = 0;
      
      while (addr < eepromSize) {
        char c = (char)EEPROM.read(addr);
        addr++;
        if (c == '&') break;
        
        if (dIdx < (int)sizeof(dataBuf) - 1) {
          dataBuf[dIdx++] = c;
        }
      }
      dataBuf[dIdx] = '\0'; // ปิดท้ายข้อความให้สมบูรณ์

      // 5. แปลงค่าด้วยคำสั่งมาตรฐานของ C (รวดเร็วและไม่กิน Heap RAM)
      if (ch == '@') {           // long
        Vode.type = Result::LONG;
        Vode.lVal = atol(dataBuf);
      } else if (ch == '#') {    // int
        Vode.type = Result::INT;
        Vode.iVal = atoi(dataBuf);
      } else if (ch == '$') {    // float
        Vode.type = Result::FLOAT;
        Vode.fVal = atof(dataBuf);
      } else if (ch == '%') {    // string
        Vode.type = Result::STRING;
        Vode.sVal = dataBuf;     // <--- จังหวะนี้จะมีการแปลงเป็น String แค่ครั้งเดียว เพื่อใส่ใน struct
      }

      return Vode;
    }

    // ไม่เจอข้อมูล คืนค่า Default
    H1 = -1;
    Result empty;
    empty.type = Result::STRING;
    empty.sVal = "-1";
    return empty;
  }

  Result My_eerom::R(char* nane, int &rawLength) {
      long addr = 0;
      char tempName[32]; // บัฟเฟอร์สำหรับพักชื่อที่อ่านจาก EEPROM (ปรับขนาดได้ตามใจชอบ)
      Result Vode;
      rawLength = 0;

      int eepromSize = EEPROM.length();

      while (addr < eepromSize) {
        // 1. หาตัวบ่งชี้ชนิดข้อมูล '@', '#', '$', '%'
        char ch = (char)EEPROM.read(addr);
        if (ch != '#' && ch != '@' && ch != '$' && ch != '%') {
          addr++;
          continue;
        }
        
        long startAddr = addr;  
        H1 = addr; // เก็บตำแหน่ง Address ที่พบ
        addr++; 

        // 2. อ่านชื่อจาก EEPROM มาเก็บใน tempName จนเจอ '&'
        int idx = 0;
        while (addr < eepromSize) {
          char c = (char)EEPROM.read(addr);
          if (c == '&') { 
            addr++; 
            break; 
          }
          if (idx < sizeof(tempName) - 1) {
            tempName[idx++] = c;
          }
          addr++;
        }
        tempName[idx] = '\0'; // ปิดท้าย string

        // 3. เปรียบเทียบชื่อด้วย strcmp
        if (strcmp(tempName, nane) != 0) {
          // ถ้าชื่อไม่ตรง ให้ข้าม (Skip) ค่าข้อมูลไปจนเจอ '&' ตัวที่สอง
          while (addr < eepromSize) {
            if ((char)EEPROM.read(addr) == '&') { 
              addr++; 
              break; 
            }
            addr++;
          }
          continue; 
        }

        // 4. ถ้าชื่อตรง → อ่านค่าข้อมูล (ส่วนนี้ใช้ String data ได้เพราะต้องส่งเข้า Result)
        String data = "";
        while (addr < eepromSize) {
          char c = (char)EEPROM.read(addr);
          if (c == '&') { 
            addr++; 
            break; 
          }
          data += c;
          addr++;
        }

        rawLength = addr - startAddr; // คำนวณความยาวข้อมูลดิบ

        // 5. แปลงค่าตามชนิดที่ระบุไว้ตอนต้น
        if (ch == '@') {           
          Vode.type = Result::LONG;
          Vode.lVal = data.toInt();
        } else if (ch == '#') {    
          Vode.type = Result::INT;
          Vode.iVal = data.toInt();
        } else if (ch == '$') {    
          Vode.type = Result::FLOAT;
          Vode.fVal = data.toFloat();
        } else if (ch == '%') {    
          Vode.type = Result::STRING;
          Vode.sVal = data;
        }

        return Vode; 
      }

      rawLength = 0;
      H1 = -1;
      return Result(String("-1")); 
  }

  Result My_eerom::R(char* nane, uint32_t &rawLength) {
      long addr = 0;
      char tempName[32]; // บัฟเฟอร์สำหรับพักชื่อที่อ่านจาก EEPROM (ปรับขนาดได้ตามใจชอบ)
      Result Vode;
      rawLength = 0;

      int eepromSize = EEPROM.length();

      while (addr < eepromSize) {
        // 1. หาตัวบ่งชี้ชนิดข้อมูล '@', '#', '$', '%'
        char ch = (char)EEPROM.read(addr);
        if (ch != '#' && ch != '@' && ch != '$' && ch != '%') {
          addr++;
          continue;
        }
        
        long startAddr = addr;  
        H1 = addr; // เก็บตำแหน่ง Address ที่พบ
        addr++; 

        // 2. อ่านชื่อจาก EEPROM มาเก็บใน tempName จนเจอ '&'
        int idx = 0;
        while (addr < eepromSize) {
          char c = (char)EEPROM.read(addr);
          if (c == '&') { 
            addr++; 
            break; 
          }
          if (idx < sizeof(tempName) - 1) {
            tempName[idx++] = c;
          }
          addr++;
        }
        tempName[idx] = '\0'; // ปิดท้าย string

        // 3. เปรียบเทียบชื่อด้วย strcmp
        if (strcmp(tempName, nane) != 0) {
          // ถ้าชื่อไม่ตรง ให้ข้าม (Skip) ค่าข้อมูลไปจนเจอ '&' ตัวที่สอง
          while (addr < eepromSize) {
            if ((char)EEPROM.read(addr) == '&') { 
              addr++; 
              break; 
            }
            addr++;
          }
          continue; 
        }

        // 4. ถ้าชื่อตรง → อ่านค่าข้อมูล (ส่วนนี้ใช้ String data ได้เพราะต้องส่งเข้า Result)
        String data = "";
        while (addr < eepromSize) {
          char c = (char)EEPROM.read(addr);
          if (c == '&') { 
            addr++; 
            break; 
          }
          data += c;
          addr++;
        }

        rawLength = addr - startAddr; // คำนวณความยาวข้อมูลดิบ

        // 5. แปลงค่าตามชนิดที่ระบุไว้ตอนต้น
        if (ch == '@') {           
          Vode.type = Result::LONG;
          Vode.lVal = data.toInt();
        } else if (ch == '#') {    
          Vode.type = Result::INT;
          Vode.iVal = data.toInt();
        } else if (ch == '$') {    
          Vode.type = Result::FLOAT;
          Vode.fVal = data.toFloat();
        } else if (ch == '%') {    
          Vode.type = Result::STRING;
          Vode.sVal = data;
        }

        return Vode; 
      }

      rawLength = 0;
      H1 = -1;
      return Result(String("-1")); 
  }

  int My_eerom::H() {
    return H1;
  }

  uint16_t My_eerom::Data_extraction() {
    if(!Use_Serial_True) {
      p.b(Starting_serial, true);
    }
    if(E.GEUP() == 0) { 
      p.text("\nNo data in EEPROM\n");
      return 0;
    }
    Len_Data = 0;
    p.text("\nStart Data_extraction\n");
    for(uint32_t i = 0; i < EEPROM.length(); i++) {
      if(EEPROM.read(i) != 0xFF) {
        for(int j = 0; j < len_Data_Buffer; j++) {
          Data_EEPROM[j] = ' ';
        }
        Data_EEPROM[len_Data_Buffer - 1] = '\0';
        TData = 0;
        uint32_t startPos = i;
        while(EEPROM.read(i) != 0xFF) {
          Data_EEPROM[TData] = EEPROM.read(i);
          i++;
          TData++;
        }
        Data_EEPROM[i+1] = '\0'; // ปิดท้าย string
        p.text("Data Buffer : ", Data_EEPROM, " : Position : ", startPos, " to ", i, "\n");
        Len_Data++;
      }
    }
    p.text("Data extraction end\n");
    return Len_Data;
  }

  int My_eerom::Search(char* name) {
    R(name);
    return H1;
  }

  int My_eerom::Search(char* name, int &Len_name) {
    R(name, Len_name);
    return H1;
  }

  int My_eerom::Search(char* name, uint32_t &Len_name) {
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

char* inputND() {
  if(!Use_Serial_True) {
    p.b(Starting_serial, true);
  }
  static char buf[Buffer_Serial_Size];
  static int Size_buf = sizeof(buf);
  static size_t idx = 0;

  while (Serial.available() > 0) {
    char c = Serial.read();
    
    if (c == '\r') {
      continue; // ข้าม Carriage Return
    }
    // ถ้าเจอการขึ้นบรรทัดใหม่ (\n) หรือบัฟเฟอร์เต็ม
    if (c == '\n' || idx >= Size_buf - 1) {
      buf[idx] = '\0'; // ปิดท้าย String ให้สมบูรณ์
      idx = 0;         // รีเซ็ตตำแหน่ง idx เป็น 0 เตรียมรับข้อความใหม่ในอนาคต
      return buf;      // คืนค่า String ที่สมบูรณ์ออกมา
    }
    buf[idx++] = c;
  }
  
  return nullptr; 
}

/* ----------------------------
   Serial Input Helpers Implementation
   ---------------------------- */
// อ่านจนเจอ '\n' หรือจนบัฟเฟอร์เต็ม (เหลือ 1 byte 0-terminator)
char* input(const char* prompt) {
  if(!Use_Serial_True) {
    p.b(Starting_serial, true);
  }
  // จองบัฟเฟอร์ภายในฟังก์ชัน (ปรับขนาดได้ตามต้องการ)
  static char buf[Buffer_Serial_Size];
  static int Size_buf = Buffer_Serial_Size;
  size_t idx = 0;
  Serial.print(prompt);
  while (true) {
    if (Serial.available() > 0) {
      char c = Serial.read();
      if (c == '\r') continue;            // ข้าม CR
      if (c == '\n' || idx >= Size_buf-1) break;
      buf[idx++] = c;
    }
  }
  buf[idx] = '\0';
  p.text(buf,'\n');
  return buf;
}

char* input() {
  if(!Use_Serial_True) {
    p.b(Starting_serial, true);
  }
  // จองบัฟเฟอร์ภายในฟังก์ชัน (ปรับขนาดได้ตามต้องการ)
  static char buf[Buffer_Serial_Size];
  static int Size_buf = Buffer_Serial_Size;
  size_t idx = 0;
  while (true) {
    if (Serial.available() > 0) {
      char c = Serial.read();
      if (c == '\r') continue;            // ข้าม CR
      if (c == '\n' || idx >= Size_buf-1) break;
      buf[idx++] = c;
    }
  }
  buf[idx] = '\0';
  return buf;
}

const int Size_Buf_Serial() {
  return Buffer_Serial_Size;
}

// ฟังก์ชันรับ base pointer, rows, cols, และข้อความ
void Segmenter(char* base, int rows, int cols, const char* Text) {
  // ล้าง buffer ทั้งหมด
  // memset(base, 0, (size_t)rows * (size_t)cols);
  // เติมช่องว่างเพื่อความชัดเจน
  for(int i = 0; i < rows; i++) {
    for(int j = 0; j < cols; j++) {
      base[i * cols + j] = '\0'; // เติมช่องว่างเพื่อความชัดเจน
    }
    base[i * cols + cols - 1] = '\0'; // ปิดท้ายแต่ละแถวด้วย null terminator
  }

  int word = 0;
  int ch = 0;

  for (int i = 0; Text[i] != '\0'; ++i) {
    char c = Text[i];
    if (c == ' ') {
      if (ch > 0) {
        base[word * cols + ch] = '\0'; // ปิดท้ายคำด้วย null terminator
        ++word;
        ch = 0;
      }
      if (word >= rows) break;
      continue;
    }
    if (ch < cols - 1) { // เว้นที่ให้ '\0'
      base[word * cols + ch] = c;
      ++ch;
    }
  }
}