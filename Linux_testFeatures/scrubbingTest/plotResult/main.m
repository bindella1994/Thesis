clear all
clc
close all

dataTotali = readtable('scrubbingTestCorruzioniTotali.csv');


indexTotali = dataTotali.Index;
percentageTotali = dataTotali.percentage;


len = size(dataTotali, 1);

figure(1)
plot(indexTotali,percentageTotali,'LineWidth',2);

xlabel('Numero di errori per iterazione')
ylabel('Percentuale di blocchi di dati corrotti[%]')
legend('Rapporto percentuale tra byte corrotti e byte totali in memoria')
grid on;

saveas(gcf,'scrubbingTestTotali.png')

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

dataParziali = readtable('scrubbingTestCorruzioniParziali.csv');


indexParziali = dataParziali.Index;
percentageParziali = dataParziali.percentage;


len = size(dataParziali, 1);

figure(2)
plot(indexParziali,percentageParziali,'LineWidth',2);

xlabel('Numero di errori per iterazione')
ylabel('Percentuale di blocchi di dati corrotti[%]')
legend('Rapporto percentuale tra byte corrotti e byte totali in memoria')
grid on;

saveas(gcf,'scrubbingTestParziali.png')



p = polyfit(indexParziali,percentageParziali,5)
y1 = polyval(p,indexParziali);
figure(3)
plot(indexParziali,percentageParziali,'o')
hold on
plot(indexParziali,y1)
hold off
