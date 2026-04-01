UOS Library Notes

เอกสารฉบับนี้เป็นสรุปฟังก์ชันและการใช้งานเบื้องต้น สำหรับผู้เริ่มต้นหรือผู้ที่ไม่คุ้นเคยกับไลบรารี UOS สามารถคัดลอกไฟล์นี้ไปวางเป็น "README.txt" หรือเปิดอ่านในโฟลเดอร์ไลบรารีได้เลย

1. I/O Helpers
  1.0 "setPinMode(pin, m)"
    - เซ็ตโหมดพิน (m: 1=OUTPUT,2=INPUT,3=INPUT\_PULLUP)
  1.1 "DRead(pin, mode)"
    - เซ็ตโหมดพิน (mode: 2=INPUT,3=INPUT\_PULLUP) แล้วอ่านค่าสัญญาณดิจิทัล
  1.2 "btn(pin)"
    - อ่านปุ่มแบบ active-low (กดแล้วคืนค่า true)
  1.3 "ARead(ch)"
    - อ่านค่าแอนะล็อกจากช่อง ch (0–1023)
  1.4 "pwm(i, v)"
    - เขียน PWM ไปยังพินในตาราง pwmPins[] ตามดัชนี i
  1.5 "outD(pin, v)"
    - เซ็ตพินเป็น OUTPUT แล้ว digitalWrite(v)

2. EEPROM Helpers
  2.1 "E.W(char *name, Result data)"
    - เอาไวเก็บข้อมูลลง EERROM แบบ อัด
  2.2 "E.R(char *name)"
    - อ่านตัวแปรที่เก็บไว ถ้าไม่เจอขึ้น -1
     2.2.1 "E.R(char *name)"
      - ใส่ชื่อตัวแปรที่จะอ่าน
     2.2.2 "E.R(char *name, int || uint32_t J1)"
      - ใส่ชื่อตัวแปรที่จะอ่าน พร้อม J1 คือขนาดข้อมูลจริง
  2.3 "E.clear()"
    - Formrt EEPROM ให้เข้ากับ ไลเบรารีนี้(เมือใช้ครั้งแรก)
  2.4 "E.D(char *name)"
    - ลบตัวแปรออกจาก EEPROM
  2.5 "E.GEUP()"
    - ดูพื่นที่ว่างแบบดิบ
  2.6 "E.GEUP_T()"
    - ดูพื่นที่ว่างแบบ int 0-100
  2.7 "E.GEUP_F()"
    - ดูพื่นที่ว่างแบบ % 0-100
  2.8 "E.H()"
    - ดูตำแหน่งบล็อกล่าสุดใน EEPROM ที่เขียนไป หรือ อ่าน
  2.9 "E.Data_extraction()"
    - ดึงข้อมูลออกมาจาก EEPROM แล้วแสดงขึ้นจอ สามารถดูไฟล์ที่ผิดผลาดหรืออ่านได้
      และยังคืนค่าออกมาว่ามีตัวแปรกี่ตัว
  2.10 "E.Search(char *name)"
    - จะหาชื่อตัวแปรใน EEPROM และจะบอกตำแหน่งบน EEPROM
     2.10.1 "E.Search(char *name)"
      - จะแค่หาชื่อและบอกตำแหน่ง ถ้าไม่เจอ -1
     2.10.2 "E.Search(char *name, int || uint32_t Len)"
      - หาตำแหน่งพร้อมความยาวข้อมูลดิบ

3. Result 
  - เป็นตั้งค่าตัวแปรที่ปลี่ยนไปๆมาๆได้ แบบเดียวกับ int -> chat โดยไม่ต้องแก้โค้คเองให้มาก
  เช่น Result UI = 1; หรือ Result UI = "1"; หรือ Result UI = 1.00; เป็นต้น
  รองรับ INT LONG FLOAT STRING CHAR และสามารถใช้เป็นตัวแปรพิเศษได้ง่าย โดยใช้ฟั่งชัน
  3.1 ดู Type คืนค่าเป็นเลข
    - ".isnum()"
    - ".isint()"
    - ".islong()"
    - ".isfloat()"
    - ".isstr()"
    - ".ischar()"

  3.2 แปลงชนิดตัวแปรให้คอมไพล์ และ แปลงชนิดข้อความเป็น int <> str ได้
    - ".toInt()"
    - ".toFloat()"
    - ".toString()"

  3.3 ดูชนิดข้อมูลแบบคืนค่าเป็น const char*
    - ".typeName_c()"
  
  3.4 ดูชนิดข้อมูลแบบคืนค่าเป็น uint8_t
    - ".typeName_t()"
  
  3.5 ฟังก์ชันเช็คว่า String นี้เป็นตัวเลขที่เอาไปคำนวณได้จริงหรือไม่
    3.5.1 CBC = Check Before Convert เอาไวดูว่าข้อความนี้เป็นเลขที่คำนวนได้ไหม การใช้งาน text.CBC(); หรือ text.toString(); text.isNumeric(text); CBC() จะง่ายกว่าในการใช้งาน
      - ".CBC()"
      - ".isNumeric(const String& s)" *** เป็นฟั่งชันของ sys ไม่จำเป็นที่ต้องใช้ ***

4. Pin-Mode Abstraction
  4.1 "setPinMode(pin,m)"
    - เซ็ตโหมดพิน (m: 1=OUTPUT,2=INPUT,3=INPUT\_PULLUP)

5. My_print Class (object: p)
   * ใช้พิมพ์ข้อความผ่าน Serial ง่ายๆ
       5.1 "p.b(b)"
     * เริ่ม Serial ด้วย baud rate b
       5.2 "p.b(b, Serial_bit)"
     * เริ่ม Serial ด้วย baud rate b และ ใส่ว่าจะรอเชื่อมกับ Serial ไหม เช่นใส่ 1 จะรอ ใส่ 0 จะข้ามไปรันโค้ค
       5.3 "p.text(x)"
     * พิมพ์ ข้อความลงไปใน x
       5.4 "p.init()"
     * คืน true ถ้า Serial พร้อมใช้งาน

6. input from Serial คืนค่าเป็นชนิด char*
   * ใช้เพื่อรับข้อความจาก Serial
        6.1 "input()"
     * รอรับแค่ข้อความจาก Serial 
        6.2 "input(const char *x)"
     * รอรับข้อความพร้อมแสดง x ลงไปใน ข้อความ(x เป็น char*)
        6.3 "inputND()"
     * รับข้อความแบบไม่รอ
