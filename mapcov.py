import os
import sys
from collections import defaultdict

if __name__ == "__main__":
    file = open("Big5-ZhuYin.map", "r", encoding="big5hkscs")
    data = file.read()
    mapp = defaultdict(list)
    fileout = open("Zhuyin-Big5.map", "wb")

    for i in data.split("\n")[0:-1]:
        ch = i.split(" ")[0]
        so = (i.split(" ")[1]).split('/') 
        fileout.write((ch+" "+ch+"\n").encode('big5hkscs'))
        start_list = []
        for ite in so:
            if ite[0] not in start_list:
                start_list.append(ite[0])
        for symbol in start_list:
            mapp[symbol].append(ch)
    ans = 0
    for i, j in mapp.items():
        fileout.write(i.encode('big5hkscs'))
        for ite in j:
            fileout.write((" "+ite).encode('big5hkscs'))
            ans += 1
        fileout.write("\n".encode('big5hkscs'))
    print(ans)
