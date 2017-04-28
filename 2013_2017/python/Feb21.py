#c2 data structures
# list
edward=['edward gumby',42]
print(type(edward),type(edward[0]),type(edward[1]))
john=['John Smith' ,50]
customer=[edward,john]
print(customer)
print(customer[-1])
print(customer[0][0][0])# in matlab index is (), not [][], more like c
num=[1,2,3,4,5,6,7,8]
print(num[0:4],num[0:5],num[1:999],num[-3:],num[-1],num[-2:-1],num,num[:],num[0:8:2],num[::4],num[::-2],num[-1:-3:-1])
# a:b, indexed with b will not show
#:::, no comma
num2=[9,10]
num=num+num2
print(num,type(num),type(num[0]))
# list adding is not array adding
num=5*num
print(num)
num=[None]*10
print(num,type(num),num[0],type(num[0]))
sentence = "hello world!"
screen_width = 80
text_width = len(sentence)
box_width = text_width + 6
left_margin = (screen_width - box_width) // 2

print('\n')
print ( ' ' * left_margin + '+' + '-' * (box_width-2) + '+')
print (' ' * left_margin + '| ' + ' ' * text_width + ' |')
print (' ' * left_margin + '| ' + sentence + ' |')
print (' ' * left_margin + '| ' + ' ' * text_width + ' |')
print (' ' * left_margin + '+' + '-' * (box_width-2) + '+')

inornot=['A','B']
print('A' in inornot, "c" in inornot,'hello' in sentence)
database = [
['albert', '1234'],
['dilbert', '4242'],
['smith', '7524'],
['jones', '9843']
]
print(['pengbo','1234'] in database,len(database))

