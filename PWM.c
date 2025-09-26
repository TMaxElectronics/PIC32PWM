#include <xc.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "PWM.h"

static uint32_t TxDIVIDERS[] = {1, 2, 4, 8, 16, 32, 64, 256};

#define TCON *(source->CON)
#define TCONbits (*source->CON)
#define TCONSET (*source->CONCLR)
#define TCONCLR (*source->CONSET)

#define PR (*source->TPR)
#define PWMPR (*module->clockSource->TPR)
#define TMR (*source->COUNTREG)

#define OCCON module->CON->w
#define OCCONbits (*module->CON)
#define OCCONSET (*module->CONSET)
#define OCCONCLR (*module->CONCLR)

#define OCRS (*module->RS)

static uint32_t PWM_populateSourceRegs(PWM_ClockSource_t * source, uint32_t timerID){
	switch(timerID){
		case 2:
            source->CON = &T2CON;
            source->CONCLR = &T2CONCLR;
            source->CONSET = &T2CONSET;
            
            source->TPR = &PR2;
            source->COUNTREG = &TMR2;
            
            source->pwmSourceSelectValue = 0;
			break;
			
		case 3:
            source->CON = &T3CON;
            source->CONCLR = &T3CONCLR;
            source->CONSET = &T3CONSET;
            
            source->TPR = &PR3;
            source->COUNTREG = &TMR3;
            
            source->pwmSourceSelectValue = 1;
			break;
			
		default:
			return 0;
	}
    
    source->timerID = timerID;
    
    return 1;
}

static uint32_t PWM_populateModuleRegs(PWM_Handle_t * module, PWM_ClockSource_t * source, uint32_t moduleID){
	switch(moduleID){
		case 1:
            module->CON = &OC1CON;
            module->CONCLR = &OC1CONCLR;
            module->CONSET = &OC1CONSET;
            module->RS = &OC1RS;
            module->outputPinValue = 0b0101;
			break;
            
		case 2:
            module->CON = &OC2CON;
            module->CONCLR = &OC2CONCLR;
            module->CONSET = &OC2CONSET;
            module->RS = &OC2RS;
            module->outputPinValue = 0b0101;
			break;
            
		case 3:
            module->CON = &OC3CON;
            module->CONCLR = &OC3CONCLR;
            module->CONSET = &OC3CONSET;
            module->RS = &OC3RS;
            module->outputPinValue = 0b0101;
			break;
            
		case 4:
            module->CON = &OC4CON;
            module->CONCLR = &OC4CONCLR;
            module->CONSET = &OC4CONSET;
            module->RS = &OC4RS;
            module->outputPinValue = 0b0101;
			break;
            
		case 5:
            module->CON = &OC1CON;
            module->CONCLR = &OC1CONCLR;
            module->CONSET = &OC1CONSET;
            module->RS = &OC1RS;
            module->outputPinValue = 0b0110;
			break;
            
		default:
			return 0;
	}

    module->moduleID = moduleID;
    module->clockSource = source;
    
    return 1;
}

PWM_ClockSource_t * PWM_initClockSource(uint32_t timerID, uint32_t frequency){
    PWM_ClockSource_t * source = pvPortMalloc(sizeof(PWM_ClockSource_t));
    
    if(!PWM_populateSourceRegs(source, timerID)){
        vPortFree(source);
        return NULL;
    }
    
    //first check if the selected timer is not already active
    if(TCONbits.ON){
        //meh its on, free memory and return NULL
        vPortFree(source);
        return NULL;
    }
    
    //set the selected frequency
    PWM_setFrequency(source, frequency);
			
    //enable the timer
    TCONbits.ON = 1;
    
    return source;
}

PWM_Handle_t * PWM_initModule(uint32_t moduleID, PWM_ClockSource_t * timer, uint32_t * outputPinRPR){
    //check for valid timer pointer
    if(timer == NULL) return;
    
    PWM_Handle_t * module = pvPortMalloc(sizeof(PWM_Handle_t));
    
    if(!PWM_populateModuleRegs(module, timer, moduleID)){
        vPortFree(module);
        return NULL;
    }
    
    //first check if the selected module is not already active
    if(OCCONbits.ON){
        //meh its on, free memory and return NULL
        vPortFree(module);
        return NULL;
    }
    
    *outputPinRPR = module->outputPinValue;
    
	//set module mode to PWM without fault input and enable the module
    OCCON = _OC2CON_ON_MASK | 0b110 | (timer->pwmSourceSelectValue ? _OC1CON_OCTSEL_MASK : 0);
    
    return module;
}

void PWM_setDuty(PWM_Handle_t * module, int32_t duty){
    //check for valid module pointer
    if(module == NULL) return;
            
    if(duty > 0xffff) duty = 0xffff;
      
    //calculate comparator value
    uint32_t comp = (duty * PWMPR) >> 16;
	
	OCRS = comp;
}

uint32_t PWM_getDuty(PWM_Handle_t * module){
    //check for valid module pointer
    if(module == NULL) return 0;
    
    return (OCRS << 16) / PWMPR;
}

uint32_t PWM_isModuleOn(PWM_Handle_t * module){
    //check for valid module pointer
    if(module == NULL) return 0;
    
	return OCCON & _OC1CON_ON_MASK;
}

void PWM_setModuleOn(PWM_Handle_t * module, uint32_t on){
    //check for valid module pointer
    if(module == NULL) return;
	if(on){
		OCCONSET = _OC1CON_ON_MASK;
	}else{
		OCCONCLR = _OC1CON_ON_MASK;
	}
}

void PWM_setFrequency(PWM_ClockSource_t * source, uint32_t freq){
    //check for valid source pointer
    if(source == NULL) return;
    
    //limit max frequency TODO add sensible limits
    if(freq > 250000 || freq < 1) return;
	
    //first make sure all outputs running from this timer are off TODO do we actually need this? If so: re-implement

    //find best match divider
    uint32_t currDividerIndex = 0;
    
    for(currDividerIndex = 0; (configPERIPHERAL_CLOCK_HZ / (TxDIVIDERS[currDividerIndex] * freq) > 0xffff) && currDividerIndex < (sizeof(TxDIVIDERS) / sizeof(TxDIVIDERS[0])); currDividerIndex++);
    
    //did we find one that is large enough?
    if(currDividerIndex > (sizeof(TxDIVIDERS) / sizeof(TxDIVIDERS[0]))){
        //no => return
        return;
    }
    
    //update timer period value and divider
	TCONbits.TCKPS = currDividerIndex;
    PR = configPERIPHERAL_CLOCK_HZ / (TxDIVIDERS[currDividerIndex] * freq);
    
	//finally re-enable all the modules that were enabled before TODO
}

uint32_t PWM_getFreq(PWM_ClockSource_t * source){
    //check for valid source pointer
    if(source == NULL) return 0;
    
	return configPERIPHERAL_CLOCK_HZ / (TxDIVIDERS[T2CONbits.TCKPS] * PR);
}