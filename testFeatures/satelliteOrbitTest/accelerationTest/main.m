clear all
clc
close all

data = readtable('result.csv');

%theoricalValues=[7 7 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 5 5 5 5 5 5 5 5 5 5 5 5 5 4 4 4 4 4 4 4 4 4 4 4 4 3 3 3 3 3 3 3 3 3 3 3 3 3 3 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1];
cppData = readtable('resultCpp.csv');
theoricalValues = cppData.meanAcceleration;

soglia =9.81/2;
len = size(data, 1);


mean = data.mean;
time = zeros(1,len);
for i = 1:len
    time(1,i)=1e-3 * i;
end
ax = data.X;
ay = data.Y;
az = data.Z;




plot(time,ax,'-.','LineWidth',1.5);
hold on;
plot(time,ay,'-.','LineWidth',1.5);
hold on;
plot(time,az,'-.','LineWidth',1.5);
hold on;
plot(time,mean,'LineWidth',2);
hold on;
plot(time,soglia*ones(1,len),'LineWidth',5);


xlabel('time[sec]')
ylabel('Acceleration [m][sec^{-2}]')
legend('X Acceleration','Y Acceleration','Z Acceleration','Mean Acceleration','Acceleration Threshold')
grid on;
saveas(gcf,'Acceleration.png')
hold off;


figure(2)
plot(time,mean,'LineWidth',2);
hold on;
plot(time,theoricalValues','.-');

xlabel('time[sec]')
ylabel('Acceleration [m][sec^{-2}]')
legend('Algorithm Result','Theorical Result')
grid on;

saveas(gcf,'CompareAlgorithResult.png')
percentageError = 0;
index = 0;
for i = 1:len
    if(theoricalValues (i) ~= mean(i) )
        percentageError = percentageError + 1;
    end
    
    index = index + 1;
end
percentageError = ( percentageError / len )* 100

