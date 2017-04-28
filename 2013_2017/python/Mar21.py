#dictionary
# list refers to values with numbers, dictionary refers to values with names.
dd = {'Alice': '2341', 'Beth': '0102', 'Cecil': '3258'}
print(dd['Alice'],dd['Beth'])
items = [('name', 'Gumby'), ('age', 42)]
d=dict(items)
d['age']=43
print(d,len(d),d['name'])
x={}
x[40]=100
print(x[40])
d={
   'p1':{'name':'pengboli','age':29},
   'p2':{'name':'pengweili','age':23}
   }
print(d['p2']['name'])    
print("Cecil's phone number is %(Cecil)s"%dd)
template = '''<html>
<head><title>%(title)s</title></head>
<body>
<h1>%(title)s</h1>
<p>%(text)s</p>
</body>'''
data = {'title': 'My Home Page', 'text': 'Welcome to my home page!'}
print( template % data)

x={1:2}
z={3:4}
y=x
w=z;
y={}
#y[1]=3 will change x, while y={} will not change x, y= is a reference. if 
# assigenment, it change the value. y={} just change the refred location to a empty dictionary 
w.clear()
print(x,y,z,w)
x=100
y=x
y=0
print(x,y)
x=[1,2,3]
y=x
y[0]=0
print(x,y)
y.clear()  # this is a very strange feature, need care
print(x,y)
# use deepcopy
import copy
x=[1,2,3]
y=copy.deepcopy(x)
print(x,y)
x=[0,2]
y[0]=100
print(x,y)

x={1:2,3:4}
y=copy.deepcopy(x)
print(x,y)
y[1]=100
print(x,y)

x={}.fromkeys([1,2]) #list works
y={}.fromkeys((1,2)) #tuple works
x[1]=10000
print(x,y)
print(x.get(1),x.get(1000000,'not available'))
x={1:2,3:4}
print(x.items())
print(x.keys())
x.pop(1)
print(x)
x.popitem()
print(x)
x.setdefault(1,3)# after change once, it will not perform setdefault.
print(x)
x[1]=10
x.setdefault(1,3)
print(x)
x={1:2,3:4}
y={1:1000}
x.update(y)
print(x)
print(x.values())