import os
import sys
import time



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

def main():
    start_time = time.time()
    Lan = sys.argv[1].capitalize() #programming language name
    extension = sys.argv[1] # extension of filenames
    path = 'collecting/collectedFiles/'+Lan+'/'
    files = os.listdir(path)  
    if not os.path.exists(path +'edited/'):
        os.makedirs(path +'edited/')  
    for file1 in files:
        if not file1.endswith('.'+extension):
            files.remove(file1)
    print('num of files : '+ str(len(files)))        
    input()
    numOfFile = 0
    for file1 in files:
        f = open(path + str(file1),'r')
        code = f.read()
        f.close()
        #-------------
        code = removeOneLineComments(code,'//')
        code = removeMultiLineComments(code,'/*','*/')
        #-------------
        f = open(path +'edited/' + str(file1),'w')
        f.write(code)
        f.close()
        numOfFile += 1
        print(numOfFile)


    print("--- %s seconds ---" % (time.time() - start_time))
    print('All Done!')


if __name__ == "__main__":
    main()