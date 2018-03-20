import requests
import os
import sys
import time
from lxml import html
from collections import Iterable
from termcolor import colored
from math import floor

# TO DO :
# multi threading
# comment!!!
# write a file that gives lan name and run requests-p and most-popular and removeComments
# determine how many words do we need?
# what to do when a request is not responding?
# how to send an ajax request in main() and provideLanReposPage() ?

mainUrl = "https://github.com/topics"
tabs = []   
trees = []
lan = sys.argv[1]
goal = int(sys.argv[2])
mode = sys.argv[3]
Lan = []
extension = ['']
lanSearch = []
path = ['']
detected = [0]

def Timer(num):
    if 0.1 * ((3/2)**num) >= 1:
        for i in range(0,floor(0.1 * ((3/2)**num))):
            b = "Reconnect in " + str(floor(0.1 * ((3/2)**num))-i) + ' seconds'
            print(b)
            sys.stdout.write("\033[F") #back to previous line
            sys.stdout.write("\033[K") #clear line
            time.sleep(1)
    else :
        b = "Reconnect in " + str(0.1 * ((3/2)**num)) + ' seconds'
        print(b)
        time.sleep(0.1 * ((3/2)**num))

def newTab(i,url):
    #, headers={'Content-Type': 'text/html'}
    i = int(i)
    try:
        start_time = time.time()
        request = requests.get(url)
        if request.status_code == 200 and not isDetected(request):
                i = 0 
                tabs.append(request)
                trees.append(html.fromstring(tabs[len(tabs)-1].content))
                print('newTab '+ str(len(tabs)) + ' : ' +tabs[len(tabs)-1].url)
                print("--- %s seconds ---" % (time.time() - start_time))
        elif isDetected(request):
            print(request.status_code)
            newTab(i,url)
        elif request.status_code != 200 :
            print(request.status_code)

    except requests.exceptions.ConnectionError:
        i += 1
        print(colored("Connection refused",'red'))
        Timer(i)
        newTab(i,url)
    except (Exception) as e :
        if hasattr(e, 'message'):
            print(colored('Unknown Error :' + e.message,'red'))
        else:
            print(colored('Unknown Error :' + str(e),'red'))
        newTab(0.1,url)

def changeTab(i,num,url):
    num = int(num)
    try:
        start_time = time.time()
        request = requests.get(url)
        if request.status_code == 200 and not isDetected(request):
            i = 0
            lastUrl = tabs[num].url
            tabs[num] = request
            trees[num] = html.fromstring(tabs[num].content)
            print('changeTab ' + str(num+1) + ' : ' + lastUrl +' changed to ' + str(tabs[num].url)) 
            print("--- %s seconds ---" % (time.time() - start_time))
        elif isDetected(request):
            print(request.status_code)
            changeTab(i,num,url)
    except requests.exceptions.ConnectionError:
        i += 1
        print(colored("Connection refused",'red'))
        Timer(i)
        changeTab(i,num,url)
    except (Exception) as e :
        if hasattr(e, 'message'):
            print(colored('Unknown Error :' + e.message,'red'))
        else:
            print(colored('Unknown Error :' + str(e),'red'))
        changeTab(i,num,url)

def closeTab(num):
    print('closeTab '+ str(num+1) + ' : ' +tabs[num].url)
    tabs.remove(tabs[num])
    trees.remove(trees[num])

def isDetected(request):
    tree = html.fromstring(request.content)
    elem = findElementsByXpath(tree,'/html/body/div/p')
    timer = 45
    if len(elem)>0 :
        if 'triggered' in elem[0].text :
            detected[0] += 1
            print(colored('Detected','blue'))
            for x in range (0,timer):  
                b = "Wait for " + str(timer-x) + ' seconds'
                print(b)
                sys.stdout.write("\033[F") #back to previous line
                sys.stdout.write("\033[K") #clear line
                time.sleep(1)
            return True
        else :
            return False
    else:
        return False

def findElementsByXpath(tree,xpath):
    list = []
    for link in tree.xpath(xpath):
        list.append(link)
    return list

def provideLanReposPage():
    #HOW TO SEND AN AJAX REQ?
    #elem = findElementsByXpath(trees[0],'//*[@id="js-pjax-container"]/div[3]/div[1]/form/button')
    #changeTab(0,elem[0].attrib['href'])    
    elems = findElementsByXpath(trees[0],'//*[@id="js-pjax-container"]/div[3]/div[1]/ul[1]/li')
    bigCheck = True
    for elem in elems:
        if isinstance(elem,Iterable) and bigCheck == True:
            a = ''
            text = ''
            check = True
            for child in elem.iter():
                if bigCheck == True:
                    if child.tag == 'a':
                        a = child.attrib['href']
                        a = ("https://github.com"+str(a))
                    if child.tag == 'p' and check == True:
                        text = child.text
                        check = False
                    if Lan[0] in text:
                        print('Lan repos page found :')
                        print(a)
                        changeTab(0,0,a)
                        bigCheck = False
    b = findElementsByXpath(trees[0],'//*[@id="js-pjax-container"]/div/div/div[2]/div/div/div[1]/button')
    b[0].set('aria-expanded' , 'true')
    c = findElementsByXpath(trees[0],'//*[@id="js-pjax-container"]/div/div/div[2]/div/div/div[1]/div/div/div[2]/a')
    bigCheck = True
    for d in c:
        for child in d.iter():
            if child.tag == 'span':
                if Lan[0] in child.text and bigCheck:
                    print('repos language selected :')
                    changeTab(0,0,d.attrib['href'])
                    bigCheck = False
                    break
    print('repos page : '+ str(tabs[0].url))

def lanProperties(lan,Lan,extension,lanSearch):
    Lan.append(lan.capitalize())
    lanSearch.append(Lan[0])
    if lan == 'java' :
        extension[0] = '.java'
    elif lan == 'c++' :
        extension[0] = '.cpp'
        lanSearch[0] = 'C%2B%2B'
    elif lan == 'c#' :
        extension[0] = '.cs'
        lanSearch[0] = 'C%23'
    elif lan == 'python' :
        extension[0] = '.py'
    elif lan == 'html' :
        Lan[0] = 'HTML'
        extension[0] = '.html'
    elif lan == 'javascript' :
        Lan[0] = 'JavaScript'
        extension[0] = '.js'
    elif lan == 'css' :
        Lan[0] = 'CSS'
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

def goalReach(numOfReposChecked):
    print(colored(str(goal)+' FILE SAVED!','green'))
    print(colored('FROM '+ str(numOfReposChecked[0])+' REPOSITORIES!','green'))
    print(colored('AND DETECTED FOR '+ str(detected[0])+' TIMES! =)','blue'))
    exit()

def openNewRepo(repo):
    #browser.switch_to_window(browser.window_handles[0])
    print('repo is now open :')
    newTab(0.1,'https://github.com' + repo.attrib['href'])
    #print(mainUrl+ repo.attrib['href'])
    
def searchDotLan(lanName):
    #elem = findElementsByXpath(trees[1],'/html/body/div[1]/header/div/div[2]/div/div/div/form/label/input[1]')
    #elem[0].set('text' , '.'+str(lan))
    print('search lan in repo done')
    changeTab(0,1,tabs[1].url+'/search?l='+lanSearch[0]+'&q='+extension[0]+'&type=&utf8=✓')
    #elem1 = findElementsByXpath(trees[1],'/html/body/div[1]/header/div/div[2]/div/div/div/form/label/a')
    #changeTab(1,tabs[1].url + elem1[0].attrib['href'])
    
def selectLan(lanName):
    links = findElementsByXpath(trees[1],'//*[@id="js-repo-pjax-container"]/div/div[1]/div/div[1]/div[1]/div/ul/li[1]/a')
    for link in links:
        print(link.attrib['href'])
        if  lanName == link.text:   
            print('codes lan selected :')
            changeTab(0,1,link.attrib['href'])
            break
        #print(link.text)

def saveNewFile(filelink,fileName,numOfFileFound,numOfReposChecked):
    if numOfFileFound[0] < goal :
        newTab(0.1,filelink)
        changeTab(0,2,'https://github.com'+findElementsByXpath(trees[2],'//*[@id="raw-url"]')[0].attrib['href'])
        code = tabs[2].text
        print('filename has created : ' + fileName)
        f = open(path[0]+ fileName ,'w')
        f.write(str(code))
        f.close()
        closeTab(2)
        numOfFileFound[0] = numOfFileFound[0] + 1
        print(str(numOfFileFound[0])+" File Found")
    elif goal == numOfFileFound[0] :
        goalReach(numOfReposChecked)
           
def saveFilesInThisPage(numOfFileFound,numOfReposChecked):
    filelinks = findElementsByXpath(trees[1],'//*[@id="code_search_results"]/div[1]/div/div[1]/a')
    print('files page is loaded')
    for filelink in filelinks:
        fileName = str('https://github.com'+filelink.attrib['href'])
        fileName = fileName.split("/")
        fileName = str(fileName[len(fileName)-2])+'.'+str(fileName[len(fileName)-1])
        if mode == 'add':
            if not os.path.exists(path[0]+fileName):
                saveNewFile('https://github.com'+filelink.attrib['href'],fileName,numOfFileFound,numOfReposChecked)
            else : 
                 print('the file has created')
        elif mode == 'reset' :
            saveNewFile('https://github.com'+filelink.attrib['href'],fileName,numOfFileFound,numOfReposChecked)
        #print('https://github.com'+filelink.attrib['href'])

def checkAllRepoPages(numOfFileFound,numOfReposChecked):
    elems = [1]
    saveFilesInThisPage(numOfFileFound,numOfReposChecked)
    while len(elems)>0 :
        check = False
        elems = findElementsByXpath(trees[1],'//*[@id="code_search_results"]/div[2]/div/a')
        for elem in elems:
            if 'Next' in elem.text:
                changeTab(0,1,'https://github.com'+elem.attrib['href'])
                check = True
                break
        if check :
            saveFilesInThisPage(numOfFileFound,numOfReposChecked)
        else :
            break
        #input()
    closeTab(1)

def reposearch(numOfFileFound,numOfReposChecked):
    repos = findElementsByXpath(trees[0],'//h3/a')
    for repo in repos:
        if (not repo.attrib['href']+'\n' in open(path[0]+'REPOS.txt','r').read()) or (mode == 'reset'):
            openNewRepo(repo)
            searchDotLan(lan)
            selectLan(Lan)
            #input()
            checkAllRepoPages(numOfFileFound,numOfReposChecked)
            numOfReposChecked[0] = numOfReposChecked[0] + 1
            if mode == 'add':
                f = open(path[0]+'REPOS.txt' ,'a')
                f.write(repo.attrib['href']+'\n')
                f.close()
        print("---------------------------------")
        print ( str(repos.index(repo)+1) +" Repository Checked")
        print("---------------------------------")
        
def main():
    lanProperties(lan,Lan,extension,lanSearch)
    print(lan + ' ' + Lan[0] +  ' '+ extension[0] + ' ' + lanSearch[0])
    
    path[0] = 'collectedFiles/'+Lan[0]+'/'
    if not os.path.exists(path[0]):
        os.makedirs(path[0])
    f = open(path[0]+'REPOS.txt' ,'a')
    f.close()

    numOfFileFound = [0]
    numOfReposChecked = [0]
    if mode == 'add':
        numOfFileFound[0] = len(os.listdir(path[0]))
        numOfReposChecked[0] = len(open(path[0]+'REPOS.txt').read().splitlines())    
    
    print(numOfFileFound[0])
    print(numOfReposChecked[0])
    
    newTab(0.1,mainUrl)
    provideLanReposPage()
    reposearch(numOfFileFound,numOfReposChecked)
    #while (numOfFileFound < goal)
    #    showMore = browser.find_element_by_xpath('//*[@id="js-pjax-container"]/div/div/div[2]/form/button')
    #    showMore.click()
    #    reposearch()

if __name__== "__main__":
    main()