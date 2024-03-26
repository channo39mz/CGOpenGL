l = []
def myfunc(b):
    if (b==0):
        return 1
    dig = b%2
    l.append(dig)
    myfunc(b//2)

myfunc(6)

for i in l:
    print(i, end='')