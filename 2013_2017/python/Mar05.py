# list cont.
print(list("hello"),''.join(list("hello")))
x=[1,2,3]
print(x)
x[0]=0
print(x)
del x[2]
print(x)
x=[1,2,3]
x[1:1]=[0, 0 ,0 ,0, 0]
print(x)
x[1:]=[0,0,0,0]
print(x)
x[1:]=[]
print(x)
x.append(1000)# return nothing, this is a function
print(x)
print(x.count(1000))# return the counted number
x.extend(x)
print(x)
x[len(x):]=x
print(x)
print(x.index(1))
x.insert(1,'insert')
print(x,type(x))
x[0:0]=["new insert"] #insert a list in
print(x,type(x[0]))
x[0:0]='new insert'#new insert is a long list
print(x,type(x[0]))
x=[1,2,3]
x.pop()
print(x)
x.pop(0)
print(x)
x='today'
x=list(x)
print(x)
x.remove('t')
print(x.reverse())# x will be reverse, but no return
print(x)
x=[0,67,4,5,6,3,4,231]
y=x
z=x[:]
w=sorted(x)
x.sort()
print(x,y,z,w)#y also sorted, this is different with other software
x=['fdf','ffff','fd']
x.sort(key=len)
print(x)
x=(1,2,3)#use () for tuples
print(type(x))