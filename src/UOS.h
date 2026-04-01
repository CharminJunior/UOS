// UOS.h
#ifndef ARDUINOOS_H
#define ARDUINOOS_H

#include <Arduino.h>
#if !defined(ARDUINO_ARCH_RP2040)
  #include <EEPROM.h>
#endif
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* ----------------------------
   Definitions and Constants
   ---------------------------- */

// ON / OFF for digitalWrite
#define ON  HIGH
#define OFF LOW

// ลดขนาดลิสต์จาก 100 → 20, ความยาวแต่ละรายการจาก 10 → 12
// #define MAX_LIST_ITEMS 5
// #define MAX_ITEM_LEN   8

#define len_Data_Buffer 64
#define Buffer_EEPROM_Size 32 
#define Buffer_Serial_Size 64

#define Starting_serial 9600

#define ROWS(a) (sizeof(a) / sizeof((a)[0]))
#define COLS(a) (sizeof((a)[0]) / sizeof((a)[0][0]))
#define SEG(base, Text) (Segmenter((char*)base, ROWS(base), COLS(base), Text))

extern int H1;
extern bool Use_Serial_True;
extern bool RandomSeeded_True;
extern char Data_EEPROM[len_Data_Buffer];
extern int TData;

// ----------------------------
// Result Struct Declaration
// ----------------------------
struct Result {
  // [เพิ่มส่วนนี้] เพิ่ม CHAR เข้าไปใน enum
  enum Type { INT, LONG, FLOAT, STRING, CHAR, DOUBLE} type;

  union {
    int   iVal;
    long  lVal;
    float fVal;
    char  cVal;   // [เพิ่มส่วนนี้] เพิ่มตัวแปรสำหรับเก็บตัวอักษร
    double dVal;  // [เพิ่มส่วนนี้] เพิ่มตัวแปรสำหรับเก็บ double
  };

  String sVal;

  /* ================= Constructors ================= */

  Result() {
    type = STRING;
    sVal = "";
  }

  Result(int x) {
    type = INT;
    iVal = x;
  }

  Result(long x) {
    type = LONG;
    lVal = x;
  }

  Result(unsigned long x) {
    type = LONG;
    lVal = x;
  }

  Result(float x) {
    type = FLOAT;
    fVal = x;
  }

  // [เพิ่มส่วนนี้] Constructor สำหรับ double  
  Result(double x) {
    type = DOUBLE;
    dVal = x;
  }

  // รองรับ C-String (const char*) โดยเก็บเป็น String ปลอดภัยต่อ Memory
  Result(const char* x) {
    type = STRING;
    sVal = x;
  }

  // [เพิ่มส่วนนี้] รองรับ char* แบบปกติ (เผื่อมีการส่งตัวแปรที่ไม่ได้เป็น const มา)
  Result(char* x) {
    type = STRING;
    sVal = x;
  }

  Result(String x) {
    type = STRING;
    sVal = x;
  }

  // [เพิ่มส่วนนี้] Constructor สำหรับ char ตัวเดียว
  Result(char x) {
    type = CHAR;
    cVal = x;
  }

  /* ================= Type checks ================= */

  bool isnum()   const { return type == INT || type == LONG || type == FLOAT || type == DOUBLE; }
  bool isint()   const { return type == INT; }
  bool islong()  const { return type == LONG; }
  bool isfloat() const { return type == FLOAT; }
  bool isstr()   const { return type == STRING; }
  bool ischar()  const { return type == CHAR; } // [เพิ่มส่วนนี้]
  bool isdouble() const { return type == DOUBLE; } // [เพิ่มส่วนนี้]

  /* ================= Conversions ================= */
  
  int toInt() const {
    switch (type) {
      case INT:    return iVal;
      case LONG:   return (int)lVal;
      case FLOAT:  return (int)fVal;
      case CHAR:   return (int)cVal; // [เพิ่มส่วนนี้] แปลง char เป็นรหัส ASCII
      case STRING: return sVal.toInt();
      case DOUBLE: return (int)dVal; // [เพิ่มส่วนนี้] แปลง double เป็น int
    }
    return 0;
  }

  float toFloat() const {
    switch (type) {
      case INT:   return (float)iVal;
      case LONG:  return (float)lVal;
      case FLOAT: return fVal;
      case CHAR:  return (float)cVal; // [เพิ่มส่วนนี้] แปลง char เป็นรหัส ASCII
      case STRING:return sVal.toFloat();
      case DOUBLE:return (float)dVal; // [เพิ่มส่วนนี้] แปลง double เป็น float
    }
    return 0;
  }

  double toDouble() const {
    switch (type) {
      case INT:   return iVal;
      case LONG:  return (double)lVal;
      case FLOAT: return (double)fVal;
      case CHAR:  return (double)cVal; // [เพิ่มส่วนนี้] แปลง char เป็น String
      case STRING:return sVal.toDouble();
      case DOUBLE:return dVal; // [เพิ่มส่วนนี้] แปลง double เป็น String
    }
    return 0;
  }

  String toString() const {
    switch (type) {
      case INT:   return String(iVal);
      case LONG:  return String(lVal);
      case FLOAT: return String(fVal);
      case CHAR:  return String(cVal); // [เพิ่มส่วนนี้] แปลง char เป็น String
      case STRING:return sVal;
      case DOUBLE:return String(dVal); // [เพิ่มส่วนนี้] แปลง double เป็น String
    }
    return "";
  }

  // CBC = Check Before Convert: ฟังก์ชันเช็คว่า String นี้เป็นตัวเลขหรือไม่ (สำหรับกรณีที่ type เป็น STRING แต่ต้องการตรวจสอบว่าเป็นเลขหรือเปล่า)
  // ฟังก์ชันเช็คว่า String นี้เป็นตัวเลขหรือไม่ (สำหรับกรณีที่ type เป็น STRING แต่ต้องการตรวจสอบว่าเป็นเลขหรือเปล่า)
  bool CBC() const {
    toString(); // เรียก toString() เพื่อให้แน่ใจว่า sVal ถูกแปลงเป็น String แล้วก่อนตรวจสอบ
    const String& s = sVal; // ใช้ sVal ที่ถูกแปลงแล้วแทน s เพื่อความชัดเจน
    if (s.length() == 0) return false;
    bool hasDecimal = false;
    uint16_t start = 0;
    
    if (s[0] == '-') { // รองรับเลขติดลบ
      if (s.length() == 1) return false; // มีแค่เครื่องหมายลบเฉยๆ ไม่นับ
      start = 1;
    }
    
    for (uint16_t i = start; i < s.length(); i++) {
      if (s[i] == '.') {
        if (hasDecimal) return false; // มีจุดทศนิยมซ้ำ ไม่ใช่เลข
        hasDecimal = true;
      } else if (!isDigit(s[i])) { // ถ้าเจอตัวอักษรที่ไม่ใช่เลข (เช่น 'A')
        return false;
      }
    }
    return true;
  }

  // ฟังก์ชันเช็คว่า String นี้เป็นตัวเลขที่เอาไปคำนวณได้จริงหรือไม่
  bool isNumeric(const String& s) const {
    if (s.length() == 0) return false;
    bool hasDecimal = false;
    uint16_t start = 0;
    
    if (s[0] == '-') { // รองรับเลขติดลบ
      if (s.length() == 1) return false; // มีแค่เครื่องหมายลบเฉยๆ ไม่นับ
      start = 1;
    }
    
    for (uint16_t i = start; i < s.length(); i++) {
      if (s[i] == '.') {
        if (hasDecimal) return false; // มีจุดทศนิยมซ้ำ ไม่ใช่เลข
        hasDecimal = true;
      } else if (!isDigit(s[i])) { // ถ้าเจอตัวอักษรที่ไม่ใช่เลข (เช่น 'A')
        return false;
      }
    }
    return true;
  }

  /* ================= Cast operators ================= */

  // operator int()    const { return (int)toFloat(); }
  // operator long()   const { return (long)toFloat(); }
  // operator float()  const { return toFloat(); }
  // operator String() const { return toString(); }

  /*explicit*/ operator int()    const { return (int)toFloat(); }
  /*explicit*/ operator long()   const { return (long)toFloat(); }
  /*explicit*/ operator float()  const { return toFloat(); }
  /*explicit*/ operator double() const { return toDouble(); } // [เพิ่มส่วนนี้] Cast เป็น double
  operator String()          const { return toString(); }
  
  // [เพิ่มส่วนนี้] Cast กลับเป็น char ตัวเดียว
  /*explicit*/ operator char()   const { 
    if (type == CHAR) return cVal;
    if (type == STRING && sVal.length() > 0) return sVal[0];
    if (type == INT || type == LONG) return (char)toInt();
    return (char)toFloat();
  }

  // [สำคัญ!] เติม explicit ตรงนี้เพื่อลดความกำกวม
  /*explicit*/ operator const char*() const { return sVal.c_str(); }

  // // [เพิ่มส่วนนี้] Cast กลับเป็น const char* (C-String)
  // operator const char*() const { 
  //   return sVal.c_str(); // คืนค่า pointer จาก String โดยตรง
  // }

  /* ================= Math operators ================= */

  Result operator+(const Result &rhs) const {

    // 🧠 กรณี: ทั้งสองฝั่งเป็น "ตัวเลขแท้"
    if (isnum() && rhs.isnum()) {
      return Result(toDouble() + rhs.toDouble());
    }

    // 🧠 กรณี: ฝั่งนี้เป็น string แต่ "เป็นเลข"
    if (isstr() && isNumeric(sVal) && rhs.isnum()) {
      return Result(sVal.toDouble() + rhs.toDouble());
    }

    // 🧠 กรณี: rhs เป็น string และ "เป็นเลข"
    if (rhs.isstr() && rhs.isNumeric(rhs.sVal) && isnum()) {
      return Result(toDouble() + rhs.sVal.toDouble());
    }

    // 🧠 กรณี: ทั้งคู่เป็น string และ "เป็นเลข"
    if (isstr() && rhs.isstr() && isNumeric(sVal) && rhs.isNumeric(rhs.sVal)) {
      return Result(sVal.toDouble() + rhs.sVal.toDouble());
    }

    // 💥 กรณีอื่น = ต่อ string
    return Result(toString() + rhs.toString());
  }

  Result operator-(const Result &rhs) const {

    // 🧠 ถ้าเป็นตัวเลขทั้งคู่
    if (isnum() && rhs.isnum()) {
      return Result(toDouble() - rhs.toDouble());
    }

    // 🧠 string ที่เป็นเลข
    if (isstr() && isNumeric(sVal) && rhs.isnum()) {
      return Result(sVal.toDouble() - rhs.toDouble());
    }

    if (rhs.isstr() && rhs.isNumeric(rhs.sVal) && isnum()) {
      return Result(toDouble() - rhs.sVal.toDouble());
    }

    if (isstr() && rhs.isstr() && isNumeric(sVal) && rhs.isNumeric(rhs.sVal)) {
      return Result(sVal.toDouble() - rhs.sVal.toDouble());
    }

    // ❌ ไม่ใช่ตัวเลข → ให้ 0 (หรือจะ error ก็ได้)
    return Result(0);
  }

  Result operator*(const Result &rhs) const {

    if (isnum() && rhs.isnum()) {
      return Result(toDouble() * rhs.toDouble());
    }

    if (isstr() && isNumeric(sVal) && rhs.isnum()) {
      return Result(sVal.toDouble() * rhs.toDouble());
    }

    if (rhs.isstr() && rhs.isNumeric(rhs.sVal) && isnum()) {
      return Result(toDouble() * rhs.sVal.toDouble());
    }

    // 🔥 string repeat
    if (isstr() && rhs.isnum()) {
      int times = rhs.toInt();
      String out = "";
      for (int i = 0; i < times; i++) out += sVal;
      return Result(out);
    }

    if (isstr() && rhs.isstr() && isNumeric(sVal) && rhs.isNumeric(rhs.sVal)) {
      return Result(sVal.toDouble() * rhs.sVal.toDouble());
    }

    return Result(0);
  }

  Result operator/(const Result &rhs) const {

    double divisor = rhs.toDouble();

    // 🧠 กันหาร 0
    if (divisor == 0) {
      return Result(0); // หรือจะทำ INF ก็ได้
    }

    if (isnum() && rhs.isnum()) {
      return Result(toDouble() / divisor);
    }

    if (isstr() && isNumeric(sVal) && rhs.isnum()) {
      return Result(sVal.toDouble() / divisor);
    }

    if (rhs.isstr() && rhs.isNumeric(rhs.sVal) && isnum()) {
      return Result(toDouble() / rhs.sVal.toDouble());
    }

    if (isstr() && rhs.isstr() && isNumeric(sVal) && rhs.isNumeric(rhs.sVal)) {
      return Result(sVal.toDouble() / rhs.sVal.toDouble());
    }

    return Result(0);
  }

  /* ================= Compare operators ================= */

  bool operator==(const Result &rhs) const {
    // 1. ถ้าเป็นตัวเลขทั้งคู่ เทียบกันได้เลย (10 == 10.0 -> true)
    if (isnum() && rhs.isnum()) {
      return toFloat() == rhs.toFloat();
    }

    // 2. ถ้าฝั่งหนึ่งเป็น String อีกฝั่งเป็นตัวเลข
    if (isstr() && rhs.isnum()) {
      if (!isNumeric(sVal)) return false; // ถ้า "Apple" เทียบกับเลข -> false ทันที
      return sVal.toFloat() == rhs.toFloat();
    }
    if (isnum() && rhs.isstr()) {
      if (!isNumeric(rhs.sVal)) return false; // ถ้าเลข เทียบกับ "Apple" -> false ทันที
      return toFloat() == rhs.sVal.toFloat();
    }

    // 3. ถ้า Type เหมือนกันเป๊ะ (String == String, Char == Char)
    if (type == rhs.type) {
      switch (type) {
        case STRING: return sVal == rhs.sVal;
        case CHAR:   return cVal == rhs.cVal;
        case INT:    return iVal == rhs.iVal;
        case LONG:   return lVal == rhs.lVal;
        case FLOAT:  return fVal == rhs.fVal;
      }
    }

    return false; // กรณีอื่นๆ เช่น Char เทียบกับ String
  }

  // 2. [เพิ่มส่วนนี้] สำหรับเทียบกับ "ข้อความ" โดยตรง (แก้ Error Ambiguous)
  bool operator==(const char* rhs) const {
    if (type == STRING) return sVal == rhs;
    if (type == CHAR)   return sVal[0] == rhs[0] && rhs[1] == '\0';
    return false;
  }

  // อย่าลืม != สำหรับ const char* ด้วย
  bool operator!=(const char* rhs) const {
    return !(*this == rhs);
  }

  // สำหรับตัวแปรอื่นๆ ให้ใช้ตัวเดิมที่มีอยู่
  bool operator!=(const Result &rhs) const {
    return !(*this == rhs);
  }

  /* ================= Debug helper ================= */

  const char* typeName_c() const {
    switch (type) {
      case INT:    return "INT";
      case LONG:   return "LONG";
      case FLOAT:  return "FLOAT";
      case STRING: return "STRING";
      case CHAR:   return "CHAR"; // [เพิ่มส่วนนี้]
      case DOUBLE: return "DOUBLE"; // [เพิ่มส่วนนี้]
    }
    return "UNKNOWN";
  }

  uint8_t typeName_t() const {
    return (uint8_t)type + 1; // ใช้ค่าของ enum โดยตรง (INT=0 → 1, LONG=1 → 2, ...)
  }

};

/* ----------------------------
   My_print Class Declaration
   ---------------------------- */
class My_print {
  public:
    My_print();

    /** คือค่า Use_Serial_True **/
    bool init();

    /** ฟังก์ชัน non‑template **/
    void b(long baud);
    void b(long baud, bool Serial_bit);

    // ฟังก์ชัน Super Print ที่รองรับ Result และ การใส่ชนิดค่าที่แปลกๆ ในคำสั่งเดียว
    // ถ้า Result เป็นคลาสที่มีเมธอด toString() ที่คืนค่า String
    // void Stext(const Result& t) {
    //   Serial.print((String)t.toString());
    // }

    void Stext(const Result& r) {
      switch (r.type) {
        case Result::INT:
          Serial.print(r.iVal);
          break;

        case Result::LONG:
          Serial.print(r.lVal);
          break;

        case Result::FLOAT:
          Serial.print(r.fVal);
          break;

        case Result::DOUBLE:
          Serial.print(r.dVal);
          break;

        case Result::CHAR:
          Serial.print(r.cVal);
          break;

        case Result::STRING:
          Serial.print(r.sVal);
          break;
      }
    }

    template<typename T>
    void Stext(const T& t) {
      Serial.print(t);
    }

    /** template text() ทั้ง declaration + definition อยู่ที่นี่เลย **/
    template<typename First>
    void text(const First& t) {
      // Serial.print(t);
      Stext(t);
    }

    // ถ้าต้องการ overload รับหลาย args
    template<typename First, typename... Rest>
    void text(const First& first, const Rest&... rest) {
      // Serial.print(first);
      text(first); // เรียก text() แบบรับ arg เดียวเพื่อพิมพ์ค่าแรก
      text(rest...);
    }

    bool operator()() const {
      return (bool)Serial;
    }
};

extern My_print p;  // ประกาศตัวแปร global

/* ----------------------------
   I/O Helpers Declaration
   ---------------------------- */
bool DRead(const uint8_t pin, const uint8_t SetPin);		// อ่าน Digital
bool btn(const uint8_t pin);				// อ่าน Digital ที่เป็นปุ่ม
int ARead(const uint8_t ch);				// อ่าน analog
void pwm(const uint8_t idx, const uint8_t value);		// ส่งค่าเป็น PWM
void outD(const uint8_t pin, const bool value);			// ส่งค่าเป็น Digital
// unsigned long gml();
// unsigned long gmc();

#if !defined(ARDUINO_ARCH_RP2040)
// ส่วนเสริม **จำเป็น**
bool isValidPosition(int pos, int length);
bool isNumber(String str);
int findValidPosition(String Compilation);
#endif

#if !defined(ARDUINO_ARCH_RP2040)
  // ----------------------------
  // New EEPROM Class 
  // ----------------------------
  // #(int), @(long), $(float), %(string)
  // รูปแบบการเก็บข้อมูล: @name&value&
  class My_eerom {
    // int H1;
  public:

    My_eerom();  // ประกาศ constructor ไว้ด้วย
    #if !defined(ESP8266) && !defined(ESP32)
      void begin();       // AVR / UNO ใช้ได้
    #else
      void begin(size_t size); // ESP ต้องส่ง size ด้วย
    #endif
    int GEUP();
    uint8_t GEUP_T();
    float GEUP_F();
    void clear();
    void D(char* name);
    void W(char* name, Result Text);
    Result R(char* nane);
    Result R(char* nane, int &rawLength);
    Result R(char* nane, uint32_t &rawLength);
    int Search(char* name);
    int Search(char* name, int &Len_name);
    int Search(char* name, uint32_t &Len_name);

    int H();
    uint16_t Data_extraction();
  };
#endif

extern My_print p;
#if !defined(ARDUINO_ARCH_RP2040)
  extern My_eerom E;
#endif

/* ----------------------------
   Pin-Mode Abstraction
   ---------------------------- */
void setPinMode(uint8_t pin, uint8_t mode);

/* ----------------------------
   Serial Input Helpers
   ---------------------------- */
// เปลี่ยนเป็นรับ input เป็น char buffer แทน String เพื่อประหยัด SRAM
 // เป็นการรับ input แบบไม่มี dalay (อ่านทันทีที่มีข้อมูลเข้ามา)
 char* inputND();
 // เป็นการรับ input แบบมี delay (รอจนกว่าจะมีการกด Enter)
 char* input(const char* prompt);
 char* input();

 const int Size_Buf_Serial();
 void Segmenter(char* base, int rows, int cols, const char* Text);

#endif // ARDUINOOS_H


