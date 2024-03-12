#ifndef PWM_INC
#define PWM_INC

#define PWM_MODULECOUNT 5

void PWM_initClockSource(uint32_t source, uint32_t frequency);
void PWM_initModule(uint32_t module, uint32_t timer, uint32_t * outputPinRPR);
void PWM_setDuty(uint32_t module, uint32_t duty);
uint32_t PWM_getDuty(uint32_t module);
uint32_t PWM_isModuleOn(uint32_t module);
void PWM_setModuleOn(uint32_t module, uint32_t on);
void PWM_setFrequency(uint32_t timer, uint32_t freq);
uint32_t PWM_getFreq(uint32_t timer);

#endif