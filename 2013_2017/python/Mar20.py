#strings

s1="hello"
#does not support assignment like s1[5]='9', in other laguage, it works.

formatt="hi, %s, %s"
values=('pengbo','li')
print(formatt%values)

formatt1="hi, pi is %.10f"
import numpy as np
print(formatt1%np.pi)

from string import Template
s=Template('hi, $x!')
print(s.substitute(x='pengbo'))

ss=Template('hi,${x}li!,$$') #there characters after x, so use braces, double $$ for $
print(ss.substitute(x='pengbo'))

sss=Template('A $thing can $action')
d={}
#dictionary{},tuple(),list [], use [] to index
d['thing']='dog'
d['action']='bark'
print(sss.substitute(d))

print('%d plus %d equals %d' %(1,2,3)) #formating is similar to that in printf, scanf
# algorithm is the most important, not the laguage 
print('%10.2f'% np.pi)
print('%-10.2f'% np.pi) #length of the content
print( '% 5d' % 10 + '\n' + '% 5d' % -10)
print( '% +5d' % 10 + '\n' + '% -+5d' % -10)# use + to add sign, use - to left align
numb=['1','2','3','4']
pluss='+'
print(pluss.join(numb))
print('c:'+'\\'.join(('usr','bin','env')))
#other fuctions:title, replace, lower, split,strip,traslate

