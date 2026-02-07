// V1.2.0
#include <UOS_test.h>
#include <avr/io.h>

#define MAX_TASK 9
#define MAX_RAM 256

typedef struct {
  const uint8_t *code;
  uint16_t len;
  uint16_t pc;
  uint32_t wakeTime;
  uint8_t alive;
} Task;

Task task[MAX_TASK];
uint8_t taskCount = 0;
uint8_t Order = 0;
uint32_t Clock_Hz = 0;
uint32_t Hz_Now = 0;
uint32_t next_tick;

uint16_t RAM[MAX_RAM];
// char Buffer[20];

static volatile uint8_t* const gpio_port[] = {
  &PORTD, &PORTD, &PORTD, &PORTD, &PORTD, &PORTD, &PORTD, &PORTD, // 0-7
  &PORTB, &PORTB, &PORTB, &PORTB, &PORTB, &PORTB,               // 8-13
  &PORTC, &PORTC, &PORTC, &PORTC, &PORTC, &PORTC                // 14-19
};

static volatile uint8_t* const gpio_ddr[] = {
  &DDRD, &DDRD, &DDRD, &DDRD, &DDRD, &DDRD, &DDRD, &DDRD,
  &DDRB, &DDRB, &DDRB, &DDRB, &DDRB, &DDRB,
  &DDRC, &DDRC, &DDRC, &DDRC, &DDRC, &DDRC
};

static const uint8_t gpio_bit[] = {
  0,1,2,3,4,5,6,7,      // D0–D7
  0,1,2,3,4,5,          // D8–D13
  0,1,2,3,4,5           // A0–A5
};

const uint8_t code_0[] PROGMEM = {                    // The program is here
  0x02, 0x00, 0x00, 0x00,
  0x0E, 0x01, 0x00, 0x0D,
  0x0B, 0x01, 0x00, 0x0D, 0x02, 0x00,
  0x12, 0x00,
  0x04, 0x01, 0x00, 0x64,
  0x03, 0x00, 0x08,
  0xFF
};

const uint8_t code_1[] PROGMEM = {                    // And this too
  0x02, 0x01, 0x00, 0x00,
  0x05, 0x02, 0x01, 0x01, 0x00, 0x01,
  0x01, 0x02, 0x01,
  0x01, 0x00, 0x0A, 0x00,
  0x04, 0x01, 0x03, 0xE8,
  0x03, 0x00, 0x04,
  0xFF
};

const uint16_t Len_Code0 = sizeof(code_0);
const uint16_t Len_Code1 = sizeof(code_1);

void setProgram(const uint8_t *prog, uint8_t length) {
  task[taskCount].code      = prog;
  task[taskCount].len       = length;
  task[taskCount].pc        = 0;
  task[taskCount].wakeTime  = 0;
  task[taskCount].alive     = 1;
  taskCount++;
}

void setProgram(const uint8_t *prog, uint8_t length, uint8_t alive_conton) {
  task[taskCount].code      = prog;
  task[taskCount].len       = length;
  task[taskCount].pc        = 0;
  task[taskCount].wakeTime  = 0;
  task[taskCount].alive     = alive_conton;
  taskCount++;
}

void Set_Read_code(uint8_t i) {
  Order = i;
}

uint8_t Read_code(uint16_t i) {
  return pgm_read_byte(&task[Order].code[i]);
}

uint16_t code_pc() {
  return task[Order].pc;
}

void code_pc_UP(uint16_t i) {
  task[Order].pc += i;
}

void code_pc_DM(uint16_t i) {
  task[Order].pc -= i;
}

void code_pc_ST(uint16_t i) {
  task[Order].pc = i;
}

inline void gpio_mode_output(uint8_t pin) {
  *gpio_ddr[pin] |= (1 << gpio_bit[pin]);
}

inline void gpio_mode_input(uint8_t pin) {
  *gpio_ddr[pin]  &= ~(1 << gpio_bit[pin]);
  *gpio_port[pin] &= ~(1 << gpio_bit[pin]);
}

inline void gpio_mode_input_pullup(uint8_t pin) {
  *gpio_ddr[pin]  &= ~(1 << gpio_bit[pin]);
  *gpio_port[pin] |=  (1 << gpio_bit[pin]);
}

inline void gpio_set(uint8_t pin, uint8_t value) {
  if (value)
    *gpio_port[pin] |=  (1 << gpio_bit[pin]);
  else
    *gpio_port[pin] &= ~(1 << gpio_bit[pin]);
}

void run() {
  // int pc = 0;
  // while(code_pc() < task[Order].len) {
  if(task[Order].alive == 1) {
    uint8_t OP = Read_code(code_pc());
    // p.text("pc = ", code_pc(), "   A = ", RAM[0], "\n");
    code_pc_UP(1);
    if(OP == 0x00) {                                                                                          // NOP

    } else if(OP == 0x01) {                                                                                   // PRINT
      OP = (uint8_t)Read_code(code_pc());
      code_pc_UP(1);
      if(OP == 0x00) {
        OP = Read_code(code_pc());
        // p.text("\nOP1 = ", OP, "\n");
        while(OP != 0x00) {
          if(OP != 0x00) { p.text((char)Read_code(code_pc())); }
          code_pc_UP(1);
          OP = Read_code(code_pc());
        }
      } else if(OP == 0x01) {
        uint16_t BN = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
        p.text((uint16_t)BN);
        code_pc_UP(2);
      } else if(OP == 0x02) {
        p.text((uint8_t)RAM[(uint8_t)Read_code(code_pc())]);
        code_pc_UP(1);
      }
    } else if(OP == 0x02) {                                                                                   // LET
        OP = Read_code(code_pc());
        code_pc_UP(1);
        uint16_t BN = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
        RAM[OP] = BN;
        code_pc_UP(2);
    } else if(OP == 0x03) {                                                                                   // GOTO
        code_pc_ST((uint16_t)(((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1))));
    } else if(OP == 0x04) {                                                                                   // DELAY
        OP = Read_code(code_pc());
        code_pc_UP(1);
        if(OP == 0x01) {
          uint16_t BN = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
          code_pc_UP(2);
          // delay(BN);
          task[Order].wakeTime = millis() + BN;
        } else if(OP == 0x02) {
          uint16_t BN = RAM[Read_code(code_pc())];
          code_pc_UP(1);
          // delay(BN);
          task[Order].wakeTime = millis() + BN;
        }
    } else if(OP == 0x05) {                                                                                   // ADD +
        OP = Read_code(code_pc());
        code_pc_UP(1);
        if(OP == 0x02) {
          uint8_t R1 = Read_code(code_pc());
          code_pc_UP(1);
          OP = Read_code(code_pc());
          code_pc_UP(1);
          if(OP == 0x01) {
            uint16_t BN = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
            RAM[R1] = RAM[R1] + BN;
            code_pc_UP(2);
          } else if(OP == 0x02) {
            RAM[R1] = RAM[R1] + RAM[Read_code(code_pc())];
            code_pc_UP(1);
          }
        }
    } else if(OP == 0x06) {                                                                                   // SUB -
        OP = Read_code(code_pc());
        code_pc_UP(1);
        if(OP == 0x02) {
          uint8_t R1 = Read_code(code_pc());
          code_pc_UP(1);
          OP = Read_code(code_pc());
          code_pc_UP(1);
          if(OP == 0x01) {
            uint16_t BN = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
            RAM[R1] = RAM[R1] - BN;
            code_pc_UP(2);
          } else if(OP == 0x02) {
            RAM[R1] = RAM[R1] - RAM[Read_code(code_pc())];
            code_pc_UP(1);
          }
        }
    } else if(OP == 0x07) {                                                                                 // IMUL *
        OP = Read_code(code_pc());
        code_pc_UP(1);
        if(OP == 0x02) {
          uint8_t R1 = Read_code(code_pc());
          code_pc_UP(1);
          OP = Read_code(code_pc());
          code_pc_UP(1);
          if(OP == 0x01) {
            uint16_t BN = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
            RAM[R1] = RAM[R1] * BN;
            code_pc_UP(2);
          } else if(OP == 0x02) {
            RAM[R1] = RAM[R1] * RAM[Read_code(code_pc())];
            code_pc_UP(1);
          }
        }
    } else if(OP == 0x08) {                                                                                   // IDIV /
        OP = Read_code(code_pc());
        code_pc_UP(1);
        if(OP == 0x02) {
          uint8_t R1 = Read_code(code_pc());
          code_pc_UP(1);
          OP = Read_code(code_pc());
          code_pc_UP(1);
          if(OP == 0x01) {
            uint16_t BN = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
            RAM[R1] = RAM[R1] / BN;
            code_pc_UP(2);
          } else if(OP == 0x02) {
            RAM[R1] = RAM[R1] / RAM[Read_code(code_pc())];
            code_pc_UP(1);
          }
        }
    } else if(OP == 0x09) {                                                                                   // MOV
        OP = Read_code(code_pc());
        code_pc_UP(1);
        if(OP == 0x02) {
          uint8_t R1 = Read_code(code_pc());
          code_pc_UP(1);
          OP = Read_code(code_pc());
          code_pc_UP(1);
          if(OP == 0x01) {
            RAM[R1] = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
            code_pc_UP(2);
          } else if(OP == 0x02) {
            RAM[R1] = RAM[Read_code(code_pc())];
            code_pc_UP(1);
          }
        }
    } else if(OP == 0x0A) {                                                                                    // IF
        // p.text("ifOK    ");
        OP = Read_code(code_pc());
        code_pc_UP(1);
        uint16_t A, B, C;
        uint8_t D;
        if(OP == 0x01) {
          A = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
          code_pc_UP(2);
        } else if(OP == 0x02) {
          A = RAM[Read_code(code_pc())];
          code_pc_UP(1);
        }
        D = Read_code(code_pc());
        code_pc_UP(1);
        OP = Read_code(code_pc());
        code_pc_UP(1);
        if(OP == 0x01) {
          B = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
          code_pc_UP(2);
        } else if(OP == 0x02) {
          B = RAM[Read_code(code_pc())];
          code_pc_UP(1);
        }
        C = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
        code_pc_UP(2);
        // p.text("C = ", C, "   D = ", D, "     A = ", A, "    B = ", B, "\n");
        if(D == 0x00) {
          if(A == B) { code_pc_ST(C); }
        } else if(D == 0x01) {
          if(A < B) { code_pc_ST(C); }
        } else if(D == 0x02) {
          if(A > B) { code_pc_ST(C); }
        } else if(D == 0x03) {
          if(A <= B) { code_pc_ST(C); }
        } else if(D == 0x04) {
          if(A >= B) { code_pc_ST(C); }
        } else if(D == 0x05) {
          if(A != B) { code_pc_ST(C); }
        }
    } else if(OP == 0x0B) {                                                                                    // SET
        OP = Read_code(code_pc());
        code_pc_UP(1);
        uint16_t A, B;
        if(OP == 0x01) {
          A = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
          code_pc_UP(2);
        } else if(OP == 0x02) {
          A = RAM[Read_code(code_pc())];
          code_pc_UP(1);
        }
        OP = Read_code(code_pc());
        code_pc_UP(1);
        if(OP == 0x01) {
          B = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
          code_pc_UP(2);
        } else if(OP == 0x02) {
          B = RAM[Read_code(code_pc())];
          code_pc_UP(1);
        }
        gpio_set((uint8_t)A, (uint8_t)B);
    } else if(OP == 0x0C) {                                                                                    // PWM
        OP = Read_code(code_pc());
        code_pc_UP(1);
        uint16_t A, B;
        if(OP == 0x01) {
          A = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
          code_pc_UP(2);
        } else if(OP == 0x02) {
          A = RAM[Read_code(code_pc())];
          code_pc_UP(1);
        }
        if(OP == 0x01) {
          B = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
          code_pc_UP(2);
        } else if(OP == 0x02) {
          B = RAM[Read_code(code_pc())];
          code_pc_UP(1);
        }
        analogWrite((uint8_t)A, B);
    } else if(OP == 0x0D) {                                                                                    // INP
        OP = Read_code(code_pc());
        code_pc_UP(1);
        uint16_t A;
        if(OP == 0x01) {
          A = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
          code_pc_UP(2);
        } else if(OP == 0x02) {
          A = RAM[Read_code(code_pc())];
          code_pc_UP(1);
        }
        gpio_mode_input((uint8_t)A);
    } else if(OP == 0x0E) {                                                                                    // OUT
        OP = Read_code(code_pc());
        code_pc_UP(1);
        uint16_t A;
        if(OP == 0x01) {
          A = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
          code_pc_UP(2);
        } else if(OP == 0x02) {
          A = RAM[Read_code(code_pc())];
          code_pc_UP(1);
        }
        gpio_mode_output((uint8_t)A);
    } else if(OP == 0x0F) {                                                                                    // PUP
        OP = Read_code(code_pc());
        code_pc_UP(1);
        uint16_t A;
        if(OP == 0x01) {
          A = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
          code_pc_UP(2);
        } else if(OP == 0x02) {
          A = RAM[Read_code(code_pc())];
          code_pc_UP(1);
        }
        gpio_mode_input_pullup((uint8_t)A);
    } else if(OP == 0x10) {                                                                                    // RD
        OP = Read_code(code_pc());
        code_pc_UP(1);
        uint16_t A, B;
        if(OP == 0x01) {
          A = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
          code_pc_UP(2);
        } else if(OP == 0x02) {
          A = RAM[Read_code(code_pc())];
          code_pc_UP(1);
        }
        code_pc_UP(1);
        B = RAM[Read_code(code_pc())];
        code_pc_UP(1);
        RAM[B] = analogRead((int)A);
    } else if(OP == 0x11) {                                                                                    // WR
        OP = Read_code(code_pc());
        code_pc_UP(1);
        uint16_t A, B;
        if(OP == 0x01) {
          A = ((uint16_t)Read_code(code_pc()) << 8) | ((uint16_t)Read_code(code_pc()+1));
          code_pc_UP(2);
        } else if(OP == 0x02) {
          A = RAM[Read_code(code_pc())];
          code_pc_UP(1);
        }
        code_pc_UP(1);
        B = RAM[Read_code(code_pc())];
        code_pc_UP(1);
        RAM[B] = digitalRead((int)A);
    } else if(OP == 0x12) {                                                                                    // NOT
        OP = Read_code(code_pc());
        code_pc_UP(1);
        RAM[OP] = !(bool)RAM[OP];
    } else if(OP == 0xFF) {                                                                                    // END
      // break;
      task[Order].alive = 0;
    }
  }
  // }
}

void Run_task(uint8_t i) {
  Order = i;
  run();
}

void init_task() {
  if(taskCount == MAX_TASK) { return; }
  for(int i = MAX_TASK; taskCount > i; i--) {
    task[i].alive = 0;
    task[i].wakeTime  = 0;
  }
}

void setup() {
  // put your setup code here, to run once:
  p.b(2000000);
  // p.text("Start\n");
  
  setProgram(code_0, Len_Code0);
  setProgram(code_1, Len_Code1);
  // setProgram(code_2, Len_Code2);
  // setProgram(code_3, Len_Code3);
  // setProgram(code_4, Len_Code4);
  // setProgram(code_5, Len_Code5);
  // setProgram(code_6, Len_Code6);
  // setProgram(code_7, Len_Code7);
  // setProgram(code_8, Len_Code8);
  init_task();
  Set_Read_code(0);

  // next_tick = micros();
  while(1) {
    uint32_t now = millis();
    if(task[0].alive == 1 && now >= task[0].wakeTime) { Run_task(0); }
    if(task[1].alive == 1 && now >= task[1].wakeTime) { Run_task(1); }
    if(task[2].alive == 1 && now >= task[2].wakeTime) { Run_task(2); }
    if(task[3].alive == 1 && now >= task[3].wakeTime) { Run_task(3); }
    if(task[4].alive == 1 && now >= task[4].wakeTime) { Run_task(4); }
    if(task[5].alive == 1 && now >= task[5].wakeTime) { Run_task(5); }
    if(task[6].alive == 1 && now >= task[6].wakeTime) { Run_task(6); }
    if(task[7].alive == 1 && now >= task[7].wakeTime) { Run_task(7); }
    if(task[8].alive == 1 && now >= task[8].wakeTime) { Run_task(8); }

    // delay(1);
    // Clock_Hz++;
    // next_tick += 50; // 14 us ≈ 70 KHz
    // while ((int32_t)(micros() - next_tick) < 0) {
    //   // ว่าง → รอ
    // }
    // if(now >= Hz_Now) {
    //   p.text("\n - Clock : ", Clock_Hz, "\n");
    //   Hz_Now = now + 1000;
    //   Clock_Hz = 0;
    // }
  }

  // p.text("Start!!!\n");
  // for(uint16_t i = 0; i < Len_Code; i++) {
  //   p.text("code = ", Read_code(i), "\n");
  //   // p.text("code = ", Read_code(i), "\n"); pgm_read_byte
  // }
  // p.text("END!!!\n");

  // run();

  // p.text("END\n");
}

void loop() {
  // put your main code here, to run repeatedly:

}
