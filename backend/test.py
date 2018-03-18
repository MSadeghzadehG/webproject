f = open('Java/'+'annotations.BackpressureSupport.java' ,'r')
import collections
code = f.read()
words1 = code.split(' ') # seprates all words in the source code
f.close()
counter = collections.Counter(words1) 
thisMostCommon = counter.most_common()
words = dict(thisMostCommon)
words.update({'a':4})
print('a' in words)
