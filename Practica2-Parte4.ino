#include <Arduino.h>
#include <stdint.h>



const uint8_t tabla [] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x07F,0x67};
volatile uint8_t censeg, decseg, seg, seg2, turn, timer2_visualization_counts;
volatile uint16_t timer2_overflow_max = 20;
const uint8_t TIMER2_VISUALIZATION_SSD = 10;
volatile uint16_t timer2_overflow_counts;
volatile boolean increment = true;
volatile boolean show = true;
String period="10";


// método para la visualización de los SSD
void showDisplay(int timecount) {
  uint8_t d, b;
  d = tabla[timecount] & 0x3F;
  PORTD = (PORTD & 0x03) | (d << 2);
  b = tabla [timecount] & 0xC0;
  PORTB = (PORTB & 0xFC) | (b >> 6);
}

void help() {
   Serial.println("pause on    Permite pausar el contador (internamente los números van cambiando pero no se muestran)");
   Serial.println("pause off   Permite reactivar el contador");
   Serial.println("reset       Hace un reseteo del contador");
   Serial.println("up          El contador avanza de manera incremental");
   Serial.println("down        El contador avanza de manera decremental");
   Serial.println("period [milisegundos]  Cambia el periodo de la cuenta a los milisegundos especificados");
  
  }

//método para habilitar el Timer2
void enableTimer2(int timeSet) {
  /*
  * Setting timer 2 in fast pwm
  * Setting a prescaler of 32  > T=2 usecs
  * 
  * (With 250 tics of Timer2 to get an interruption, that would be 500 usecs per interruption)
  */
  timer2_overflow_max= 2*timeSet;
  timer2_overflow_counts = 0;
  timer2_visualization_counts = 0;
  TCCR2A=0;
  TCCR2B=0;
  TIMSK2=0;
  TCCR2A |= (1<<(WGM21)) | (1<<(WGM20));
  TCCR2B |= (1<<(WGM22)) | (1<<(CS21)) | (1<<(CS20));
  OCR2A=249;
  TIMSK2 = (1<<(TOIE2)); 
  
}

// interrupt vector for timer1 overflow event
ISR(TIMER2_OVF_vect)
{
 PINC=(1<<0); 
 timer2_overflow_counts++;
 timer2_visualization_counts++;
 if (show) {
   if (timer2_overflow_counts == timer2_overflow_max && increment) {
    timer2_overflow_counts = 0;
    censeg++;
    if (censeg == 10) {
      censeg=0;
      decseg++;
      if (decseg == 10) {
        decseg = 0;
        seg++;
        if (seg >= 10 || seg < 0) {
          seg = 0;
          seg2++; 
          if (seg2 == 10) {
            seg2 = 0;
          }
        }
      }
    }
     }else if (timer2_overflow_counts == timer2_overflow_max && increment == false) {
      timer2_overflow_counts = 0;
      if (censeg != 0) {
        censeg--;
        }else {
          censeg = 9;
          if (decseg != 0) {
            decseg--;
            }else {
              decseg = 9;
              if ( seg != 0) {
                seg--;
                }else {
                  seg = 9;
                  if (seg2 != 0) {
                    seg2--;
                    }else {
                      seg2 = 9;
                      }
                  }
              }
          }          
      }
  }
    
    if (timer2_visualization_counts == TIMER2_VISUALIZATION_SSD) { // if y cada 5 ms mostrar los 4 numeros. FPS = 50
        timer2_visualization_counts = 0;
        switch (turn){
          case 0:
            PORTB = (PORTB & 0x03) | 0b00111000;
            showDisplay(censeg);
            turn++;
            break;
          case 1:
            PORTB = (PORTB & 0x03) | 0b00110100;
            showDisplay(decseg);
            turn++;
            break;
          case 2:      
         //   PORTB = (PORTB & 0x03) | 0b00101100;
            showDisplay(seg);
            PORTB= (PORTB & 0x01) | 0b00101110;
            turn++;
            break;
          case 3:
            PORTB = (PORTB & 0x03) | 0b00011100;
            showDisplay(seg2);
            turn = 0;
        }
    }
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(250);
  DDRD = 0b11111100;
  DDRB = 0b00111111;
  DDRC = 0b00000001;
  PORTD=0;
  PORTB=0;
  PORTC=0;
  censeg = 0;
  decseg = 0;
  seg = 0;
  seg2 = 0;
  turn = 0;
  
 /* Serial.println(">------------- Timer2 settings before");
  Serial.print(" TCCR2A="); Serial.println(TCCR2A,BIN);
  Serial.print(" TCCR2B="); Serial.println(TCCR2B,BIN);*/
  // Setting timer2 and enable timer2 overflows 
  enableTimer2(10);

  /*Serial.println(">------------- Timer2 settings after");
  Serial.print(" TCCR2A="); Serial.println(TCCR2A,BIN);
  Serial.print(" TCCR2B="); Serial.println(TCCR2B,BIN);*/
}


void loop() {
  // only loops
  if (Serial.available() > 0) {
    String input = Serial.readString();
    input.trim();
    if (input.equals("reset")) {
      censeg = 0;
      decseg = 0;
      seg = 0;
      seg2 = 0;
      }else if (input.equals("pause on")) {
        show = false;
     //   PORTB = (PORTB & 0x03) | 0b00111100;
        }else if (input.equals("pause off")) {
          show = true;
          }else if (input.equals("up")) {
            increment = true;
            }else if (input.equals("down")) {
              increment = false;
              }else if (input.startsWith("period ")) {
                period = input.substring(input.indexOf(" "), input.length());
                enableTimer2(period.toInt());    
                }else if (input.equals("help")) {
                  help();
                  }
    }
}
