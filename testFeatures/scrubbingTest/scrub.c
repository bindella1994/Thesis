/*
 * scrub.c
 *
 *  Created on: 22 set 2021
 *      Author: laboratorio
 */



/*
 *
 * Funzione di calcolo della parit� Verticale, genera un byte a partire da un blocco di otto byte
 *
 */


#include "scrub.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

struct persistent_RAM persistent_RAM_={0};
struct persistent_RAM persistent_RAM_Copy_={0};
struct parity parity_part={0};
struct SatelliteStatus satelliteStatus_={0};

int errori,correzioni = 0;


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

    hn = count_errors(parity_part.h2[index] ^ parity_part.h1[index]);
    vn = count_errors(parity_part.v2[index] ^ parity_part.v1[index]);
    dn = count_errors(parity_part.d2[index] ^ parity_part.d1[index]);
    find_position(parity_part.h2[index] ^ parity_part.h1[index], h);
    find_position(parity_part.v2[index] ^ parity_part.v1[index], v);
    find_position(parity_part.d2[index] ^ parity_part.d1[index], d);

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

void find_position(int16_t a, int16_t *ptr)
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

        parity_part.h1[index]=Calc_HParity(Data);
        parity_part.v1[index]=Calc_VParity(Data);
        parity_part.d1[index]=Calc_DParity(Data);

    }
    else if (stage == 1)
    {
        parity_part.h2[index]=Calc_HParity(Data);
        parity_part.v2[index]=Calc_VParity(Data);
        parity_part.d2[index]=Calc_DParity(Data);
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
void printPersistentRam(){
    int i  = 0;
    for ( i = 0 ; i<PERSISTENT_RAM_LENGTH; ++i){
        printf("Elemento: %x\n",persistent_RAM_.Memory[i]);
    }

}

void printParity(){
    uint32_t l;

    for (l = 0; l < PERSISTENT_RAM_LENGTH/8; l++)
    {
        printf("h1:%x\tv1:%x\td1:%x_____",parity_part.h1[l],
                parity_part.v1[l],parity_part.d1[l]);
        printf("h2:%x\tv2:%x\td2:%x\n",parity_part.h2[l],
                parity_part.v2[l],parity_part.d2[l]);
    }
    printf("___________________________________\n");



}

void insertSingleCorruption(unsigned char* startingPointer, unsigned int length, unsigned char log ){
    unsigned int arrayIndex = rand() % length;
    unsigned int byteIndex = rand() % 8;
	    if(log!=0){
	    printf("Location %d before the corruption was: 0x%x\n",arrayIndex,startingPointer[arrayIndex]);
	    }
    startingPointer[arrayIndex] = startingPointer[arrayIndex] ^ ( 1 << byteIndex );
	    if(log!=0){
	    printf("Location %d after the corruption was: 0x%x\n",arrayIndex,startingPointer[arrayIndex]);
	    }
}

int numberOfErrors(uint8_t* startingPointer){
    int result = 0;
    int i;
    for (i=0;i<PERSISTENT_RAM_LENGTH;++i){
        if(startingPointer[i]!=persistent_RAM_Copy_.Memory[i]){
            ++result;
        }
    }
    return result;
}

void initSatelliteStatus(uint8_t* startingPointer, uint32_t length){
    uint32_t i;
    for ( i = 0; i<length;++i ){
        startingPointer[i]=(uint8_t)rand();
    }
}

void copyPersistentRamForFinalTest(uint8_t* startingPointer,uint8_t* copyStartingPointer, uint32_t length){
    uint32_t i;
    for ( i = 0; i<length;++i ){
        copyStartingPointer[i]=startingPointer[i];
    }
}
