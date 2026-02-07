
# Universal Operating System

UOS – Everything must be possible. Everything difficult ends at UOS.
It works like a normal library, until it reaches maximum performance dictated by the actual OS. It’s stronger and faster, but not as stable as it should be, though it's still at an acceptable level.

## Function/Working

- I/O Used like arms and legs

- miniOS Easier than RTOS and more powerful, compiled by UC

- EasyEEROM Easier is better

- EasySerial(input,print) As free as a friend
## Running Tests

It's a simple Hello performance.

```bash
#include <UOS.h>

void setup() {
  p.b(9600);                            // set Serial = 9600
  p.text("Hello UNO!\n");               // Hello UNO!
  p.text("input >>>",input(),"\n");     // input >>>
  p.text("hello : ",5," :)\n");         // hello : 5 :)
}

void loop() {}

```

## EEROM Tests

Writing EEROM will be very easy.

```bash
#include <UOS.h>

void setup() {
  p.b(115200);           // เริ่ม Serial Monitor
  //EEPROM.begin(1024);    // เริ่ม EEPROM สำหรับ UNO (1KB)

  p.text("\n--- เริ่มทดสอบ ArduinoUOS ---\n");

  // ล้าง EEPROM ครั้งแรก (ถ้าต้องการ)
  // E.clear();
  
  // เขียนข้อมูลใหม่ (จะเขียนก็ต่อเมื่อค่าเปลี่ยน)
  E.W("text", "Hello World");
  E.W("name", "UOS");
  
  // อ่านค่าที่เก็บไว้
  String val1 = E.R("text");
  String val2 = E.R("name");
  p.text("ค่า text: ", val1, "\n");
  p.text("ค่า name: ", val2, "\n");

  // อ่านค่าพร้อมความยาวจริงใน EEPROM
  int size;
  String val3 = E.R("text", size);
  p.text("ค่า text (ขนาดจริง): ", val3, " / ", size, " bytes\n");

  // ลบข้อมูล name
  E.D("name");
  p.text("ลบ 'name' แล้ว\n");

  // ตรวจสอบพื้นที่ EEPROM
  int freeBytes = E.GEUP();      // byte ที่มีข้อมูลจริง
  float freePercent = E.GEUP_F();// เปอร์เซ็นต์การใช้งาน
  p.text("พื้นที่ที่ใช้จริง: ", freeBytes, " bytes\n");
  p.text("คิดเป็น: ", freePercent, "%\n");

  // อ่านตำแหน่งล่าสุด (H1)
  p.text("ตำแหน่งล่าสุด H(): ", E.H(), "\n");

  p.text("--- จบการทดสอบ ---\n");
}

void loop() {
  // ไม่มีอะไรใน loop
}

```


## 1. I/O Helpers

| ฟังก์ชัน             | คำอธิบาย                                                       | ตัวอย่างการใช้                        |
| -------------------- | -------------------------------------------------------------- | ------------------------------------- |
| `setPinMode(pin, m)` | ตั้งโหมดพิน (m: 1=OUTPUT, 2=INPUT, 3=INPUT\_PULLUP)            | `setPinMode(5,1); // ตั้งเป็น OUTPUT` |
| `DRead(pin, mode)`   | ตั้งโหมดพิน (2=INPUT,3=INPUT\_PULLUP) แล้วอ่านค่าสัญญาณดิจิทัล | `int val = DRead(7, 2);`              |
| `btn(pin)`           | อ่านปุ่มแบบ active-low (กดคืนค่า `true`)                       | `if(btn(2)) { /* กดปุ่ม */ }`         |
| `ARead(ch)`          | อ่านค่าแอนะล็อกจากช่อง ch (0–1023)                             | `int sensorVal = ARead(0);`           |
| `pwm(i, v)`          | เขียน PWM ไปยังพินในตาราง pwmPins\[] ตามดัชนี i                | `pwm(0, 128); // ครึ่งความสว่าง`      |
| `outD(pin, v)`       | ตั้งพินเป็น OUTPUT แล้ว digitalWrite(v)                        | `outD(13, HIGH);`                     |

## 2. EEPROM Helpers

| ฟังก์ชัน          | คำอธิบาย                                       | ตัวอย่างการใช้                    |
| ----------------- | ---------------------------------------------- | --------------------------------- |
| `E.W(name, data)` | เก็บข้อมูลลง EEPROM แบบบีบอัด                  | `E.W("count", 123);`              |
| `E.R(name)`       | อ่านตัวแปรที่เก็บไว้ (ไม่เจอคืนค่า -1)         | `int val = E.R("count");`         |
| `E.R(name, size)` | อ่านตัวแปร พร้อมระบุขนาดข้อมูลจริง             | `E.R("config", 16);`              |
| `E.clear()`       | ฟอร์แมต EEPROM ให้เข้ากับไลบรารีนี้ (ครั้งแรก) | `E.clear();`                      |
| `E.D(name)`       | ลบตัวแปรออกจาก EEPROM                          | `E.D("temp");`                    |
| `E.GEUP()`        | ดูพื้นที่ว่างแบบดิบใน EEPROM                   | `int freeBytes = E.GEUP();`       |
| `E.GEUP_F()`      | ดูพื้นที่ว่างแบบเปอร์เซ็นต์ใน EEPROM           | `float freePercent = E.GEUP_F();` |
| `E.H()`           | ดูตำแหน่งบล็อกล่าสุดที่เขียน/อ่านใน EEPROM     | `int lastBlock = E.H();`          |

## 3. Watchdog
| ฟังก์ชัน | คำอธิบาย                       | ตัวอย่างการใช้ |
| -------- | ------------------------------ | -------------- |
| `wdOn()` | เปิดใช้งาน watchdog (1 วินาที) | `wdOn();`      |
| `wdR()`  | รีเซ็ต watchdog                | `wdR();`       |

## 4. Pin-Mode Abstraction
| ฟังก์ชัน             | คำอธิบาย                                            | ตัวอย่างการใช้      |
| -------------------- | --------------------------------------------------- | ------------------- |
| `setPinMode(pin, m)` | ตั้งโหมดพิน (m: 1=OUTPUT, 2=INPUT, 3=INPUT\_PULLUP) | `setPinMode(9, 2);` |

## 5. Serial I/O Helpers
| ฟังก์ชัน      | คำอธิบาย                                             | ตัวอย่างการใช้                       |
| ------------- | ---------------------------------------------------- | ------------------------------------ |
| `sIn(prompt)` | แสดง prompt แล้วอ่านบรรทัดจาก Serial (คืนค่า char\*) | `char* input = sIn("Enter name: ");` |

## 6. My_print Class (object: p)
| ฟังก์ชัน    | คำอธิบาย                             | ตัวอย่างการใช้             |
| ----------- | ------------------------------------ | -------------------------- |
| `p.b(b)`    | เริ่ม Serial ด้วย baud rate b        | `p.b(115200);`             |
| `p.text(x)` | พิมพ์ข้อความ `x`                     | `p.text("Hello, world");`  |
| `p()`       | คืนค่า `true` หาก Serial พร้อมใช้งาน | `if(p()) p.text("Ready");` |

## โค้คทดสอบ

มันมีไม่หมดหรอ ให้ไปอ่านในไฟล์ "Read.text"
```bash
#include <UOS.h>

// ตัวอย่างการทดสอบ ArduinoUOS Library

void setup() {
  // เริ่ม Serial ที่ 115200 baud
  p.b(115200);
  delay(1000);
  p.text("ArduinoUOS Test Begin\n");

  // --- 1. I/O Helpers ---
  p.text("Testing I/O Helpers...\n");
  setPinMode(13, 1);       // ตั้งพิน 13 เป็น OUTPUT
  outD(13, HIGH);          // พิน 13 = HIGH (เปิด LED)
  delay(500);
  outD(13, LOW);           // ปิด LED
  delay(500);

  int btnPressed = btn(2); // อ่านปุ่ม pin 2 (active low)
  p.text("Button on pin 2 pressed? ");
  p.text(btnPressed ? "YES\n" : "NO\n");

  int analogVal = ARead(0); // อ่านค่า analog pin 0
  p.text("Analog read ch0 = ");
  p.text(String(analogVal).c_str());
  p.text("\n");

  unsigned long ms = gml(); // ค่า millis()
  unsigned long us = gmc(); // ค่า micros()
  p.text("Millis: ");
  p.text(String(ms).c_str());
  p.text(" Micros: ");
  p.text(String(us).c_str());
  p.text("\n");

  // --- 2. EEPROM Helpers ---
  p.text("Testing EEPROM Helpers...\n");
  E.clear();             // ฟอร์แมต EEPROM ก่อน
  E.W("testVal", 1234);  // เก็บค่า 1234 ในชื่อ testVal
  int eepromVal = E.R("testVal");
  p.text("EEPROM read testVal = ");
  p.text(String(eepromVal).c_str());
  p.text("\n");

  // --- 3. Watchdog ---
  p.text("Testing Watchdog...\n");
  wdOn(); // เปิด watchdog 1 วินาที
  p.text("Watchdog started.\n");

  // --- 4. Pin-Mode Abstraction (เหมือน I/O) ---
  setPinMode(12, 1);    // ตั้ง pin 12 เป็น OUTPUT
  outD(12, HIGH);
  delay(200);
  outD(12, LOW);

  // --- 5. Serial I/O Helpers ---
  p.text("Testing Serial Input...\n");
  p.text("Type something and press Enter:\n");
  char* userInput = sIn("> "); // รอรับข้อความจาก Serial
  p.text("You typed: ");
  p.text(userInput);
  p.text("\n");

  // --- 6. List Management ---
  p.text("Testing List Management...\n");
  pClr();
  pAdd("First message");
  pAdd("Second message");
  sClr();
  sAdd("Auxiliary 1");
  sAdd("Auxiliary 2");
  p.text("Lists updated.\n");

  // --- 7. My_print class ---
  if(p()) {
    p.text("Serial is ready for printing!\n");
  }

  // --- 8. OLED Fonts (แสดงแค่ชื่อฟอนต์ตัวอย่าง) ---
  p.text("OLED fonts available:\n");
  p.text("1) u8g2_font_ncenB08_tr\n");
  p.text("2) u8g2_font_6x10_tf\n");
  p.text("... (ดูรายละเอียดในเอกสาร)\n");
}

void loop() {
  wdR(); // รีเซ็ต watchdog เพื่อป้องกันรีเซ็ตเครื่อง

  delay(1000);  // เว้น 1 วินาทีระหว่างวนลูป
}

```
