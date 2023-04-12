apps = ['bfs', 'tc', 'pr']

import os

dire = os.getcwd()

for app in apps:
    tmp = os.path.join(dire, app+"-dir")
    
    if not os.path.isdir(tmp):
        os.mkdir(tmp)

    st =  'make TARGET={0}'.format(app)
    os.system(st)
    
    st = "cp cust* {0}-dir".format(app)
    os.system(st)
    
    st = "cp sim.* {0}-dir".format(app)
    os.system(st)
