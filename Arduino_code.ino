
//#define DEBUG_TIMING

#ifdef DEBUG_TIMING
  #define UNIT 20   
#else
  #define UNIT 1      
#endif

const int PIN_DATA = 18;         
const int PIN_SYNC = 19;         
const int PIN_PB_ENABLE = 25;    
const int PIN_PB_SELECT = 26;    

// surname: Sun
const uint32_t param_a = 800 * UNIT;      //a = 8 * 100us
const uint32_t param_b = 600 * UNIT;      //b = 6 * 100us
const int      param_c = 17;              //c = 13 + 4
const uint32_t param_d = 6500 * UNIT;     //d = 13 * 500us
const uint32_t T_SYNC_ON = 50 * UNIT;     //50us

volatile bool outputEnabled = false;           
volatile bool alternativeMode = false;         
bool lastPB1 = LOW;
bool lastPB2 = LOW;

enum State { SEND_SYNC, SEND_DATA_ON, SEND_DATA_OFF, SEND_IDLE };
State currentState = SEND_SYNC;
int currentPulse = 1;
unsigned long stateStartTime = 0;
uint32_t currentInterval = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_SYNC, OUTPUT);
  pinMode(PIN_PB_ENABLE, INPUT_PULLDOWN); 
  pinMode(PIN_PB_SELECT, INPUT_PULLDOWN); 
  
  Serial.println("DATA_SignalA,SYNC_SignalB"); 
}

void loop() {
  //按钮检测
  bool reading1 = digitalRead(PIN_PB_ENABLE);
  bool reading2 = digitalRead(PIN_PB_SELECT);

  if (reading1 == HIGH && lastPB1 == LOW) {
    outputEnabled = !outputEnabled;
    Serial.print("PB1_CLICKED:ENABLED=");
    Serial.println(outputEnabled ? "YES" : "NO");
    if (!outputEnabled) {
      digitalWrite(PIN_DATA, LOW);
      digitalWrite(PIN_SYNC, LOW);
      currentState = SEND_SYNC;
      currentPulse = 1;
    }
    delay(200); 
  }
  lastPB1 = reading1;

  //切换模式
  if (reading2 == HIGH && lastPB2 == LOW) {
    alternativeMode = !alternativeMode;
    Serial.print("PB2_CLICKED:MODE=");
    Serial.println(alternativeMode ? "ALT_REVERSED" : "NORMAL");
    delay(200);
  }
  lastPB2 = reading2;

  //串口绘图输出
  if (outputEnabled) {
    Serial.print(digitalRead(PIN_DATA) * 1.0); 
    Serial.print(",");
    Serial.println(digitalRead(PIN_SYNC) * 1.5); // SYNC稍微画高一点方便对比
  }

  //波形生成状态机
  if (!outputEnabled) return;

  unsigned long now = (UNIT == 1000) ? millis() : micros();

  if (now - stateStartTime >= currentInterval) {
    stateStartTime = now;
    switch (currentState) {
      case SEND_SYNC:
        digitalWrite(PIN_SYNC, HIGH);
        currentInterval = T_SYNC_ON;
        currentState = SEND_DATA_ON;
        currentPulse = 1;
        break;

      case SEND_DATA_ON:
        digitalWrite(PIN_SYNC, LOW);
        digitalWrite(PIN_DATA, HIGH);
        {
          //n从1到17变化
          int n_val = alternativeMode ? (param_c - currentPulse + 1) : currentPulse;
          currentInterval = param_a + ((n_val - 1) * 50 * UNIT); // T_ON(n) 公式 
        }
        currentState = SEND_DATA_OFF;
        break;

      case SEND_DATA_OFF:
        digitalWrite(PIN_DATA, LOW);
        if (currentPulse < param_c) {
          currentInterval = param_b;
          currentState = SEND_DATA_ON;
          currentPulse++;
        } else {
          currentInterval = param_d;
          currentState = SEND_IDLE;
        }
        break;

      case SEND_IDLE:
        currentInterval = 0; 
        currentState = SEND_SYNC;
        break;
    }
  }
}