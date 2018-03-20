# Description:
#   Count The Most popular Words In Files And Sort Them
import os
import sys
import time
from collections import Counter
from operator import itemgetter

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
    for file1 in files:
        if not file1.endswith(extension[0]):
            files.remove(file1)
    print('num of files : '+ str(len(files)))
    return files

def getWords(file,path):
    f = open(path + str(file),'r')
    code = f.read()
    words = code.split(' ') # seprates all words in the source code
    counter = Counter(words)
    f.close() 
    return dict(counter.most_common()) # creates tuples of word and num of repetes in this source code

def output(file,path):
    f = open(path+'allWords.csv','w')
    f.write(str(file))
    f.close()
    
def main():
    start_time = time.time()
    lan = sys.argv[1]
    Lan = ['']
    extension = ['']
    forEdited = sys.argv[2]

    lanProperties(lan,Lan,extension)
    if (forEdited == 'n')
        path = 'collecting/collectedFiles/'+Lan[0]+'/'
    else :
        path = 'collecting/collectedFiles/'+Lan[0]+'/edited/'
    allWords = {} #contains All words

    files = findFiles(path,extension)

    for file1 in files:
        thisWords = getWords(file1,path) #contains this file words
        for wordAndNum in thisWords:
            if wordAndNum in allWords:
                allWords.update({wordAndNum : thisWords.get(wordAndNum)+allWords.get(wordAndNum)})
            else :
                allWords.update({wordAndNum :thisWords.get(wordAndNum)})
    allWords = sorted(list(allWords.items()), key=itemgetter(1), reverse=True)# sorts allWords
    #print(allWords)

    #----------------------create output file
    output(allWords,path)
    print("--- %s seconds ---" % (time.time() - start_time))
    print('All Done!')
    
if __name__ == "__main__":
    main()