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
  2.1 "E.W(name, data)"
    - เอาไวเก็บข้อมูลลง EERROM แบบ อัด
  2.2 "E.R(name)"
    - อ่านตัวแปรที่เก็บไว ถ้าไม่เจอขึ้น -1
     2.2.1 "E.R(name)"
      - ใส่ชื่อตัวแปรที่จะอ่าน
     2.2.2 "E.R(name, J1)"
      - ใส่ชื่อตัวแปรที่จะอ่าน พร้อม J1 คือขนาดข้อมูลจริง
  2.3 "E.clear()"
    - Formrt EEROM ให้เข้ากับ ไลเบรารีนี้(เมือใช้ครั้งแรก)
  2.4 "E.D(name)"
    - ลบตัวแปรออกจาก EEROM
  2.5 "E.GEUP()"
    - ดูพื่นที่ว่างแบบดิบ
  2.6 "E.GEUP_F()"
    - ดูพื่นที่ว่างแบบ %
  2.7 "E.H()"
    - ดูตำแหน่งบล็อกล่าสุดใน EEROM ที่เขียนไป หรือ อ่าน
  2.8 "E.Data_extraction()"
    - ดึงข้อมูลออกมาจาก EEPROM แล้วแสดงขึ้นจอ สามารถดูไฟล์ที่ผิดผลาดหรืออ่านได้
      และยังคืนค่าออกมาว่ามีตัวแปรกี่ตัว
  2.9 "E.Search(name)"
    - จะหาชื่อตัวแปรใน EEPROM และจะบอกตำแหน่งบน EEPROM
     2.9.1 "E.Search(name)"
      - จะแค่หาชื่อและบอกตำแหน่ง ถ้าไม่เจอ -1
     2.9.2 "E.Search(name, Len)"
      - หาตำแหน่งเพราะความยาวข้อมูลดิบ

3. Result 
  - เป็นตั้งค่าตัวแปรที่ปลี่ยนไปๆมาๆได้ แบบเดียวกับ int -> chat โดยไม่ต้องแก้โค้คเองให้มาก
  เช่น Result UI = 1; หรือ Result UI = "1"; หรือ Result UI = 1.00; เป็นต้น

4. Pin-Mode Abstraction
  4.1 "setPinMode(pin,m)"
    - เซ็ตโหมดพิน (m: 1=OUTPUT,2=INPUT,3=INPUT\_PULLUP)

5. Serial I/O Helpers
  5.1 "sIn(prompt)"
    - แสดง prompt แล้วอ่านบรรทัดจาก Serial (return char\*)

6. My_print Class (object: p)
   * ใช้พิมพ์ข้อความผ่าน Serial ง่ายๆ
       6.1 "p.b(b)"
     * เริ่ม Serial ด้วย baud rate b
       6.2 "p.b(b, Serial_bit)"
     * เริ่ม Serial ด้วย baud rate b และ ใส่ว่าจะรอเชื่อมกับ Serial ไหม เช่นใส่ 1 จะรอ ใส่ 0 จะข้ามไปรันโค้ค
       6.3 "p.text(x)"
     * พิมพ์ ข้อความลงไปใน x
       6.4 "p.init()"
     * คืน true ถ้า Serial พร้อมใช้งาน
