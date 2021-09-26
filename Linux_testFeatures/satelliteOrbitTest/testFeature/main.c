#include <stdio.h>
#include "Acceleration.h"
#include <time.h>

int main()
{
    
    srand(time(NULL));
    unsigned short i = 0;
    int x,y,z;
    
    FILE *fs;
    fs = fopen("result.csv", "a");	//Creazione del file .csv in cui salvare i valori di accelerazione e 
					//relativo valore calcolato di accelerazione media
	if(fs == NULL){
	    printf("Couldn't open file\n");
	    return;
	}
	fprintf(fs, "X,Y,Z,mean\n");	//L'header del formato csv
    fclose(fs);
	
    for (i = 0; i < 3*ACCELERATION_BUFFER_SIZE ;++i){
	
    	x = 25*(rand()%10)-(2*rand()%2-1)*(rand()%10)-2*i;
    	y = -5*(rand()%10)+(2*rand()%2-1)*(rand()%15)+i;
    	z = 15*(rand()%3)+(2*rand()%2-1)*(rand()%5);
        float a = getAbsAcceleration(x,y,z);
        addAcceleration(&a);
    }    
	
	

    unsigned char satelliteStatus = isSatelliteInSpace();
    if(satelliteStatus!=0){
        printf("\nSoglia non superata\n");
    }else{
        printf("\nSoglia SUPERATA\n");
    }

    return 0;
}


