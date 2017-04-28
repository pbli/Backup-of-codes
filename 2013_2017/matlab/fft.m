clc
N=length(x1);
x1=x1-mean(x1);
figure
subplot(2,1,1);
plot(x1),xlabel('sampling sequence'),ylabel('amplitude'),title('initial signal');
x=x1;
sampleLength=length(x);
sampleFreq=fs;
Data=x;
fft_result = abs(fft(Data-mean(Data))) * 2 / sampleLength;
%coordinates transfer for plotting
% time_plot_s = 0:1/sampleFreq:(sampleLength-1) / sampleFreq;   %time axis
fft_plot_Hz = (sampleFreq/sampleLength)*(1:sampleLength/2);
%frequency axis, sampleFreq/sampleLength
yy=fft_result(1:sampleLength/2);
subplot(2,1,2);
plot(fft_plot_Hz,yy);
title('frequency domain');
ylabel('amplitude');
xlabel('f/Hz');