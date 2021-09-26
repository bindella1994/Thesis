#include "Acceleration.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

float getMeanAcceleration()
{
    float result=0;
    short i;
    if(accelerationBuffer.full == 0x0 ){
        
        for (i=0;i<accelerationBuffer.index;++i){
            result+=accelerationBuffer.buffer[i];
        }
        
        result = result/accelerationBuffer.index;
    }
    else{
        
        for (i=0;i<ACCELERATION_BUFFER_SIZE;++i){
            result+=accelerationBuffer.buffer[i];
        }
        
        result = result/ACCELERATION_BUFFER_SIZE;
    }
    return result;
}



void addAcceleration(const float* a)
{
    accelerationBuffer.buffer[accelerationBuffer.index % ACCELERATION_BUFFER_SIZE] = *a;
    
    if(accelerationBuffer.index<ACCELERATION_BUFFER_SIZE){
        ++accelerationBuffer.index;
    }else{
        accelerationBuffer.index=0;
    }
    
    if(!accelerationBuffer.full && ( accelerationBuffer.index == ACCELERATION_BUFFER_SIZE ) ){
        accelerationBuffer.full = 0xff;
    }
    accelerationBuffer.mean3DAcceleration = getMeanAcceleration();
    
    
    FILE *fs;
    fs = fopen("result.csv", "a");
	fprintf(fs, "%f\n",accelerationBuffer.mean3DAcceleration);
	fclose(fs);
	
	
}




float getAbsAcceleration(int32_t x, int32_t y, int32_t z)
{
    
    FILE *fs;
    fs = fopen("result.csv", "a");
    //float a = 100;
    //int16_t x, y, z;
    float xFloat, yFloat, zFloat;   
    //This constant value is a multiplier needed to evaluate acceleration on earth
    float earthAccelerationMultiplier = 0.04f;
    float result;

    //abacus_sensors_acc_read(&x, &y, &z);
    //This conversion front int16_t to float is needed to evaluate with more precision the absolute value of acceleration
    xFloat = (float)(x) * earthAccelerationMultiplier;
    yFloat = (float)(y) * earthAccelerationMultiplier;
    zFloat = (float)(z) * earthAccelerationMultiplier;
    result = (float) floor( sqrt(xFloat * xFloat + yFloat * yFloat + zFloat * zFloat ) ) ;
    fprintf(fs, "%f,%f,%f,",xFloat,yFloat,zFloat);

    fclose(fs);
    return result;
}

void setAbsAcceleration()
{
    int32_t x,y,z;
    float absAccelerationValue=getAbsAcceleration(x,y,z);
    addAcceleration(&absAccelerationValue);
    
}


uint8_t checkAccelerationAtStartTime()
{
    //printf("Valore medio: %d",getMeanAcceleration());
    if(getMeanAcceleration()>THRESHOLD_ACCELERATION){

        return 0xff;
    }
    else
    {
        return 0;
    }
}





uint8_t isSatelliteInSpace()
{
    uint8_t result=0x0;
    uint16_t i;
    printf("TEST\n");
    //for(i=0;i<2*ACCELERATION_BUFFER_SIZE;++i){
    //    setAbsAcceleration();
        //blink_routine(100);

    //}
    if(!checkAccelerationAtStartTime())
    {
        //abacus_sleep_msec(100000UL); //number of msecseconds = 100 sec
        result=0xff;
    }
    return result;
}




