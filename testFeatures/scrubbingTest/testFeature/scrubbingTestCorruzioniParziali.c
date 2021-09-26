/*
 ============================================================================
 Name        : main.c
 Author      : Manuel Banneòlla
 Version     : 1.0
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "scrub.h"
#include <time.h>
extern struct persistent_RAM persistent_RAM_;
extern struct persistent_RAM persistent_RAM_Copy_;
extern struct SatelliteStatus satelliteStatus_;


int main(int argc, char *argv[]) {
    srand(time(NULL));
    int nErrors = 0;
    int k;

    int iterations = 10;
    int errorPerIteration = 1;
    if(argv[1]!=0){
    	iterations = atoi(argv[1]);
    }
    if(argv[2]!=0){
    	errorPerIteration = atoi(argv[2]);
    }
    
    for(k=0;k<iterations;++k){
    
        initSatelliteStatus((uint8_t*)&satelliteStatus_.totalMissionMinutes, PERSISTENT_RAM_LENGTH);
        
        backupInPersistentRam (&persistent_RAM_.Memory[0],(uint8_t*)&satelliteStatus_.totalMissionMinutes,PERSISTENT_RAM_LENGTH);
        
        //This is a backup needed  for the oracol to detect any persistent corruption in satelliteStatus
        copyPersistentRamForFinalTest(&persistent_RAM_.Memory[0],&persistent_RAM_Copy_.Memory[0],PERSISTENT_RAM_LENGTH);
        
        setScrubParity(&persistent_RAM_.Memory[0],PERSISTENT_RAM_LENGTH/8);

        //printParity();
        int i = 0 ;
        //printf("First Element: %x",satelliteStatus_.totalMissionMinutes);
        for(i = 0; i < errorPerIteration; ++i){
            insertSingleCorruption(&persistent_RAM_.Memory[0],PERSISTENT_RAM_LENGTH,0);
        }
        for(i = 0; i < errorPerIteration; ++i){
            insertSingleCorruption((uint8_t*)&satelliteStatus_.totalMissionMinutes,PERSISTENT_RAM_LENGTH,0);
        }
        //persistent_RAM_.Memory[0]=rand();
        //persistent_RAM_.Memory[0]=3;
        //scrub_recovery(&persistent_RAM_.Memory[0],PERSISTENT_RAM_LENGTH/8);
        //printPersistentRam();
        for(i = 0; i< PERSISTENT_RAM_LENGTH/8 ; ++i) {
        scrub_recovery(&persistent_RAM_.Memory[8*i],i);
        }
        //printParity();
	uint8_t isPersistentGood = 0;
        //initSatelliteStatus((uint8_t*)&satelliteStatus_.totalMissionMinutes, PERSISTENT_RAM_LENGTH);
        for(i = 0; i< PERSISTENT_RAM_LENGTH/8 ; ++i) {
        	scrub_recovery(&persistent_RAM_.Memory[8*i],i);
        
        	if(scrub_recovery(&persistent_RAM_.Memory[8*i],i)!=0){
            		isPersistentGood =0xff;
            		break;
        	}

        }
        
        
        if(isPersistentGood == 0){
        	restoreFromPersistentRam ( &persistent_RAM_.Memory[0],(uint8_t*)&satelliteStatus_.totalMissionMinutes,PERSISTENT_RAM_LENGTH );
        }
        //scrub_recovery(&persistent_RAM_.Memory[0],PERSISTENT_RAM_LENGTH/8);
        nErrors += numberOfErrors((uint8_t*)&satelliteStatus_.totalMissionMinutes);
    }

     float percentage = 100*(float)nErrors/(iterations*PERSISTENT_RAM_LENGTH);
     //printf("Errori totali: %f \%\n",percentage);
	printf("%f\n",percentage);

   // int finalErrors = 0;
    //finalErrors = numberOfErrors((uint8_t*)&satelliteStatus_.totalMissionMinutes);
    return 0;
}
