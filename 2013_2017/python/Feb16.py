# C1 statements, input,output,strings,numbers, basic functions
print(4/2,1/2,1/2.0,1.0/20)
# all factors are treated as float even though all variables are integer

print(1//2,1//4, 2//3,1.6//2)
# this is just get the quotient not rounding, but inegter gets integer, float get float

print(5%2,10%3,1.6%3)
# this is for remainder

import math
import cmath
print(math.ceil(1.1),math.floor(1.1),round(1.1),round(1.9),abs(-0.1),2**3,math.sqrt(2),cmath.sqrt(-1))

x=1000000000000000000000000 # no declare 
y=0.0
print(x, "is" ,type(x),"\n",y,"is",type(y))

# print(1/0) can not pass interperter
x=0xFF
y=0o10
z=0b10
print(x,y,z,2*x)

'''
x=input("x value:")
y=input("y value:")
z=input("z value:")

'''
print(x,y,z,type(x),type(y),type(z),pow(2,3), 2**3)
print((1-1j)*(1+1j)) # the imaginary part need write even it is 1
print("\'") # escape 
print("hello "+ "world!")
temp=20
print("the temperature is "+repr(temp))
print("hello \
world")
x=1+2+3+4\
+5
print(x)

#speical characters need care, raw string concept
print(r"c:\nowhere")
print("哈哈")
print(u"哈哈")
y=str(x)
print(type(x),type(y))