from math import sqrt
for i in range(99,0,-1):
    x=sqrt(i)
    if x==int(x):
        print(i)
        break
    
for i in range(10000,0,-1):
    x=sqrt(i)
    if x==int(x):
        print(i)
        continue

i=0
while i<10:
    print(i)
    i+=1

print([x*x for x in range(10)])

girls = ['alice', 'bernice', 'clarice']
boys = ['chris', 'arnold', 'bob']
print([b+'+'+g for b in boys for g in girls if b[0] == g[0]]) #not very good and clear.
i=input('a number')
if int(i)<10:
    print(i)
else:
    pass    

exec( "print( 'Hello, world!')") #useful for some programming
x=10
print(eval('x*x'))


def fib(n):
    'get fib numbers'
    fibs = [0, 1]
    for i in range(n):
        fibs.append(fibs[-2] + fibs[-1])
    print(fibs)
fib(8)
print(fib.__doc__)