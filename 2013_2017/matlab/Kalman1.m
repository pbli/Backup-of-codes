
clear
clc
close all
T=0.001;%fs
q=0.001*0.1;
n=100/T %t
for i=1:n
x(i)=100*i*T*i*T;
x(i)=100*(sin(T*2*pi*i/100));
end
y = randn(size(x));
x1=x+10*y;
X1(1:3,1:n)=0;
F=[1 T 0.5*T*T; 0 1 T ;0 0 1];
P=zeros(3,3);
H=[1,0,0];
Q=[q,0,0;0,q,0;0, 0, q];

R=var(y);
for i=2:length(x1)
    X1(:,i)=F*(X1(:,i-1));
    P=F*P*F'+Q;
    K=P*H'*(H*P*H'+R)^(-1);
    X1(:,i)=X1(:,i)+K*(x1(i)-H*X1(:,i));
    P=(eye(3,3)-K*H)*P;
end
plot(x1)
hold on
plot(x)
plot(X1(1,:))
figure
hold on
plot(x1-X1(1,:))
plot(x-X1(1,:))

    
    