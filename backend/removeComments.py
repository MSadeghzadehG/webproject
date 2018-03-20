import os
import sys
import time

def lanProperties(lan,Lan,extension):
    Lan[0] = lan.capitalize()
    if lan == 'java' :
        extension[0] = '.java'
    elif lan == 'c++' :
        extension[0] = '.cpp'
    elif lan == 'c#' :
        extension[0] = '.cs'
    elif lan == 'python' :
        extension[0] = '.py'
    elif lan == 'html' :
        Lan = 'HTML'
        extension[0] = '.html'
    elif lan == 'javascript' :
        Lan = 'JavaScript'
        extension[0] = '.js'
    elif lan == 'css' :
        Lan = 'CSS'
        extension[0] = '.css'
    elif lan == 'haskell' :
        extension[0] = '.hs'
    elif lan == 'kotlin' :
        extension[0] = '.kt'
    elif lan == 'ruby' :
        extension[0] = '.rb'
    elif lan == 'rust' :
        extension[0] = '.rs'
    elif lan == 'scala' :
        extension[0] = '.scala'
    elif lan == 'swift' :
        extension[0] = '.swift'
    elif lan == 'c' :
        extension[0] = '.c'

def findFiles(path,extension):
    files = os.listdir(path)  
    if not os.path.exists(path +'edited/'):
        os.makedirs(path +'edited/')  
    for file1 in files:
        if not file1.endswith('.'+extension):
            files.remove(file1)
    print('num of files : '+ str(len(files)))
    return files

def removeOneLineComments(code,commenter):
    code1 = ''
    line = 0
    check = False
    for i in range(0,len(code)):
        for j in range(0,len(commenter)) :
            if code[i+j] != commenter[j]:
                break
            if j == len(commenter)-1:
                check = True
        if not check:
            code1 = code1 + code[i]
        if code[i]=='\n':
            line += 1
            if check and len(code)>i:
                check = False
                i += 1
    return code1

def removeMultiLineComments(code,start,end):
    code1 = ''
    line = 0
    check = False
    for i in range(0,len(code)):
        for j in range(0,len(start)) :
            if code[i+j] != start[j]:
                break
            if j == len(start)-1:
                check = True
        if code[i]=='\n':
            line += 1
        if i>=len(end):
            for j in range(0,len(end)) :
                if code[i-len(end)+j] != end[j]:
                    break
                if j == len(end)-1:
                    check = False
        if not check and i<len(code):
            code1 = code1 + code[i] 
    return code1 

def output(code,file,path):
    f = open(path +'edited/' + str(file),'w')
    f.write(code)
    f.close()

def main():
    start_time = time.time()
    lan = sys.argv[1]
    Lan = ['']
    extension = ['']
    lanProperties(lan,Lan,extension)
    path = 'collecting/collectedFiles/'+Lan[0]+'/'

    files = findFiles(path,extension[0])           
    numOfEditedFile = 0
    
    for file1 in files:
        code = open(path + str(file1),'r').read()
        #-------------
        code = removeOneLineComments(code,'//')
        code = removeMultiLineComments(code,'/*','*/')
        #-------------
        output(code,file1,path)
        numOfEditedFile += 1
        print(numOfEditedFile)

    print("--- %s seconds ---" % (time.time() - start_time))
    print('All Done!')

if __name__ == "__main__":
    main()