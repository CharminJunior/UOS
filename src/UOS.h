// UOS.h
#ifndef ARDUINOOS_H
#define ARDUINOOS_H

#include <Arduino.h>
#if !defined(ARDUINO_ARCH_RP2040)
  #include <EEPROM.h>
#endif
// #include <EEPROM_R4T0.h>
// #include <avr/wdt.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
// #include <U8g2lib.h>
// #include <type_traits>
#include "tinyexpr.h"

/* ----------------------------
   Definitions and Constants
   ---------------------------- */

// ON / OFF for digitalWrite
#define ON  HIGH
#define OFF LOW

// ลดขนาดลิสต์จาก 100 → 20, ความยาวแต่ละรายการจาก 10 → 12
#define MAX_LIST_ITEMS 5
#define MAX_ITEM_LEN   8

#define len_Data_Buffer 100

extern int H1;
extern bool Use_Serial_True;
extern char Data_EEPROM[len_Data_Buffer];
extern int TData;

/* ----------------------------
   Global Variables (Lists)
   ---------------------------- */
// ตอนนี้กิน SRAM เพียง 20×12 + 20×12 = 480 bytes เท่านั้น
// extern char hsuorg[MAX_LIST_ITEMS][MAX_ITEM_LEN];
// extern char sysItems[MAX_LIST_ITEMS][MAX_ITEM_LEN];
// extern int list_count;
// extern int list_count_2;

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

    /** template text() ทั้ง declaration + definition อยู่ที่นี่เลย **/
    template<typename First>
    void text(const First& t) {
      Serial.print(t);
    }

    // ถ้าต้องการ overload รับหลาย args
    template<typename First, typename... Rest>
    void text(const First& first, const Rest&... rest) {
      Serial.print(first);
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

// ----------------------------
// Result Struct Declaration
// ----------------------------
struct Result {
  enum Type { INT, LONG, FLOAT, STRING } type;

  union {
    int   iVal;
    long  lVal;
    float fVal;
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

  Result(float x) {
    type = FLOAT;
    fVal = x;
  }

  Result(const char* x) {
    type = STRING;
    sVal = x;
  }

  Result(String x) {
    type = STRING;
    sVal = x;
  }

  /* ================= Type checks ================= */

  bool isnum()   const { return type == INT || type == LONG || type == FLOAT; }
  bool isint()   const { return type == INT; }
  bool islong()  const { return type == LONG; }
  bool isfloat() const { return type == FLOAT; }
  bool isstr()   const { return type == STRING; }

  /* ================= Conversions ================= */

  float toFloat() const {
    switch (type) {
      case INT:   return (float)iVal;
      case LONG:  return (float)lVal;
      case FLOAT: return fVal;
      case STRING:return sVal.toFloat();
    }
    return 0;
  }

  String toString() const {
    switch (type) {
      case INT:   return String(iVal);
      case LONG:  return String(lVal);
      case FLOAT: return String(fVal);
      case STRING:return sVal;
    }
    return "";
  }

  /* ================= Cast operators ================= */

  operator int()    const { return (int)toFloat(); }
  operator long()   const { return (long)toFloat(); }
  operator float()  const { return toFloat(); }
  operator String() const { return toString(); }

  /* ================= Math operators ================= */

  Result operator+(const Result &rhs) const {
    if (isstr() || rhs.isstr())
      return Result(toString() + rhs.toString());
    return Result(toFloat() + rhs.toFloat());
  }

  Result operator-(const Result &rhs) const {
    return Result(toFloat() - rhs.toFloat());
  }

  Result operator*(const Result &rhs) const {
    return Result(toFloat() * rhs.toFloat());
  }

  Result operator/(const Result &rhs) const {
    if (rhs.toFloat() == 0) return Result(0);
    return Result(toFloat() / rhs.toFloat());
  }

  /* ================= Compare operators ================= */

  bool operator==(const Result &rhs) const {
    if (isnum() && rhs.isnum())
      return toFloat() == rhs.toFloat();

    if (type != rhs.type) return false;

    switch (type) {
      case STRING: return sVal == rhs.sVal;
      case INT:    return iVal == rhs.iVal;
      case LONG:   return lVal == rhs.lVal;
      case FLOAT:  return fVal == rhs.fVal;
    }
    return false;
  }

  bool operator!=(const Result &rhs) const {
    return !(*this == rhs);
  }

  /* ================= Debug helper ================= */

  const char* typeName() const {
    switch (type) {
      case INT:    return "INT";
      case LONG:   return "LONG";
      case FLOAT:  return "FLOAT";
      case STRING: return "STRING";
    }
    return "UNKNOWN";
  }
};

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
    float GEUP_F();
    void clear();
    void D(String name);
    void W(String name, Result Text);
    Result R(String nane);
    Result R(String nane, uint32_t &rawLength);
    uint32_t My_eerom::Search(String name);
    uint32_t My_eerom::Search(String name, uint32_t &Len_name);

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
 char* input(const char* prompt);
 char* input();

double fx(const char* num);

#endif // ARDUINOOS_H


