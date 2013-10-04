# !/usr/bin/python

import re
import codecs 
import os

ip = "10.0.3.1"
#ip = r'10.200.*'

"""
from the file get the ip
"""
#with open("check-free-ip.to.rdpzhitong.rdp") as file:
#    ip = file.readline().strip()

#searchText = r"((2[0-4]\d|25[0-5]|[01]?\d\d?)\.){3}(2[0-4]\d|25[0-5]|[01]?\d\d?\.)"
#searchText = r"^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$"
searchText = r"\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}"
#searchText = r"10.200.*" 

"""
replace text first be found
"""
def replaceFileText(fileName,model,searchText,replaceText):
    with open(fileName,model,encoding) as file:
        line = file.read()
        seekNum = line.find(searchText)
        if seekNum != -1:
           file.seek(seekNum)
           file.write(replaceText)

"""
replace all  text 
"""
def replaceFileTextAll(fileName,model,searchText,replaceText):
    with open(fileName,model) as file:
        seekNum = 0
        for line in file.readlines():
            lineNum = line.find(searchText)
            if lineNum != -1:
                print "find:",line
                file.seek(seekNum + lineNum)
                file.write(replaceText)  	
            seekNum += len(line)

"""
replace the text in a utf-16 little endian file
"""
def replaceUTF16LeFileText(fileName,model,searchText,replaceText,encoding):
    tempFileName = "tmp_"+fileName 
    with codecs.open(fileName,model,encoding) as inFile:
         with codecs.open(tempFileName,'w',encoding) as outFile:
              for line in inFile:
                  fixed_line = re.sub(searchText,replaceText,line)
                  outFile.write(fixed_line.strip() + "\n")
    #os.remove(fileName)
    #os.rename(tempFileName,fileName)
    #os.system("chown liuan:liuan " + fileName)

#def replaceUTF16LeFileTextTwo(fileName,model,searchText,replaceText,encoding):
#    with codecs.open(fileName,model,encoding) as file:
#        seekNum = 0
#        for line in inFile:


def replaceFileTextV3(fileName,searchText,replaceText,encoding):
    lines = []
    with codecs.open(fileName,"r",encoding) as file:
        for line in file:
            lines.append(re.sub(searchText,replaceText,line))
    with codecs.open(fileName,"w",encoding) as file:
        for line in lines:
            file.write(line)


def replaceFileTextWithDifferentCoding(fileName,model,searchText,replaceText,encoding):
    with codecs.open(fileName,model,encoding) as file:
        seekNum = 0
        for line in file:
            lineNum = line.encode(encoding).find(searchText.decode().encode(encoding))            
            if lineNum != -1:
                print line.strip()
                file.seek(seekNum + lineNum)
                file.write(replaceText)
            seekNum += len(line.encode(encoding))

def replaceUTF16LeFileText(fileName,model,searchText,replaceText,encoding):
    with codecs.open(fileName,model,encoding) as file:
        seekNum = 0
        for line in file:
            lineNum = line.find(searchText)            
            if lineNum != -1:
                file.seek(seekNum + 2*lineNum)
                file.write(replaceText)
            seekNum += 2*len(line)

#replaceFileTextAll("rdpzhitong.html","r+",searchText,ip)

#replaceUTF16LeFileText("rdpzhitong_test.rdp","r+",r"10.200.*",ip,"utf-16-le")
#replaceFileTextWithDifferentCoding("rdpzhitong_test.rdp","r+",searchText,ip,"utf-16-le")
replaceFileTextV3("rdpzhitong_test.html",searchText,ip,"ascii")
replaceFileTextV3("rdpzhitong_test.rdp",searchText,ip,"utf-16-le")
