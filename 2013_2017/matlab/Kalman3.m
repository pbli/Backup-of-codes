close all
q=0.0001*0.09;
X1(1:3,1:645)=0;
X1(1,1)=theta(1);
P=eye(3,3)*0.10;
H=[1,0,0];
Q=[q,0,0;0,q,0;0, 0, q];
R=0.0001;
for i=2:length(theta)-1
    T=t(i)-t(i-1);
    F=[1 T 0.5*T*T; 0 1 T ;0 0 1];
    X1(:,i)=F*(X1(:,i-1));
    P=F*P*F'+Q;
    K=P*H'*(H*P*H'+R)^(-1);
    X1(:,i)=X1(:,i)+K*(theta(i)-H*X1(:,i));
    P=(eye(3,3)-K*H)*P;
end
figure
plot(theta,'b')
hold on
plot(X1(1,:),'r')
thetaDot=diff(X1(1,:))./diff(t');



q=0.0001*0.01;% how is the process, smaller, smoother
X1(1:3,1:644)=0;
X1(1,1)=thetaDot(1);
P=eye(3,3)*0.001;% the measurement influence, how serious
H=[1,0,0];
Q=[q,0,0;0,q,0;0, 0, q];
R=0.001; % how good the measurement, smaller, better
for i=2:length(thetaDot)
    T=t(i+1)-t(i);
    F=[1 T 0.5*T*T; 0 1 T ;0 0 1];
    X1(:,i)=F*(X1(:,i-1));
    P=F*P*F'+Q;
    K=P*H'*(H*P*H'+R)^(-1);
    X1(:,i)=X1(:,i)+K*(thetaDot(i)-H*X1(:,i));
    P=(eye(3,3)-K*H)*P;
end
figure
hold on
plot(thetaDot,'b')
plot(X1(1,1:644),'r')

rdot=diff(r)./diff(t);
q=0.0001*0.09;
X1(1:3,1:645)=0;
X1(1,1)=rdot(1);
P=eye(3,3)*0.10;
H=[1,0,0];
Q=[q,0,0;0,q,0;0, 0, q];
R=0.001;
for i=2:length(theta)-1
    T=t(i)-t(i-1);
    F=[1 T 0.5*T*T; 0 1 T ;0 0 1];
    X1(:,i)=F*(X1(:,i-1));
    P=F*P*F'+Q;
    K=P*H'*(H*P*H'+R)^(-1);
    X1(:,i)=X1(:,i)+K*(rdot(i)-H*X1(:,i));
    P=(eye(3,3)-K*H)*P;
end
figure
plot(rdot,'b')
hold on
plot(X1(1,:),'r')
