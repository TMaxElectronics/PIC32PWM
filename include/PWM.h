#ifndef PWM_INC
#define PWM_INC

typedef union {
  struct {
    uint32_t OCM:3;
    uint32_t OCTSEL:1;
    uint32_t OCFLT:1;
    uint32_t OC32:1;
    uint32_t :7;
    uint32_t SIDL:1;
    uint32_t :1;
    uint32_t ON:1;
  };
  struct {
    uint32_t OCM0:1;
    uint32_t OCM1:1;
    uint32_t OCM2:1;
  };
  struct {
    uint32_t :13;
    uint32_t OCSIDL:1;
  };
  struct {
    uint32_t w:32;
  };
} OCxCONbits_t;

typedef union {
  struct {
    uint32_t :1;
    uint32_t TCS:1;
    uint32_t :2;
    uint32_t TCKPS:3;
    uint32_t TGATE:1;
    uint32_t :5;
    uint32_t SIDL:1;
    uint32_t :1;
    uint32_t ON:1;
  };
  struct {
    uint32_t :4;
    uint32_t TCKPS0:1;
    uint32_t TCKPS1:1;
    uint32_t TCKPS2:1;
  };
  struct {
    uint32_t :13;
    uint32_t TSIDL:1;
    uint32_t :1;
    uint32_t TON:1;
  };
  struct {
    uint32_t w:32;
  };
} TxCONbits_t;

typedef struct{
    volatile TxCONbits_t    *   CON;
    volatile uint32_t       *   CONCLR;
    volatile uint32_t       *   CONSET;
    
    volatile uint32_t       *   TPR;
    volatile uint32_t       *   COUNTREG;
    
    uint32_t timerID;
    uint32_t pwmSourceSelectValue;
} PWM_ClockSource_t;

typedef struct{
    volatile OCxCONbits_t   *   CON;
    volatile uint32_t       *   CONCLR;
    volatile uint32_t       *   CONSET;
    
    volatile uint32_t       *   RS;
    
    uint32_t moduleID;
    
    uint32_t outputPinValue;
    
    PWM_ClockSource_t * clockSource;
} PWM_Handle_t;

PWM_ClockSource_t * PWM_initClockSource(uint32_t timerID, uint32_t frequency);
PWM_Handle_t * PWM_initModule(uint32_t moduleID, PWM_ClockSource_t * timer, uint32_t * outputPinRPR);
void PWM_setDuty(PWM_Handle_t * module, int32_t duty);
uint32_t PWM_getDuty(PWM_Handle_t * module);
uint32_t PWM_isModuleOn(PWM_Handle_t * module);
void PWM_setModuleOn(PWM_Handle_t * module, uint32_t on);
void PWM_setFrequency(PWM_ClockSource_t * source, uint32_t freq);
uint32_t PWM_getFreq(PWM_ClockSource_t * source);

#if defined(OC8CON)
#define PWM_MODULECOUNT 8
#elif defined(OC7CON)
#define PWM_MODULECOUNT 7
#elif defined(OC6CON)
#define PWM_MODULECOUNT 6
#elif defined(OC5CON)
#define PWM_MODULECOUNT 5
#elif defined(OC4CON)
#define PWM_MODULECOUNT 4
#elif defined(OC3CON)
#define PWM_MODULECOUNT 3
#elif defined(OC2CON)
#define PWM_MODULECOUNT 2
#elif defined(OC1CON)
#define PWM_MODULECOUNT 1
#else
#error No PWM Channels available on this device!
#endif

#endif