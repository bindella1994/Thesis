
#include "Acceleration.h"
#include <math.h>
#include "abacus.h"

void addAcceleration(const int32_t* a)
{
    accelerationBuffer.buffer[accelerationBuffer.index % ACCELERATION_BUFFER_SIZE] = *a;

    if(accelerationBuffer.index < ACCELERATION_BUFFER_SIZE){
        ++accelerationBuffer.index;
    }else{
        accelerationBuffer.index=0;
    }

    if(!accelerationBuffer.full && ( accelerationBuffer.index == ACCELERATION_BUFFER_SIZE ) ){
        accelerationBuffer.full = 0xff;
    }
    accelerationBuffer.mean3DAcceleration = getMeanAcceleration();

}

void setAbsAcceleration()
{
    int32_t absAccelerationValue=getAbsAcceleration();
    addAcceleration(&absAccelerationValue);
}


int32_t getAbsAcceleration()
{

    int16_t x, y, z;
    float xFloat, yFloat, zFloat;   
    //This constant value is a multiplier needed to evaluate acceleration on earth
    float earthAccelerationMultiplier = 0.04f;
    int32_t result;

    abacus_sensors_acc_read(&x, &y, &z);
    //This conversion front int16_t to float is needed to evaluate with more precision the absolute value of acceleration
    xFloat = (float)(x) * earthAccelerationMultiplier;
    yFloat = (float)(y) * earthAccelerationMultiplier;
    zFloat = (float)(z) * earthAccelerationMultiplier;
    result = (int32_t) floor( sqrt(xFloat * xFloat + yFloat * yFloat + zFloat * zFloat ) ) ;
    return result;
}


int32_t getMeanAcceleration()
{
    int32_t result=0;
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


uint8_t checkAccelerationAtStartTime()
{

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
    for( i=2*ACCELERATION_BUFFER_SIZE;i>0; --i){
        setAbsAcceleration();
        //blink_routine(100);

    }
    if(!checkAccelerationAtStartTime())
    {
        abacus_sleep_msec(100000UL); //number of msecseconds = 100 sec
        result=0xff;
    }
    return result;
}
