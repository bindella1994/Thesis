
#include <iostream>
#include <boost/circular_buffer.hpp>
#define MAX_SIZE 32
#include <math.h>
#include <vector>
#include <cstddef>
#include <sstream>
#include <vector>
#include <fstream>
#include <string>
std::vector<double> myVector_;

using namespace std;

struct AccelerationSample
{
    public:
    float x,y,z;  
};

vector<AccelerationSample> readCSV(std::string inputFile)
{   
    ifstream csvFile;
    string strPathCSVFile = inputFile;
    csvFile.open(strPathCSVFile);

    if (!csvFile.is_open())
    {
        cout << "Path Wrong!!!!" << endl;
        exit(EXIT_FAILURE);
    }

     vector<AccelerationSample> samples;
    //vector< double> acc_X;
    //vector< double> acc_Y;
    //vector< double> acc_Z;
    //vector<double> mean;

    string line;
    vector <string> vec;
    getline(csvFile, line); // skip the 1st line

    while (getline(csvFile,line))
    {
        if (line.empty()) // skip empty lines:
        {
            //cout << "empty line!" << endl;
            continue;
        }

        istringstream iss(line);
        string lineStream;
        string::size_type sz;

        vector <double> row;

        while (getline(iss, lineStream, ','))
        {  
            row.push_back(stold(lineStream,&sz)); // convert to double
        }
	AccelerationSample tempSample;
	tempSample.x = row[0];
	tempSample.y = row[1];
	tempSample.z = row[2];
	samples.push_back(tempSample);
	/*
        acc_X.push_back(row[0]);
        acc_Y.push_back(row[1]);
        acc_Z.push_back(row[2]);
        mean.push_back(row[3]);
        */
    }

    //cout << "size ts = " << timeStampIMU.size() << endl;
    /*for (size_t i = 0; i < samples.size(); i++)
    {
    	AccelerationSample  mySample = samples.at(i);
        cout << "ax = "  << mySample.x << endl;
        cout << "ay = "  << mySample.y << endl;
        cout << "az = "  << mySample.z << endl;
        cout << "--------------------------------" << endl;
    }*/
    return samples;
}

vector <AccelerationSample> samples_;
int main(){
    samples_ = readCSV("result.csv");  
    
    ::boost::circular_buffer<float> myCircularBuffer_;
    myCircularBuffer_.resize(MAX_SIZE);
    
    
    
    
    
    
    
    std::cout<<"Inizio\n";
    FILE* fw;
    fw=fopen("resultCpp.csv","w");
    fprintf(fw,"Index,meanAcceleration\n");
    int index = 0;
    for (auto sample : samples_) {
        
        float absAcceleration = std::sqrt(sample.x*sample.x + sample.y*sample.y + sample.z*sample.z);
        myCircularBuffer_.push_back(absAcceleration); 

        
        int sizeCounter = 0;
        float meanAcceleration = 0;
         
         
        
        for (auto itr = myCircularBuffer_.begin(); itr!=myCircularBuffer_.end() ; ++itr){
              meanAcceleration+=*itr;
              //std::cout<<*itr<<",";
            if(*itr!=0){
            sizeCounter++;
            }
        }
        if(sizeCounter!=0){
            meanAcceleration = meanAcceleration/sizeCounter;
        }
        //std::cout<<"\tCounter: "<<counter;
        //std::cout<<"\n";
        
        //std::cout<<"Media: "<<buf<<"\n";
        fprintf(fw,"%d,%f\n",index,meanAcceleration);
        ++index;
    }
    fprintf(fw,"\n");
    fclose(fw);
    
    std::cout<<"Fine\n";
    
    
    return 0;
}
