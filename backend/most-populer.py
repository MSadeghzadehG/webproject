# Description:
#   Count The Most popular Words In Files And Sort Them
import collections
import os
import sys
import time
import operator

def main():
    start_time = time.time()
    Lan = sys.argv[1].capitalize() #programming language name
    extension = sys.argv[1] # extension of filenames
    mostCommon = {} #a list that contain most popular words
    #-----------------------find files
    files = os.listdir(Lan) 
    for file1 in files:
        if not file1.endswith('.'+extension):
            files.remove(file1)
    print('num of files : '+ str(len(files)))
    #-----------------------exploring in files
    for file1 in files:
        #print(os.path.join("/mydir", file))
        f = open(Lan+'/'+ str(file1),'r')
        code = f.read()
        words = code.split(' ') # seprates all words in the source code
        counter = collections.Counter(words) 
        thisMostCommon = dict(counter.most_common()) # creates tuples of word and num of repetes in this source code
        for wordAndNum in thisMostCommon:
            if wordAndNum in mostCommon:
                mostCommon.update({wordAndNum : thisMostCommon.get(wordAndNum)+mostCommon.get(wordAndNum)})
            else :
                mostCommon.update({wordAndNum :thisMostCommon.get(wordAndNum)})
        f.close()
    mostCommon = sorted(list(mostCommon.items()), key=operator.itemgetter(1), reverse=True)# sorts mostCommon
    #print(mostCommon)
    f = open(Lan+'/mostCommonWords.csv','w')
    f.write(str(mostCommon))
    f.close()
    print("--- %s seconds ---" % (time.time() - start_time))
    print('All Done!')
    
if __name__ == "__main__":
    main()