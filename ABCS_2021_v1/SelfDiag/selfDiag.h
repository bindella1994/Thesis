

#ifndef INCLUDE_SELFDIAG_H_
#define INCLUDE_SELFDIAG_H_
#include "abacus.h"

//Accelerometro
//abacusversion2013 and abacusversion2014
#define AB_ADDRESS_ACC_ID               0xE5
#define ADXL345_STD_CONFIG              0x00
#define ADXL345DEVID                    0x00
//abacusversion2017
#define MPU9250_IMU_ID			0x71
#define MPU9250_ACC_STD_CONFIG		0x00	


//Giroscopio
//abacusversion2013 and abacusversion2014
#define AB_ADDRESS_GYR_ID               0xD3
#define L3G4200_DEVID                   0x0F
#define L3G4200_STD_CONFIG              0x00
//abacusversion2017 lo stesso dell'accelerometro
#define MPU9250_GYR_STD_CONFIG		0x00


//Magnetometro
//abacusversion2013 and abacusversion2014
#define HMC5883_DEVID                   0x48   
#define HMC5883_STD_CONFIG              0x00
#define HMC5883IDREG_A                  0x0A
//abacusversion2017
#define AK8963_DEVID                   0x48
#define AK8963_STD_CONFIG              0x00


//RTC (Real Time Clock)
#define BQ32000_STD_CONFIG              0x80
#define BQ32000CAL_CFG1                 0x07
//CPU temp sensor
#define DS1775_CPU_STD_CONFIG           0x60

//FPGA temp sensor
#define DS1775_FPGA_STD_CONFIG          0x60


//Codifica della priorità di autodiagnosi
// Il byte che manterrà ì'nformazione avrà dunque la seguente struttura:
//  0b xxxx xxxx ---> Ogni singolo bit (tranne il secondo che non è utilizzato) avrà la seguente logica:
//                    0 -> Sensore funzionante
//                    1 -> Sensore non funzionante
//

#define AUTO_I2C_RETURN_VALUE           0x80

#define AUTO_FPGA_TEMP_RETURN_VALUE     0x20
#define AUTO_CPU_TEMP_RETURN_VALUE      0x10
#define AUTO_RTC_RETURN_VALUE           0x08
#define AUTO_MAG_RETURN_VALUE           0x04
#define AUTO_GYR_RETURN_VALUE           0x02
#define AUTO_ACC_RETURN_VALUE           0x01


//Valori di identificazione della memoria flash
#define FLASH_IDEN_BYTE_0 0x20
#define FLASH_IDEN_BYTE_1 0x20
#define FLASH_IDEN_BYTE_2 0x18


//#pragma PERSISTENT(PersistentRam)
struct DiagnosticFlags
{
        uint8_t I2CFlag;                           
        uint8_t accelerometerSensorFlag;
        uint8_t gyroscopeSensorFlag;
        uint8_t magnetometerSensorFlag;
        uint8_t fpgaTemperatureSensorFlag;

        uint8_t cpuTemperatureSensorFlag;
        uint8_t rtcFlag;
        uint8_t flashFlag;
        uint8_t fpgaTemperatureSensorSecondFlag;
        uint8_t cpuTemperatureSensorSecondFlag;		
}diagnosticFlags;
//={ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

//DiagnosticFlags diagnosticFlags;
//Funzioni che controllano il funzionamento di ogni singolo sensore
uint8_t     checkAccelerometerId(uint8_t* flag);
uint8_t     checkGyroscopeId(uint8_t* flag);
uint8_t     checkMagnetometerId(uint8_t* flag);
uint8_t     checkRtcId(uint8_t* flag);
uint8_t     checkTempCpuId(uint8_t* flag);
uint8_t     checkTempFpgaId(uint8_t* flag);

void controlIntegrity(uint32_t timeInterval);


//Funzionne di controllo della memoria flash
void        checkFlashMemoryId(uint8_t* flag);


/*Funzione che permette di valutare la maggioranza di 0 o 1 all'interno di un byte.
* In questo la corruzione di pochi bit non inficia sulla valutazione del valore della flag
*
* Esempio:
* Flag_x=0xff;   => 0b 1111 1111
* Tramite una corruzione vedo:
* Flag_x=0x3f;   => 0b 0011 1111
* La funzione decide per flag=1 correttamente
* soltanto 5 corruzione o pi� di fatto mi danno un valore sbagliato.
*
*/
uint8_t     majorityDecisionByte(uint8_t value);


void sensorDiag(void);
void memoryDiag(void);

int8_t selfDiagInit();





#endif /* INCLUDE_SELFDIAG_H_ */
