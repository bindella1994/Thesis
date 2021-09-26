//Inizializzazione delle librerie necessarie al funzionamento
#include "selfDiag.h"
#include "abacus.h"


/*
 * Quelle flag riferite ad ogni parte dell'hardware definite come persistent
 * ci permettono di monitorare anche dopo un reset l'effettivo funzionamento.
 * flag=0 significa funzionante
 * flag=1 significa da escludere direttamente all'avvio
 *
 */


uint8_t     majorityDecisionByte(uint8_t value){
    uint8_t result = 0;
    uint8_t counter = 0;
    uint8_t bit = 0;
    uint8_t mask;
    uint8_t i;
    for(i=0;i<8;++i) {
        mask = 1<<i;
        bit = value & mask;
        if( bit != 0x0 ) {
            ++counter;
        }
    }
    if(counter>3){
        result = 0xff;
    }
    return result;
}

/*raises flag for non functional sensors*/
void sensorDiag(void)
{
    uint8_t counter=0;
    if ( majorityDecisionByte(diagnosticFlags.I2CFlag) == 0x0 )
    {


        // Prima di controllare ogni singolo sensore, ad ogni iterazione si esegue un controllo sul bus I2C condiviso tra tutti i sottosistemi interrogati.
        // Questo evita il ricontrollo del bus I2C e di conseguenza un possibile overflow al contatore (verrebbe sommato per n volte 0x80
        // Evito per sicurezza il controllo dei sensori e li disabilito a priori nel caso in cui il bus non sia funzionante

        if( ( majorityDecisionByte(diagnosticFlags.accelerometerSensorFlag) == 0x0) )
        {
            counter+=checkAccelerometerId(&diagnosticFlags.accelerometerSensorFlag);
        }
        else
        {
            //Se riscontro un flag alta, per sicurezza escludo il sensore
            abacus_sensors_acc_power_off();
        }


        /********************/
        if( (majorityDecisionByte(diagnosticFlags.gyroscopeSensorFlag)==0x0) )
        {
            counter+=checkGyroscopeId(&diagnosticFlags.gyroscopeSensorFlag);
        }
        else
        {
            abacus_sensors_gyro_power_off();
        }

        /*******************/
        if( ( majorityDecisionByte(diagnosticFlags.magnetometerSensorFlag)==0x0)  && ( majorityDecisionByte(diagnosticFlags.I2CFlag) == 0x0 ) )
        {
            counter+=checkMagnetometerId(&diagnosticFlags.magnetometerSensorFlag);
        }
        else
        {
            abacus_sensors_magnetometer_power_off();
        }
        /*******************/
        if( ( majorityDecisionByte(diagnosticFlags.cpuTemperatureSensorFlag)==0x0)  && ( majorityDecisionByte(diagnosticFlags.I2CFlag) == 0x0 ) )
        {
            counter+=checkTempCpuId(&diagnosticFlags.cpuTemperatureSensorFlag);
        }
        else
        {
            if( (majorityDecisionByte(diagnosticFlags.cpuTemperatureSensorSecondFlag)==0x0)  && ( majorityDecisionByte(diagnosticFlags.I2CFlag) == 0x0 ) )
            {
                counter+=checkTempCpuId(&diagnosticFlags.cpuTemperatureSensorSecondFlag);
            }
            else
            {
                abacus_sensors_temperatureCPU_power_off();
            }
        }
        /********************/
        if( ( majorityDecisionByte(diagnosticFlags.fpgaTemperatureSensorFlag)==0x0) && ( majorityDecisionByte(diagnosticFlags.I2CFlag) == 0x0 ) )
        {
            counter+=checkTempFpgaId(&diagnosticFlags.fpgaTemperatureSensorFlag);
        }
        else
        {
            if( ( majorityDecisionByte(diagnosticFlags.fpgaTemperatureSensorSecondFlag)==0x0) && ( majorityDecisionByte(diagnosticFlags.I2CFlag) == 0x0 ) )
            {
                counter+=checkTempFpgaId(&diagnosticFlags.fpgaTemperatureSensorSecondFlag);
            }
            else
            {
                abacus_sensors_temperatureFPGA_power_off();
            }
        }

        if( ( majorityDecisionByte(diagnosticFlags.rtcFlag)==0x0) && ( majorityDecisionByte(diagnosticFlags.I2CFlag) == 0x0 ) )
        {
            counter+=checkRtcId(&diagnosticFlags.rtcFlag);
        }
    }
    else
    {
        counter = 0xff;
        diagnosticFlags.accelerometerSensorFlag = 0xff;
        diagnosticFlags.I2CFlag = 0xff;
        diagnosticFlags.accelerometerSensorFlag  = 0xff;
        diagnosticFlags.gyroscopeSensorFlag  = 0xff;
        diagnosticFlags.magnetometerSensorFlag = 0xff;
        diagnosticFlags.fpgaTemperatureSensorFlag  = 0xff;
        diagnosticFlags.cpuTemperatureSensorFlag = 0xff;
        diagnosticFlags.rtcFlag = 0xff;
        diagnosticFlags.fpgaTemperatureSensorSecondFlag = 0xff;
        diagnosticFlags.cpuTemperatureSensorSecondFlag = 0xff;
    }

    if(counter > 0x8)
    {                                             
        WDTCTL = 0xDEAD;
    }


}


/*check for the memory issues and implement appropriate routines*/
void memoryDiag(void)
{
    //Da completare
    if(majorityDecisionByte(diagnosticFlags.flashFlag)==0x0)
    {
        checkFlashMemoryId(&diagnosticFlags.flashFlag);
    }

}

/*
 * Ogni richiesta di controllo del determinato sensore avviene tramite protocollo I2C. Questo protocollo si svolge in due parti: prima si richiede la determinata risorsa, poi il sensore ris
 * ponde alla determinata richiesta. Ci� non avviene sempre e qualcosa pu� andare storto (sensore rotto, bus I2C non funzionante). Per evitare che il codice si blocchi imponiamo quindi un
 * tempo limite di risposta, pari ad un numero precisato di operazioni. Se il contatore raggiunge il limite, significa che il bus non risponde e di conseguenza bisogna supporre che entrambe
 * le risorse (sensore e bus) non sono utilizzabili. In tal caso le flag a byte rimangono alte e dopo un software reset obbligato le risorse vengono escluse o il problema viene risolto.
 * Come ultima spiaggia abbiamo inoltre attivato il watchdog timer che ci permette di non bloccare il codice nel caso in cui tutta la procedura venga bloccata da un evento inaspettato.
 *
 *
 *
 */
uint8_t checkAccelerometerId(uint8_t* flag)
{



    //WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;
    *flag=0xFF;
    diagnosticFlags.I2CFlag=0xFF;

    uint8_t counter=0;
    uint8_t result=0;

    /*
     **  Una volta appurato che il bus I2C risponde e ci fornisce la determinata risorsa, bisogna valutare che i registri non siano corrotti.
     **  Per Fare ci� vi � un registro dedicato nella maggior parte dei sensori chiamato "Device ID" che ci permette di leggere il nome del sensore.
     **  Se ci� non avviene correttamente, il registro � corrotto.
     **  Nel caso in cui il registro non sia corrotto andiamo poi ad indagare il registro di configurazione, il quale deve essere impostato secondo un determinato valore
     **  esadecimale (gi� deciso a priori). Se ci� non avviene significa che qualcosa � cambiato e ci� � inaspettato.
     **  In tale caso, andiamo a resettare le impostazioni del sensore.
     **
     **  SINTASSI DELLA RICHIESTA I2C DI LETTURA DI UN REGISTRO:
     **    while(uint8_t abacus_i2c_readRegister(uint8_t busSelect,uint8_t address, uint8_t reg)
     */


    while( ( result = abacus_i2c_readRegister(AB_I2C_BUS00, AB_ADDRESS_ACC, ADXL345DEVID) ) == 0x00)
    {
        counter++;

        if(counter > 30000)             //i2c is not responding, power reset, so i both acc and i2c not working to be sure
        {
            result = AUTO_ACC_RETURN_VALUE+AUTO_I2C_RETURN_VALUE;
            /*
             *
             * Il valore dipende dalla priorit� con cui bisogna risolvere il problema: questo identifica che sia il bus IC che l'accelerometro sono non funzionanti:
             *
             * 0b1000 0000 + 0b0000 0001 = 0b1000 0001 = 129
             *  I2C fault     acc FAULT
             *
             *
             *
             */
        }
    }

    diagnosticFlags.I2CFlag=0x00;

    if(abacus_i2c_readRegister(AB_I2C_BUS00, AB_ADDRESS_ACC, ADXL345CONFIGREG)!=ADXL345_STD_CONFIG)
    {
        //Calibra il sensore
        abacus_sensors_acc_init();
    }

    if(result==AB_ADDRESS_ACC_ID)
    {
        *flag=0x00;  //both sensor and i2c working, return is 0
        result = 0;
    }
    else
    {
        *flag=0xff;  //only sensor not working, return 1
        result = AUTO_ACC_RETURN_VALUE;
    }

    return result;

}


uint8_t checkGyroscopeId(uint8_t* flag)
{
    diagnosticFlags.gyroscopeSensorFlag=0xff;
    /*
     * Ogni richiesta di controllo del determinato sensore avviene tramite protocollo I2C. Questo protocollo si svolge in due parti: prima si richiede la determinata risorsa, poi il sensore ris
     * ponde alla determinata richiesta. Ci� non avviene sempre e qualcosa pu� andare storto (sensore rotto, bus I2C non funzionante). Per evitare che il codice si blocchi imponiamo quindi un
     * tempo limite di risposta, pari ad un numero precisato di operazioni. Se il contatore raggiunge il limite, significa che il bus non risponde e di conseguenza bisogna supporre che entrambe
     * le risorse (sensore e bus) non sono utilizzabili. In tal caso le flag a byte rimangono alte e dopo un software reset obbligato le risorse vengono escluse o il problema viene risolto.
     * Come ultima spiaggia abbiamo inoltre attivato il watchdog timer che ci permette di non bloccare il codice nel caso in cui tutta la procedura venga bloccata da un evento inaspettato.
     *
     *
     *
     */


    WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;
    *flag=0xFF;
    diagnosticFlags.I2CFlag=0xFF;

    uint8_t counter=0;
    uint8_t result=0;


    /*
     **  Una volta appurato che il bus I2C risponde e ci fornisce la determinata risorsa, bisogna valutare che i registri non siano corrotti.
     **  Per Fare ci� vi � un registro dedicato nella maggior parte dei sensori chiamato "Device ID" che ci permette di leggere il nome del sensore.
     **  Se ci� non avviene correttamente, il registro � corrotto.
     **  Nel caso in cui il registro non sia corrotto andiamo poi ad indagare il registro di configurazione, il quale deve essere impostato secondo un determinato valore
     **  esadecimale (gi� deciso a priori). Se ci� non avviene significa che qualcosa � cambiato e ci� � inaspettato.
     **  In tale caso, andiamo a resettare le impostazioni del sensore.
     **
     **  SINTASSI DELLA RICHIESTA I2C DI LETTURA DI UN REGISTRO:
     **    while(uint8_t abacus_i2c_readRegister(uint8_t busSelect,uint8_t address, uint8_t reg)
     */
    //MPU9250_WHO_AM_I         0x75 // Should return 0x71

    while((result = abacus_i2c_readRegister(AB_I2C_BUS00, AB_ADDRESS_GYRO2014, L3G4200_DEVID))==0x00)
    {
        counter++;
        if(counter > 30000)             //i2c is not responding, i reset, so i set both acc and i2c not working to be sure
        {

            result = AUTO_GYR_RETURN_VALUE+AUTO_I2C_RETURN_VALUE;
            /*
             *
             * Il valore dipende dalla priorit� con cui bisogna risolvere il problema: questo identifica che sia il bus IC che l'accelerometro sono non funzionanti:
             *
             * 0b1000 0000 + 0b0000 0010 = 0b1000 0010 = 130
             *  I2C fault     acc FAULT
             *
             *
             *
             */
        }
    }

    /*
     *
     * Vado quindi a controllare il valore di configurazione del sensore, se ci� non va bene, richiamo la funzione di calibrazione
     * del determinato sensore
     *
     *
     *
     */
    diagnosticFlags.I2CFlag=0x00;  // HO quindi abbassato la flag sia del sensore che del bus
    if(abacus_i2c_readRegister(AB_I2C_BUS00, AB_ADDRESS_GYRO2013, L3G4200CONFIGREG)!=L3G4200_STD_CONFIG)
    {
        //Calibra il sensore
        abacus_sensors_gyro_init(AB_ADDRESS_GYRO2014);
    }

    if(result==AB_ADDRESS_GYR_ID )
    {
        *flag=0x00;   //both sensor and i2c working, return is 0
        result = 0;
    }
    else
    {
        *flag=0xff;   //only sensor not working, return 2
        result = AUTO_GYR_RETURN_VALUE;
    }
    return result;
}


uint8_t checkMagnetometerId(uint8_t* flag)
{

    /*
     * Ogni richiesta di controllo del determinato sensore avviene tramite protocollo I2C. Questo protocollo si svolge in due parti: prima si richiede la determinata risorsa, poi il sensore ris
     * ponde alla determinata richiesta. Ci� non avviene sempre e qualcosa pu� andare storto (sensore rotto, bus I2C non funzionante). Per evitare che il codice si blocchi imponiamo quindi un
     * tempo limite di risposta, pari ad un numero precisato di operazioni. Se il contatore raggiunge il limite, significa che il bus non risponde e di conseguenza bisogna supporre che entrambe
     * le risorse (sensore e bus) non sono utilizzabili. In tal caso le flag a byte rimangono alte e dopo un software reset obbligato le risorse vengono escluse o il problema viene risolto.
     * Come ultima spiaggia abbiamo inoltre attivato il watchdog timer che ci permette di non bloccare il codice nel caso in cui tutta la procedura venga bloccata da un evento inaspettato.
     *
     *
     *
     */


    WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;
    *flag=0xFF;
    diagnosticFlags.I2CFlag=0xFF;

    uint8_t counter=0;
    uint8_t result=0;
    /*
     **  Una volta appurato che il bus I2C risponde e ci fornisce la determinata risorsa, bisogna valutare che i registri non siano corrotti.
     **  Per Fare ci� vi � un registro dedicato nella maggior parte dei sensori chiamato "Device ID" che ci permette di leggere il nome del sensore.
     **  Se ci� non avviene correttamente, il registro � corrotto.
     **  Nel caso in cui il registro non sia corrotto andiamo poi ad indagare il registro di configurazione, il quale deve essere impostato secondo un determinato valore
     **  esadecimale (gi� deciso a priori). Se ci� non avviene significa che qualcosa � cambiato e ci� � inaspettato.
     **  In tale caso, andiamo a resettare le impostazioni del sensore.
     **
     **  SINTASSI DELLA RICHIESTA I2C DI LETTURA DI UN REGISTRO:
     **    while(uint8_t abacus_i2c_readRegister(uint8_t busSelect,uint8_t address, uint8_t reg)
     */

    while((result = abacus_i2c_readRegister(AB_I2C_BUS00, AB_ADDRESS_MAGNETOMETER, HMC5883IDREG_A))==0x00)
    {
        counter++;
        if(counter > 30000)             //i2c is not responding, i reset, so i set both acc and i2c not working to be sure
        {
            result = AUTO_MAG_RETURN_VALUE+AUTO_I2C_RETURN_VALUE;
            /*
             *
             * Il valore dipende dalla priorit� con cui bisogna risolvere il problema: questo identifica che sia il bus IC che l'accelerometro sono non funzionanti:
             *
             * 0b1000 0000 + 0b0000 0100 = 0b1000 0100 = 132
             *  I2C fault     acc FAULT
             *
             *
             *
             */
        }
    }
    //I2C replied with good timing, so is working, lets see if the ID is correct

    diagnosticFlags.I2CFlag=0x00;  // HO quindi abbassato la flag sia del sensore che del bus


    /*
     *
     * Vado quindi a controllare il valore di configurazione del sensore, se ci� non va bene, richiamo la funzione di calibrazione
     * del determinato sensore
     *
     *
     *
     */

    //Flag_Diagnostic_Part.I2CFlag=0x00;  // HO quindi abbassato la flag sia del sensore che del bus
    if(abacus_i2c_readRegister(AB_I2C_BUS00, AB_ADDRESS_MAGNETOMETER, HMC5883MODEREG)!=HMC5883_STD_CONFIG)
    {
        //Calibra il sensore
        abacus_sensors_magnetometer_init();
    }

    if(result==HMC5883_DEVID)
    {
        *flag=0x00;  //both sensor and i2c working, return is 0
        result = 0;
    }
    else
    {
        *flag=0xff;  //only sensor not working, return 1
        result = AUTO_MAG_RETURN_VALUE;
    }
    return result;
}


uint8_t checkRtcId(uint8_t* flag)
{

    /*
     * Ogni richiesta di controllo del determinato sensore avviene tramite protocollo I2C. Questo protocollo si svolge in due parti: prima si richiede la determinata risorsa, poi il sensore ris
     * ponde alla determinata richiesta. Ci� non avviene sempre e qualcosa pu� andare storto (sensore rotto, bus I2C non funzionante). Per evitare che il codice si blocchi imponiamo quindi un
     * tempo limite di risposta, pari ad un numero precisato di operazioni. Se il contatore raggiunge il limite, significa che il bus non risponde e di conseguenza bisogna supporre che entrambe
     * le risorse (sensore e bus) non sono utilizzabili. In tal caso le flag a byte rimangono alte e dopo un software reset obbligato le risorse vengono escluse o il problema viene risolto.
     * Come ultima spiaggia abbiamo inoltre attivato il watchdog timer che ci permette di non bloccare il codice nel caso in cui tutta la procedura venga bloccata da un evento inaspettato.
     *
     *
     *
     */


    WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;
    *flag=0xFF;
    diagnosticFlags.I2CFlag=0xFF;

    uint16_t counter=0;
    uint8_t result=0;

    /*
     **  Una volta appurato che il bus I2C risponde e ci fornisce la determinata risorsa, bisogna valutare che i registri non siano corrotti.
     **  Per Fare ci� vi � un registro dedicato nella maggior parte dei sensori chiamato "Device ID" che ci permette di leggere il nome del sensore.
     **  Se ci� non avviene correttamente, il registro � corrotto.
     **  Nel caso in cui il registro non sia corrotto andiamo poi ad indagare il registro di configurazione, il quale deve essere impostato secondo un determinato valore
     **  esadecimale (gi� deciso a priori). Se ci� non avviene significa che qualcosa � cambiato e ci� � inaspettato.
     **  In tale caso, andiamo a resettare le impostazioni del sensore.
     **
     **  SINTASSI DELLA RICHIESTA I2C DI LETTURA DI UN REGISTRO:
     **    while(uint8_t abacus_i2c_readRegister(uint8_t busSelect,uint8_t address, uint8_t reg)
     */

    while((result = abacus_i2c_readRegister(AB_I2C_BUS00, AB_ADDRESS_RTC, BQ32000CAL_CFG1))==0x00)
    {
        counter++;
        if(counter > 30000)             //i2c is not responding, i reset, so i set both acc and i2c not working to be sure
        {
            result = AUTO_RTC_RETURN_VALUE+AUTO_I2C_RETURN_VALUE;

            /*
             *
             * Il valore dipende dalla priorit� con cui bisogna risolvere il problema: questo identifica che sia il bus IC che l'accelerometro sono non funzionanti:
             *
             * 0b1000 0000 + 0b0000 1000 = 0b1000 1000 = 136
             *  I2C fault     RTC FAULT
             *
             *
             *
             */

        }
    }
    /*
     *
                buffertmp = abacus_i2c_readRegister(AB_I2C_BUS00, AB_ADDRESS_RTC, BQ32000CAL_CFG1);
                //I2C replied with good timing, so is working, lets see if the ID is correct
     *
     */
    diagnosticFlags.I2CFlag=0x00;  // HO quindi abbassato la flag sia del sensore che del bus
    if(result==BQ32000_STD_CONFIG)
    {
        *flag=0x00;  //both sensor and i2c working, return is 0
        result = 0;
    }
    else
    {
        *flag=0xff;  /*L'RTC � uno strumento fondamentale per il funzionamento del sistema. Bisogna creare un sistema che
        * lo escluda soltanto nel caso di guasto continuativo (anche se lo resetto). Questo potrebbe essere fatto
        *  sfruttando la flag che ho alzato non come una semplice flag che esclude direttamente al reset, ma con un sistema
        * pi� complesso come questo:
        * 1) Avvio abacus e l'RTC � certamente attivo e con la flag abbassata (a 0x0 )
        * 2) Se riscontro un problema, alzo la flag, reimposto l'RTC e rebootto.
        * 3) Al reboot riscontro la flag alta. Esiste per� una seconda flag che tiene conto se il problema � perdurato e questa � ancora bassa.
        *  Attivo quindi l'RTC e eseguo tutta la fase di autodiagnosi.
        *  4) Se non riscontro problemi, era un semplice problema di settaggi. Se riscontro nuovamente un problema (bus bloccato, config reg diverso)
        *  significa che il problema � perdurato. Attivo di conseguenza la seconda flag (Sensor_broken_flag) e rebootto.
        *  5) In questo caso tutte e due le flag sono attive e so che non devo attivare l'RTC
        *
        *  Ricapitolando:
        *  HO due flag:
        *  a) Flag_Diagnostic_Part.RTCFlag
        *  b) Flag_Diagnostic_Part.RTCbrokenflag
        *
        *  Possibilit�:
        *
        *  1) RTCFlag == 0 && RTCbrokenflag == 0 tutto funziona bene
        *  2) RTCFlag == 1 && RTCbrokenflag == 0 riscontro per la prima volta un problema
        *  3) RTCFlag == 0 && RTCbrokenflag == 1 NON SUCCEDE MAI, PERCH� IMPONGO CHE LA BROKEN FLAG SIA MODIFICABILE DOPO LA RTC FLAG
        *  2) RTCFlag == 1 && RTCbrokenflag == 1 problema perdurato, devo disattivare l'RTC
        */
        abacus_RTC_init();                  // Non sono sicuro che calibri in maniera giusta. Nella libreria sembra resettare tutto al 1 gennaio 2000
        result = AUTO_RTC_RETURN_VALUE;
    }
    return result;
}


uint8_t checkTempCpuId(uint8_t* flag)
{

    /*
     * Nel caso dei sensori di temperatura non esiste un device ID. DI conseguenza possiamo controllare soltanto il registro di impostazione
     *
     *
     *
     */


    WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;
    *flag=0xFF;
    diagnosticFlags.I2CFlag=0xFF;

    uint16_t counter=0;
    uint8_t result=0;


    /*
     **
     **  SINTASSI DELLA RICHIESTA I2C DI LETTURA DI UN REGISTRO:
     **    while(uint8_t abacus_i2c_readRegister(uint8_t busSelect,uint8_t address, uint8_t reg)
     */

    while((result = abacus_i2c_readRegister(AB_I2C_BUS00, AB_ADDRESS_TEMPFPGA, DS1775CONFIGREG))==0x00)
    {
        counter++;
        if(counter > 30000)             //i2c is not responding, i reset, so i set both acc and i2c not working to be sure
        {
            result = AUTO_CPU_TEMP_RETURN_VALUE+AUTO_I2C_RETURN_VALUE;
            /*
             *
             * Il valore dipende dalla priorit� con cui bisogna risolvere il problema: questo identifica che sia il bus IC che l'accelerometro sono non funzionanti:
             *
             * 0b1000 0000 + 0b0010 0000 = 0b1010 0000 = 128+32 = 160
             *  I2C fault     FPGA temp FAULT
             *
             *
             *
             */
        }
    }
    //buffertmp = abacus_i2c_readRegister(AB_I2C_BUS00, AB_ADDRESS_TEMPCPU, DS1775CONFIGREG);
    //I2C replied with good timing, so is working, lets see if the ID is correct
    diagnosticFlags.I2CFlag=0x00;  // HO quindi abbassato la flag sia del sensore che del bus
    if(result==DS1775_CPU_STD_CONFIG)
    {
        *flag=0x00;  //both sensor and i2c working, return is 0
        result = 0;
    }
    else
    {
        *flag=0xff;  //Stesso discorso dell'RTC
        /*L'RTC � uno strumento fondamentale per il funzionamento del sistema. Bisogna creare un sistema che
         * lo escluda soltanto nel caso di guasto continuativo (anche se lo resetto). Questo potrebbe essere fatto
         *  sfruttando la flag che ho alzato non come una semplice flag che esclude direttamente al reset, ma con un sistema
         * pi� complesso come questo:
         * 1) Avvio abacus e l'RTC � certamente attivo e con la flag abbassata (a 0x0 )
         * 2) Se riscontro un problema, alzo la flag, reimposto l'RTC e rebootto.
         * 3) Al reboot riscontro la flag alta. Esiste per� una seconda flag che tiene conto se il problema � perdurato e questa � ancora bassa.
         *  Attivo quindi l'RTC e eseguo tutta la fase di autodiagnosi.
         *  4) Se non riscontro problemi, era un semplice problema di settaggi. Se riscontro nuovamente un problema (bus bloccato, config reg diverso)
         *  significa che il problema � perdurato. Attivo di conseguenza la seconda flag (Sensor_broken_flag) e rebootto.
         *  5) In questo caso tutte e due le flag sono attive e so che non devo attivare l'RTC
         *
         *  Ricapitolando:
         *  HO due flag:
         *  a) Flag_Diagnostic_Part.RTCFlag
         *  b) Flag_Diagnostic_Part.RTCbrokenflag
         *
         *  Possibilit�:
         *
         *  1) RTCFlag == 0 && RTCbrokenflag == 0 tutto funziona bene
         *  2) RTCFlag == 1 && RTCbrokenflag == 0 riscontro per la prima volta un problema
         *  3) RTCFlag == 0 && RTCbrokenflag == 1 NON SUCCEDE MAI, PERCH� IMPONGO CHE LA BROKEN FLAG SIA MODIFICABILE DOPO LA RTC FLAG
         *  2) RTCFlag == 1 && RTCbrokenflag == 1 problema perdurato, devo disattivare l'RTC
         */
        abacus_sensors_temperature_init(); //Vado a ricalibrare entrambi i sensori di temperatura. Potrei anche generare una funzione che calibri soltanto
        //uno dei sensori di temperatura
        result = AUTO_CPU_TEMP_RETURN_VALUE;
    }
    return result;
}




uint8_t checkTempFpgaId(uint8_t* flag)
{

    /*
     * Nel caso dei sensori di temperatura non esiste un device ID. DI conseguenza possiamo controllare soltanto il registro di impostazione
     *
     *
     *
     */


    WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;
    *flag=0xFF;
    diagnosticFlags.I2CFlag=0xFF;

    uint16_t counter=0;
    uint8_t result=0;

    /*
     **
     **  SINTASSI DELLA RICHIESTA I2C DI LETTURA DI UN REGISTRO:
     **    while(uint8_t abacus_i2c_readRegister(uint8_t busSelect,uint8_t address, uint8_t reg)
     */

    while((result = abacus_i2c_readRegister(AB_I2C_BUS00, AB_ADDRESS_TEMPFPGA, DS1775CONFIGREG))==0x00)
    {
        counter++;
        if(counter > 30000)             //i2c is not responding, i reset, so i set both acc and i2c not working to be sure
        {
            result = AUTO_CPU_TEMP_RETURN_VALUE+AUTO_I2C_RETURN_VALUE;
            /*
             *
             * Il valore dipende dalla priorit� con cui bisogna risolvere il problema: questo identifica che sia il bus IC che l'accelerometro sono non funzionanti:
             *
             * 0b1000 0000 + 0b0010 0000 = 0b1010 0000 = 128+32 = 160
             *  I2C fault     FPGA temp FAULT
             *
             *
             *
             */
        }
    }
    //buffertmp = abacus_i2c_readRegister(AB_I2C_BUS00, AB_ADDRESS_TEMPFPGA, DS1775CONFIGREG);
    //I2C replied with good timing, so is working, lets see if the ID is correct
    diagnosticFlags.I2CFlag=0x00;  // HO quindi abbassato la flag sia del sensore che del bus
    if(result==DS1775_FPGA_STD_CONFIG)
    {
        *flag=0x00;  //both sensor and i2c working, return is 0
        result = 0;
    }
    else
    {
        *flag=0xff;  //Stesso discorso dell'RTC
        /*L'RTC � uno strumento fondamentale per il funzionamento del sistema. Bisogna creare un sistema che
         * lo escluda soltanto nel caso di guasto continuativo (anche se lo resetto). Questo potrebbe essere fatto
         *  sfruttando la flag che ho alzato non come una semplice flag che esclude direttamente al reset, ma con un sistema
         * pi� complesso come questo:
         * 1) Avvio abacus e l'RTC � certamente attivo e con la flag abbassata (a 0x0 )
         * 2) Se riscontro un problema, alzo la flag, reimposto l'RTC e rebootto.
         * 3) Al reboot riscontro la flag alta. Esiste per� una seconda flag che tiene conto se il problema � perdurato e questa � ancora bassa.
         *  Attivo quindi l'RTC e eseguo tutta la fase di autodiagnosi.
         *  4) Se non riscontro problemi, era un semplice problema di settaggi. Se riscontro nuovamente un problema (bus bloccato, config reg diverso)
         *  significa che il problema � perdurato. Attivo di conseguenza la seconda flag (Sensor_broken_flag) e rebootto.
         *  5) In questo caso tutte e due le flag sono attive e so che non devo attivare l'RTC
         *
         *  Ricapitolando:
         *  HO due flag:
         *  a) Flag_Diagnostic_Part.RTCFlag
         *  b) Flag_Diagnostic_Part.RTCbrokenflag
         *
         *  Possibilit�:
         *
         *  1) RTCFlag == 0 && RTCbrokenflag == 0 tutto funziona bene
         *  2) RTCFlag == 1 && RTCbrokenflag == 0 riscontro per la prima volta un problema
         *  3) RTCFlag == 0 && RTCbrokenflag == 1 NON SUCCEDE MAI, PERCH� IMPONGO CHE LA BROKEN FLAG SIA MODIFICABILE DOPO LA RTC FLAG
         *  2) RTCFlag == 1 && RTCbrokenflag == 1 problema perdurato, devo disattivare l'RTC
         */
        abacus_sensors_temperature_init(); //Vado a ricalibrare entrambi i sensori di temperatura. Potrei anche generare una funzione che calibri soltanto
        //uno dei sensori di temperatura
        result = AUTO_FPGA_TEMP_RETURN_VALUE;
    }
    return result;
}



uint8_t majorityDecisionChar(uint8_t value)
{
    uint8_t counter=0;
    uint8_t k=0;
    for (k=0;k<8;k++)
    {
        counter+=((value & (1<<k))>>k);
    }
    if(counter>3)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/**************************************************/

void checkFlashMemoryId(uint8_t* flag)
{

    /*
     * Ogni richiesta di controllo del determinato sensore avviene tramite protocollo I2C. Questo protocollo si svolge in due parti: prima si richiede la determinata risorsa, poi il sensore ris
     * ponde alla determinata richiesta. Ci� non avviene sempre e qualcosa pu� andare storto (sensore rotto, bus I2C non funzionante). Per evitare che il codice si blocchi imponiamo quindi un
     * tempo limite di risposta, pari ad un numero precisato di operazioni. Se il contatore raggiunge il limite, significa che il bus non risponde e di conseguenza bisogna supporre che entrambe
     * le risorse (sensore e bus) non sono utilizzabili. In tal caso le flag a byte rimangono alte e dopo un software reset obbligato le risorse vengono escluse o il problema viene risolto.
     * Come ultima spiaggia abbiamo inoltre attivato il watchdog timer che ci permette di non bloccare il codice nel caso in cui tutta la procedura venga bloccata da un evento inaspettato.
     *
     *
     *
     */


    WDTCTL = (WDTCTL & 0x00FF) | WDTPW | WDTCNTCL;
    *flag=0xFF;

    uint8_t result=0;

    //abacus_flashMCU_init();

    //Get the 3 byte identification
    if( (memoryFlashMCU.flash_identification[0]==FLASH_IDEN_BYTE_0) &&
            (memoryFlashMCU.flash_identification[1]==FLASH_IDEN_BYTE_1) &&
            (memoryFlashMCU.flash_identification[2]==FLASH_IDEN_BYTE_2) )
    {
        *flag=0x00;
    }
    else
    {
        *flag=0xFF;
    }

}

int8_t selfDiagInit()
{
    memoryDiag();
    sensorDiag();

    return 0;
}


void controlIntegrity(uint32_t timeInterval)
{

}



