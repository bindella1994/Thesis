#include "PersistentRam.h"
#include <time.h>
#include <stdlib.h>
//struct RamBackup ramBackup_;

int16_t Calc_VParity(int16_t Data[])
{
    int16_t parity = 0;
    int16_t i;
    for(i=0; i<8; i++)
    {
        parity = parity ^ Data[i];
    }
    return parity;
}

int16_t Calc_HParity(int16_t Data[])
{
    int16_t parity = 0;
    int16_t temp;
    int16_t i,j, index;
    for(i=0; i<8; i++)
    {
        index = 0x80;
        temp = 0;
        for(j=0; j<8; j++)
        {
            if(Data[i] & index)
            {
                temp = temp ^ 0x0100;
            }
            index = index >> 1;
        }
        parity = parity ^ temp;
        parity = parity >> 1;
    }
    return parity;
}

int16_t Calc_DParity(int16_t Data[])
{
    int16_t parity = 0;
    int16_t dparity = 0;
    int16_t temp;
    int16_t i,j, index;
    for(i=0; i<8; i++)
    {
        index = 0x01;
        temp = 0;
        for(j=i; j>=0; j--)
        {
            if(Data[j] & index)
            {
                temp = temp ^ 0x0100;
            }
            index = index << 1;
        }
        parity = parity ^ temp;
        parity = parity >> 1;
    }

    for(i=1; i<=7; i++)
    {
        index = 0x80;
        temp = 0;
        for(j=i; j<=7; j++)
        {
            if(Data[j] & index)
            {
                temp = temp ^ 0x80;
            }
            index = index >> 1;
        }
        dparity = dparity ^ temp;
        dparity = dparity >> 1;
    }
    dparity = dparity << 8;
    dparity = dparity ^ parity;

    return dparity;
}

int8_t scrubbing_bytes(int16_t Data[8],int8_t index)
{
    int16_t h[8],v[8],d[16];
    int8_t dn, hn, vn;

    hn = count_errors(parity_.h2[index] ^ parity_.h1[index]);
    vn = count_errors(parity_.v2[index] ^ parity_.v1[index]);
    dn = count_errors(parity_.d2[index] ^ parity_.d1[index]);
    find_position(parity_.h2[index] ^ parity_.h1[index], h);
    find_position(parity_.v2[index] ^ parity_.v1[index], v);
    find_position(parity_.d2[index] ^ parity_.d1[index], d);

    if(hn==0 && vn==0 && dn==0)
    {
        return 0;
    }
    if((hn==1 && vn==1 && dn==1) && (d[0]==h[0]+v[0]))
    {
        toggle_bit(Data,8,h[0],v[0]);
    }
    if((hn==1 && vn==1 && dn==1) && (d[0]!=h[0]+v[0]))
    {
        if(d[0]<(h[0]+v[0]))
        {
            toggle_bit(Data,8,h[0],v[0]-1);
            toggle_bit(Data,8,h[0]-1,v[0]);
            toggle_bit(Data,8,h[0]-1,v[0]-1);
        }
        else
        {
            toggle_bit(Data,8,h[0],v[0]+1);
            toggle_bit(Data,8,h[0]+1,v[0]);
            toggle_bit(Data,8,h[0]+1,v[0]+1);
        }
    }
    if((hn==1 && vn==1 && dn==3) && (d[1]==h[0]+v[0]))
    {
    }
    if(hn==2 && vn==2 && dn==2)
    {
        if(d[0]==h[0]+v[0])
        {
            toggle_bit(Data,8,h[0],v[0]);
            toggle_bit(Data,8,h[1],v[1]);
        }
        else
        {
            toggle_bit(Data,8,h[1],v[0]);
            toggle_bit(Data,8,h[0],v[1]);
        }
    }
    if(hn==0 && vn==2 && dn==2)
    {
        toggle_bit(Data,8,(d[0]-v[0]),v[0]);
        toggle_bit(Data,8,(d[1]-v[1]),v[1]);
    }
    if(hn==2 && vn==0 && dn==2)
    {
        toggle_bit(Data,8,h[0],(d[0]-h[0]));
        toggle_bit(Data,8,h[0],(d[1]-h[1]));
    }
    if(hn==2 && vn==2 && dn==0)
    {
        toggle_bit(Data,8,h[0],v[1]);
        toggle_bit(Data,8,h[1],v[0]);
    }
    if(hn==1 && vn==3 && dn==3)
    {
        toggle_bit(Data,8,h[0],v[0]);
        toggle_bit(Data,8,h[0],v[1]);
        toggle_bit(Data,8,h[0],v[2]);
    }
    if(hn==3 && vn==1 && dn==3)
    {
        toggle_bit(Data,8,h[0],v[0]);
        toggle_bit(Data,8,h[1],v[0]);
        toggle_bit(Data,8,h[2],v[0]);
    }
    if(hn==3 && vn==3 && dn==1)
    {
        toggle_bit(Data,8,h[2],v[0]);
        toggle_bit(Data,8,h[1],v[1]);
        toggle_bit(Data,8,h[0],v[2]);
    }
    if(hn==3&&vn==3&&dn==3&&(d[0]==h[0]+v[0])&&(d[1]==h[1]+v[1])&&(d[2]==h[2]+v[2]))
    {
        toggle_bit(Data,8,h[0],v[0]);
        toggle_bit(Data,8,h[1],v[1]);
        toggle_bit(Data,8,h[2],v[2]);
    }

    return 1;
}

int16_t find_position(int16_t a, int16_t *ptr)
{
    int16_t i;
    int16_t index = 1;
    for(i=0; i<16; i++)
    {
        if(a & index)
        {
            *ptr=i;
            ptr++;
        }
        index = index <<1;
    }
}
void initialize(int16_t *pt, int16_t size)
{
    int16_t i;
    for(i=0; i<size; i++)
    {
        *pt=-1;
        pt++;
    }
}

int16_t count_errors(int16_t a)
{
    int16_t i;
    int16_t index = 1;
    int16_t count = 0;

    for(i=0; i<16; i++)
    {
        if(a & index)
        {
            count=count+1;
        }
        index = index <<1;
    }
    return count;
}

void toggle_bit(int16_t *pt,int16_t len,int16_t i,int16_t j)
{
    if(i<len)
    {
        pt[i] ^= 1UL << (j);
    }
}

void scrubbing_parity_generation(int16_t Data[8],int8_t stage,int8_t index )
{
    int16_t h[8],v[8],d[16];

    initialize(h,8);
    initialize(v,8);
    initialize(d,16);
    if(stage == 0)
    {

        parity_.h1[index]=Calc_HParity(Data);
        parity_.v1[index]=Calc_VParity(Data);
        parity_.d1[index]=Calc_DParity(Data);

    }
    else if (stage == 1)
    {
        parity_.h2[index]=Calc_HParity(Data);
        parity_.v2[index]=Calc_VParity(Data);
        parity_.d2[index]=Calc_DParity(Data);
    }
    else
    {

    }
}

void setScrubParity( uint8_t* index_pointer, uint8_t index_struct )
{
    uint8_t i,j;
    uint8_t *pointer = (uint8_t*)index_pointer;
    for ( i = 0; i<index_struct ; ++i){
        int16_t Data_memory_control[8]={0};
        for ( j=0; j<8; j++ )
        {
            Data_memory_control[j]=(int16_t)*pointer;
            pointer++;
        }
        scrubbing_parity_generation(Data_memory_control,0,i);
    }
}

int8_t scrub_recovery( uint8_t* index_pointer, uint8_t index_struct )
{
    uint8_t i,j;
    uint8_t return_value=0;
    uint8_t *pointer;
    int16_t Data_memory_control[8];
    pointer=(uint8_t*)(index_pointer);
    for ( i = 0; i<index_struct ; ++i){
        for ( j=0; j<8; j++ )
        {
            Data_memory_control[j]=pointer[8*i+j];
        }
        scrubbing_parity_generation(Data_memory_control,1,i);
        int result = scrubbing_bytes(Data_memory_control,i);
        if(result==1)
        {

            for ( j=0; j<8; j++ )
            {

                pointer[j+8*i]=Data_memory_control[j];
            }
            //++correzioni;
            return_value+=1;
        }
        else if(result==2){
            //++errori;
        }
        else if(result==0){
            //printf("Tutto bene\n");
        }
    }

    //printf("Correzioni: %d",correzioni);
    //printf("Errori:     %d",errori);
    return return_value;
}

void backupInPersistentRam (uint8_t* persistentStartingPointer,
        uint8_t* memoryStartingPointer,
        uint32_t length){
    int i  = 0;
    for (  i = 0 ; i<length; ++i){
        persistentStartingPointer[i]=memoryStartingPointer[i];
    }
}


void restoreFromPersistentRam (uint8_t* persistentStartingPointer,
        uint8_t* memoryStartingPointer,
        uint32_t length){
    int i = 0;
    for (  i = 0 ; i<length; ++i){
        memoryStartingPointer[i]=persistentStartingPointer[i];
    }
}

void insertSingleCorruption(unsigned char* startingPointer, unsigned int length){
    srand(time(NULL));
    unsigned int arrayIndex = rand() % length;
    unsigned int byteIndex = rand() % 8;

    startingPointer[arrayIndex] = startingPointer[arrayIndex] ^ ( 1 << byteIndex );

}


uint8_t cleanPersistentRam(uint16_t *byteElements, uint32_t size){
    int i;
    for ( i = 0;i< PERSISTENT_RAM_LENGTH; ++i) {
        ramBackup_.persistentRam[i] = 0;
    }
    ramBackup_.isPersistentRamWritten = 0;
    int j;
    for ( j = 0 ; j < PERSISTENT_RAM_LENGTH / 8 ; ++j){
        parity_.h1[j] = 0;
        parity_.v1[j] = 0;
        parity_.d1[j] = 0;
        parity_.h2[j] = 0;
        parity_.v2[j] = 0;
        parity_.d2[j] = 0;
    }
    return 0;
}

/*
void FourDimensionalParityCodeBackup(int16_t *startingPointer, int16_t size)
{
    //int a, b;
    int16_t Data[8];
    int16_t h[8], v[8], d[16];
    //ParityElement parityElement1, parityElement2, parityElementN;
    int16_t i, j;//, index;
    for (i = 0; i < size; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            Data[j] = *startingPointer;
            startingPointer++;
        }
        initialize(h, 8);
        initialize(v, 8);
        initialize(d, 16);
        ramBackup_.fourDimensionalParityPart[i].hParity = Calc_HParity(Data);
        ramBackup_.fourDimensionalParityPart[i].vParity = Calc_VParity(Data);
        ramBackup_.fourDimensionalParityPart[i].dParity = Calc_DParity(Data);
    }
}
void FourDimensionalParityCodeRestore(int16_t *startingPointer, int16_t size)
{
    int countErrors = 0;
    //int a, b;
    int16_t Data[8];
    int16_t h[8], v[8], d[16];
    ParityElement parityElement1, parityElement2, parityElementN;
    int16_t i, j; //, index;

    for (i = 0; i < size; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            Data[j] = startingPointer[8 * i + j];
        }
        initialize(h, 8);
        initialize(v, 8);
        initialize(d, 16);
        //A priori
        parityElement1.hParity = ramBackup_.fourDimensionalParityPart[i].hParity;
        parityElement1.vParity = ramBackup_.fourDimensionalParityPart[i].vParity;
        parityElement1.dParity = ramBackup_.fourDimensionalParityPart[i].dParity;
        //A posteriori
        parityElement2.hParity = Calc_HParity(Data);
        //parityElement2.vParity = Calc_VParity(Data);
        parityElement2.dParity = Calc_DParity(Data);
        //Errors Evaluation
        parityElementN.hParity = count_errors(parityElement2.hParity ^ parityElement1.hParity);
        parityElementN.vParity = count_errors(parityElement2.vParity ^ parityElement1.vParity);
        parityElementN.dParity = count_errors(parityElement2.dParity ^ parityElement1.dParity);

        countErrors += count_errors(parityElement2.hParity ^ parityElement1.hParity);
        countErrors += count_errors(parityElement2.vParity ^ parityElement1.vParity);
        countErrors += count_errors(parityElement2.dParity ^ parityElement1.dParity);
        //Find Position Of Errors
        find_position(parityElement2.hParity ^ parityElement1.hParity, h);
        find_position(parityElement2.vParity ^ parityElement1.vParity, v);
        find_position(parityElement2.dParity ^ parityElement1.dParity, d);

        if (parityElementN.hParity == 0 && parityElementN.vParity == 0 && parityElementN.dParity == 0)
        {
            //printf("\nThere is no error.\n\n");
        }
        if ((parityElementN.hParity == 1 && parityElementN.vParity == 1 && parityElementN.dParity == 1) && (d[0] == h[0] + v[0]))
        {
            //printf("\nSingle bit error\n\n");
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0], v[0]);
        }
        if ((parityElementN.hParity == 1 && parityElementN.vParity == 1 && parityElementN.dParity == 1) && (d[0] != h[0] + v[0]))
        {
            //printf("\nTriple bit error, correctable\n");
            if (d[0] < (h[0] + v[0]))
            {
                //printf("Error bit locations:");
                //printf("(%d,%d),(%d,%d),(%d,%d)", h[0], v[0] - 1, h[0] - 1, v[0], h[0] - 1, v[0] - 1);
                //printf("\n");
                toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0], v[0] - 1);
                toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0] - 1, v[0]);
                toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0] - 1, v[0] - 1);
            }
            else
            {
                //printf("Error bit locations:");
                //printf("(%d,%d),(%d,%d),(%d,%d)", h[0], v[0] + 1, h[0] + 1, v[0], h[0] + 1, v[0] + 1);
                //printf("\n");
                toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0], v[0] + 1);
                toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0] + 1, v[0]);
                toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0] + 1, v[0] + 1);
            }
        }
        if ((parityElementN.hParity == 1 && parityElementN.vParity == 1 && parityElementN.dParity == 3) && (d[1] == h[0] + v[0]))
        {
            //printf("\nInnocent bit error,\n\n");
        }
        if (parityElementN.hParity == 2 && parityElementN.vParity == 2 && parityElementN.dParity == 2)
        {
            //printf("\nDouble-bit scattered error\n");
            if (d[0] == h[0] + v[0])
            {
                //printf("Error bit locations:");
                //printf("(%d,%d) and (%d,%d).\n", h[0], v[0], h[1], v[1]);
                toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0], v[0]);
                toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[1], v[1]);
            }
            else
            {
                //printf("Error bit locations:");
                //printf("(%d,%d) and (%d,%d).\n", h[1], v[0], h[0], v[1]);
                toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0], v[0]);
                toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[1], v[1]);
            }
        }
        if (parityElementN.hParity == 0 && parityElementN.vParity == 2 && parityElementN.dParity == 2)
        {
            //printf("\nDouble-bit error in same row\n");
            //printf("Error bit locations:");
            //printf("(%d,%d) and (%d,%d).\n", (d[0] - v[0]), v[0], (d[1] - v[1]), v[1]);
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, (d[0] - v[0]), v[0]);
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, (d[1] - v[1]), v[1]);
        }
        if (parityElementN.hParity == 2 && parityElementN.vParity == 0 && parityElementN.dParity == 2)
        {
            //printf("\nDouble-bit error in same column\n");
            //printf("Error bit locations:");
            //printf("(%d,%d) and (%d,%d).\n", h[0], (d[0] - h[0]), h[0], (d[1] - h[1]));
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0], (d[0] - h[0]));
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0], (d[1] - h[1]));
        }
        if (parityElementN.hParity == 2 && parityElementN.vParity == 2 && parityElementN.dParity == 0)
        {
            //printf("\nDouble-bit error in same diagonal\n");
            //printf("Error bit locations:");
            //printf("(%d,%d) and (%d,%d).\n", h[0], v[1], h[1], v[0]);
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0], v[1]);
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[1], v[0]);
        }
        if (parityElementN.hParity == 1 && parityElementN.vParity == 3 && parityElementN.dParity == 3)
        {
            //printf("\nTriple-bit error in same row\n");
            //printf("Error bit locations:");
            //printf("(%d,%d), (%d,%d) and (%d,%d).\n", h[0], v[0], h[0], v[1], h[0], v[2]);
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0], v[0]);
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0], v[1]);
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0], v[2]);
        }
        if (parityElementN.hParity == 3 && parityElementN.vParity == 1 && parityElementN.dParity == 3)
        {
            //printf("\nTriple-bit error in same column\n");
            //printf("Error bit locations:");
            //printf("(%d,%d), (%d,%d) and (%d,%d).\n", h[0], v[0], h[1], v[0], h[2], v[0]);
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0], v[0]);
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[1], v[0]);
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[2], v[0]);
        }
        if (parityElementN.hParity == 3 && parityElementN.vParity == 3 && parityElementN.dParity == 1)
        {
            //printf("\nTriple-bit error in same diagonal\n");
            //printf("Error bit locations:");
            //printf("(%d,%d), (%d,%d) and (%d,%d).\n", h[2], v[0], h[1], v[1], h[0], v[2]);
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[2], v[0]);
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[1], v[1]);
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0], v[2]);
        }
        if (parityElementN.hParity == 3 && parityElementN.vParity == 3 && parityElementN.dParity == 3 && (d[0] == h[0] + v[0]) && (d[1] == h[1] + v[1]) && (d[2] == h[2] + v[2]))
        {
            //printf("\nTriple-bit scattered error\n");
            // Other types are not considered to avoid complexity
            //printf("Error bit locations:");
            //printf("(%d,%d), (%d,%d) and (%d,%d).\n", h[0], v[0], h[1], v[1], h[2], v[2]);
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[0], v[0]);
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[1], v[1]);
            toggleBit(&startingPointer[8 * i], PERSISTENT_RAM_BYTE_SIZE, h[2], v[2]);
        }
        if (parityElementN.dParity > 3 || parityElementN.dParity < 0)
        {
            //printf("\nThe error can only be detected.\n");
        }
    }
    //printf("Il numero di errori rimasti è: %d\n", countErrors);
}

uint8_t cleanPersistentRam(uint16_t *byteElements, uint32_t size)
{
    uint8_t result = 0x00;
    int16_t  i;
    for ( i = 0; i < size; ++i)
    {
        byteElements[i] = 0x0000;
    }

    result = 0xff;
    ramBackup_.isPersistentRamWritten = 0x00;
    return result;
}

uint8_t backupDataInPersistentRam(uint16_t *byteElements, uint8_t *startingPointer, uint32_t size)
{
    uint8_t result = 0x00;
    int16_t  i;
    for ( i = 0; i < size; ++i)
    {
        byteElements[i] = (uint16_t)startingPointer[i];
    }
    ramBackup_.isPersistentRamWritten =  0xff;
    result = 0xff;
    return result;
}

uint8_t restoreDataFromPersistentRam(uint16_t *byteElements, uint8_t *startingPointer, uint32_t size)
{
    uint8_t result = 0x00;
    int16_t  i;
    for (i = 0; i < size; ++i)
    {
        startingPointer[i] = (uint8_t)byteElements[i];
    }
    result =  0xff;
    return result;
}

uint8_t isPersistentRamUncorrupted(int16_t *startingPointer, int16_t size)
{
    uint8_t result = 0x0;
    int16_t countErrors = 0;
    //int16_t a;
    int16_t Data[8];
    int16_t h[8], v[8], d[16];
    ParityElement parityElement1, parityElement2; //parityElementN
    int16_t i, j;// , index;

    for (i = 0; i < size; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            Data[j] = startingPointer[8 * i + j];
        }
        initialize(h, 8);
        initialize(v, 8);
        initialize(d, 16);
        //A priori
        parityElement1.hParity = ramBackup_.fourDimensionalParityPart[i].hParity;
        parityElement1.vParity = ramBackup_.fourDimensionalParityPart[i].vParity;
        parityElement1.dParity = ramBackup_.fourDimensionalParityPart[i].dParity;
        //A posteriori
        parityElement2.hParity = Calc_HParity(Data);
        parityElement2.vParity = Calc_VParity(Data);
        parityElement2.dParity = Calc_DParity(Data);
        //Errors Evaluation
        countErrors += count_errors(parityElement2.hParity ^ parityElement1.hParity);
        countErrors += count_errors(parityElement2.vParity ^ parityElement1.vParity);
        countErrors += count_errors(parityElement2.dParity ^ parityElement1.dParity);
    }
    //printf("Il numero di errori rimasti è: %d\n", countErrors);
    if (countErrors > 0)
    {
        result = 0xff;
    }
    return result;
}

uint8_t isScrubbingPossible(int16_t *startingPointer, int16_t size)
{
    return ((ramBackup_.isPersistentRamWritten!=0) && isPersistentRamUncorrupted((int16_t *)startingPointer, size));
}
*/
