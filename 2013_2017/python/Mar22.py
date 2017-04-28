#statements

#1. assignment
x,y,z=0,0,0
print(x,y,z)

values=[1,2,3]
x,y,z=values
print(x,y,z)

x={'name':'qq','age':100}
key,value=x.popitem()
print(key,value)

x=y=1000 # very good feature, finally
print(x,y)

x+=2
y*=2
print(x,y)

a='a'
b='b'
a+=b
a*=2
print(a)

print(True+False+11111111111)
print(bool('pengbo li'))

#2. conditional structure
name=input("type ur name")
print(name)
if name!='':
    print('Hello,', name)
    if 'i' in name:
        print('i in ur name')
    else:
        print('i not in ur name')
else:
    print('Name can not be empty') 

i=int(input('put a number'))
if i==0:
    print('0')
elif i>0:
    print('positive')
else:
    print('negative')

# >, <, !=, ==, is, is not,in, not in,

#difference between = and deepcopy
import copy
x=[1,2,3]
y=x;
z=copy.deepcopy(x)
print(x is y)
print(z is x )
    
if 1<2 and 2<3:
    print('everything is ok')

age=int(input('ur age'))
assert 0<age<200, '0-200, plz'

# 3. loops
x=1
while x<100:
    print(x)
    x=x+1

name = ''
while not name:
    name=input('Please enter your name: ')
print ('Hello, %s!' % name)

x=[0,0,1,1,4,5,6]
for i in x:
    print(i)

for i in range(0,10,2): # 10 will not be printed
    print(i)
    
d={'x':1,'y':2}
for key in d:
    print(key, 'is', d[key])

names = ['anne', 'beth', 'george', 'damon']
ages = [12, 45, 32, 102]
print(list(zip(names,ages)))
for name, age in zip(names, ages):
    print(name, 'is', age, 'years old')
    
