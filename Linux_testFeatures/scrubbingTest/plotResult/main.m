clear all
clc
close all



dataParziali = readtable('scrubbingTestCorruzioniParziali.csv');


indexParziali = dataParziali.Index;
percentageParziali = dataParziali.percentage;


len = size(dataParziali, 1);

p = polyfit(indexParziali,percentageParziali,4)
xForFit = linspace(1,200,200)
y1 = polyval(p,xForFit)


figure(2)
scatter(indexParziali,percentageParziali,'LineWidth',2);
hold on;
plot(xForFit,y1,'LineWidth',2,LineStyle='--');

xlabel('Numero di errori per iterazione')
ylabel('Rapporto percentuale di byte corrotti  e byte totali in memoria[%]')
legend('Data','Fitted curve')
grid on;

saveas(gcf,'scrubbingTestParziali.png')
