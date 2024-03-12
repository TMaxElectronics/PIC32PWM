#include <xc.h>
#include <stdint.h>

#include "PWM.h"

static uint32_t TxDIVIDERS[] = {1, 2, 4, 8, 16, 32, 64, 256};

static void PWM_setDutyRegister(uint32_t module, uint32_t value);
static uint32_t PWM_getDutyRegister(uint32_t module);
static void PWM_clearControlRegister(uint32_t module, uint32_t value);
static void PWM_setControlRegister(uint32_t module, uint32_t value);
static void PWM_writeControlRegister(uint32_t module, uint32_t value);
static uint32_t PWM_getControlRegister(uint32_t module);
static uint32_t PWM_getTimerCompareValue(uint32_t module);




void PWM_initClockSource(uint32_t source, uint32_t frequency){
	//initialize a timer as a pwm clock source. Which ones are available depends on the device used

	switch(source){
		case 2:
			//first check if the selected timer is not already active
			if(T2CONbits.ON){
				//damn it is... assert and return without modifying any settings
				configASSERT(0);
				return;
			}
			
			//set the selected frequency
			PWM_setFrequency(source, frequency);
			
			//enable the timer
			T2CONbits.ON = 1;
			
			break;
			
		case 3:
			//first check if the selected timer is not already active
			if(T3CONbits.ON){
				//damn it is... assert and return without modifying any settings
				configASSERT(0);
				return;
			}
			
			//set the selected frequency
			PWM_setFrequency(source, frequency);
			
			//enable the timer
			T2CONbits.ON = 1;
			
			break;
			
		default:
			return;
	}
}

void PWM_initModule(uint32_t module, uint32_t timer, uint32_t * outputPinRPR){
	//set module mode to PWM without fault input and enable the module
	PWM_setControlRegister(module, _OC2CON_ON_MASK | 0b110 | ((timer == 3) ? _OC1CON_OCTSEL_MASK : 0));
}

void PWM_setDuty(uint32_t module, uint32_t duty){
    if(duty > 0xffff) return;
      
    
    //calculate comparator value
    uint32_t comp = (duty * PWM_getTimerCompareValue(module)) >> 16;
	
	PWM_setDutyRegister(module, comp);
}

uint32_t PWM_getDuty(uint32_t module){
    return (PWM_getDutyRegister(module) << 16) / PWM_getTimerCompareValue(module);
}

uint32_t PWM_isModuleOn(uint32_t module){
	return (PWM_getControlRegister(module) & _OC1CON_ON_MASK) != 0;
}

void PWM_setModuleOn(uint32_t module, uint32_t on){
	if(on){
		PWM_setControlRegister(module, _OC1CON_ON_MASK);
	}else{
		PWM_clearControlRegister(module, _OC1CON_ON_MASK);
	}
}

void PWM_setFrequency(uint32_t timer, uint32_t freq){
	//is the selected timer even enabled? If not just return without doing anything
	switch(timer){
		case 2:
			if(!T2CONbits.ON) return;
		case 3:
			if(!T3CONbits.ON) return;
	}
	
    //limit max frequency
    if(freq > 250000 || freq < 15) return;
	
    //first make sure all outputs running from this timer are off
	uint32_t moduleEnables[PWM_MODULECOUNT] = {[0 ... (PWM_MODULECOUNT - 1)] = 0};
	for(uint32_t i = 0; i < PWM_MODULECOUNT; i++){
		if(timer == (((PWM_getControlRegister(i + 1) & _OC1CON_OCTSEL_MASK) >> 3) + 2)){
			moduleEnables[i] = PWM_isModuleOn(i + 1);
			PWM_setModuleOn(i + 1, 0);
		}
	}

    //find best match divider
    uint32_t currDividerIndex = 0;
    
    for(currDividerIndex = 0; (configPERIPHERAL_CLOCK_HZ / (TxDIVIDERS[currDividerIndex] * freq) > 0xffff) && currDividerIndex < (sizeof(TxDIVIDERS) / sizeof(TxDIVIDERS[0])); currDividerIndex++);
    
    //did we find one that is large enough?
    if(currDividerIndex > (sizeof(TxDIVIDERS) / sizeof(TxDIVIDERS[0]))){
        //no => return
        return;
    }
    
    //update timer period value and divider
	switch(timer){
		case 2:
			T2CONbits.TCKPS = currDividerIndex;
			PR2 = configPERIPHERAL_CLOCK_HZ / (TxDIVIDERS[currDividerIndex] * freq);
			break;
		case 3:
			T3CONbits.TCKPS = currDividerIndex;
			PR3 = configPERIPHERAL_CLOCK_HZ / (TxDIVIDERS[currDividerIndex] * freq);
			break;
	}
    
	//finally re-enable all the modules that were enabled before
	uint32_t moduleEnables[PWM_MODULECOUNT] = {[0 ... (PWM_MODULECOUNT - 1)] = 0};
	for(uint32_t i = 0; i < PWM_MODULECOUNT; i++){
		if(moduleEnables[i]) PWM_setModuleOn(i + 1, 1);
	}
}

uint32_t PWM_getFreq(uint32_t timer){
	switch(timer){
		case 2:
			return configPERIPHERAL_CLOCK_HZ / (TxDIVIDERS[T2CONbits.TCKPS] * PR2);
		case 3:
			return configPERIPHERAL_CLOCK_HZ / (TxDIVIDERS[T3CONbits.TCKPS] * PR3);
	}
}



//Hardware access functions
static uint32_t PWM_getTimerCompareValue(uint32_t module){
	return (PWM_getControlRegister(module) & _OC1CON_OCTSEL_MASK) ? PR3 : PR2;
}

static uint32_t PWM_getControlRegister(uint32_t module){
	switch(module){
		case 1:
			return OC1CON;
		case 2:
			return OC2CON;
		case 3:
			return OC3CON;
		case 4:
			return OC4CON;
		case 5:
			return OC5CON;
			
		default:
			return 0;
	}
}

static void PWM_writeControlRegister(uint32_t module, uint32_t value){
	switch(module){
		case 1:
			OC1CON = value;
			return;
		case 2:
			OC2CON = value;
			return;
		case 3:
			OC3CON = value;
			return;
		case 4:
			OC4CON = value;
			return;
		case 5:
			OC5CON = value;
			return;
			
		default:
			return;
	}
}

static void PWM_setControlRegister(uint32_t module, uint32_t value){
	switch(module){
		case 1:
			OC1CONSET = value;
			return;
		case 2:
			OC2CONSET = value;
			return;
		case 3:
			OC3CONSET = value;
			return;
		case 4:
			OC4CONSET = value;
			return;
		case 5:
			OC5CONSET = value;
			return;
			
		default:
			return;
	}
}

static void PWM_clearControlRegister(uint32_t module, uint32_t value){
	switch(module){
		case 1:
			OC1CONCLR = value;
			return;
		case 2:
			OC2CONCLR = value;
			return;
		case 3:
			OC3CONCLR = value;
			return;
		case 4:
			OC4CONCLR = value;
			return;
		case 5:
			OC5CONCLR = value;
			return;
			
		default:
			return;
	}
}

static uint32_t PWM_getDutyRegister(uint32_t module){
	switch(module){
		case 1:
			return OC1RS;
		case 2:
			return OC2RS;
		case 3:
			return OC3RS;
		case 4:
			return OC4RS;
		case 5:
			return OC5RS;
			
		default:
			return 0;
	}
}

static void PWM_setDutyRegister(uint32_t module, uint32_t value){
	switch(module){
		case 1:
			OC1RS = value;
			return;
		case 2:
			OC2RS = value;
			return;
		case 3:
			OC3RS = value;
			return;
		case 4:
			OC4RS = value;
			return;
		case 5:
			OC5RS = value;
			return;
			
		default:
			return;
	}
}