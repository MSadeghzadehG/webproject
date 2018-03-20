from lxml import html
from collections import Iterable
import requests
import os
import sys
from termcolor import colored
import time

# TO DO :
# multi threading
# get files as equal as goal
# comment!!!

mainUrl = "https://github.com/topics"
tabs = []   
trees = []
lan = sys.argv[1]
goal = sys.argv[2]
mode = sys.argv[3]
Lan = []
extension = ['']
lanSearch = []
path = ''

def isDetected(request):
    tree = html.fromstring(request.content)
    elem = findElementsByXpath(tree,'/html/body/div/p')
    if len(elem)>0 :
        if 'triggered' in elem[0].text :
            print(colored('Detected','blue'))
            for x in range (0,60):  
                b = "Wait for " + str(60-x) + ' seconds'
                print (b, end="\r")
                time.sleep(1)
            return True
        else :
            return False
    else:
        return False

def newTab(i,url):
    #, headers={'Content-Type': 'text/html'}
    i = int(i)
    try:
        start_time = time.time()
        request = requests.get(url)
        #while request.status_code != 200:
        #for x in range (0,5):  
        #    b = "Loading" + "." * x
        #    print(request.status_code)
        #    print (b, end="\r")
        #    time.sleep(1)
        if request.status_code == 200 and not isDetected(request):
                tabs.append(request)
                trees.append(html.fromstring(tabs[len(tabs)-1].content))
                print('newTab '+ str(len(tabs)) + ' : ' +tabs[len(tabs)-1].url)
                print("--- %s seconds ---" % (time.time() - start_time))
        elif isDetected(request):
            newTab(i,url)
        else:
            print('Web site does not responding'+ request.status_code)
    except requests.exceptions.ConnectionError:
        j=i
        while j>0 :
            print('Reconnect in %d seconds...' , j)
            time.sleep(1)
            j -= 1
        print(colored("Connection refused",'red'))
        newTab(i * (3/2),url)
    except (Exception) as e :
        if hasattr(e, 'message'):
            print(colored('Unknown Error :' + e.message,'red'))
        else:
            print(colored('Unknown Error :' + str(e),'red'))
        newTab(0.1,url)

def changeTab(num,url):
    num = int(num)
    try:
        start_time = time.time()
        request = requests.get(url)
        if request.status_code == 200 and not isDetected(request):
            lastUrl = tabs[num].url
            tabs[num] = request
            trees[num] = html.fromstring(tabs[num].content)
            print('changeTab ' + str(num+1) + ' : ' + lastUrl +' changed to ' + str(tabs[num].url)) 
            print("--- %s seconds ---" % (time.time() - start_time))
        elif isDetected(request):
            changeTab(num,url)
    except requests.exceptions.ConnectionError:
        print(colored("Connection refused",'red'))
        changeTab(num,url)
    except (Exception) as e :
        if hasattr(e, 'message'):
            print(colored('Unknown Error :' + e.message,'red'))
        else:
            print(colored('Unknown Error :' + str(e),'red'))
        changeTab(num,url)

def closeTab(num):
    print('closeTab '+ str(num+1) + ' : ' +tabs[num].url)
    tabs.remove(tabs[num])
    trees.remove(trees[num])
    
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
                        changeTab(0,a)
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
                    changeTab(0,d.attrib['href'])
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

def openNewRepo(repo):
    #browser.switch_to_window(browser.window_handles[0])
    print('repo is now open :')
    newTab(0.1,'https://github.com' + repo.attrib['href'])
    #print(mainUrl+ repo.attrib['href'])
    
def searchDotLan(lanName):
    #elem = findElementsByXpath(trees[1],'/html/body/div[1]/header/div/div[2]/div/div/div/form/label/input[1]')
    #elem[0].set('text' , '.'+str(lan))
    print('search lan in repo done')
    changeTab(1,tabs[1].url+'/search?l='+lanSearch[0]+'&q='+extension[0]+'&type=&utf8=✓')
    #elem1 = findElementsByXpath(trees[1],'/html/body/div[1]/header/div/div[2]/div/div/div/form/label/a')
    #changeTab(1,tabs[1].url + elem1[0].attrib['href'])
    
def selectLan(lanName):
    links = findElementsByXpath(trees[1],'//*[@id="js-repo-pjax-container"]/div/div[1]/div/div[1]/div[1]/div/ul/li[1]/a')
    for link in links:
        print(link.attrib['href'])
        if  lanName == link.text:   
            print('codes lan selected :')
            changeTab(1,link.attrib['href'])
            break
        #print(link.text)

def saveNewFile(filelink,fileName,numOfFileFound):
    newTab(0.1,filelink)
    changeTab(2,'https://github.com'+findElementsByXpath(trees[2],'//*[@id="raw-url"]')[0].attrib['href'])
    code = tabs[2].text
    print('filename has created : ' + fileName)
    f = open(path+ fileName ,'w')
    f.write(str(code))
    f.close()
    closeTab(2)
    numOfFileFound[0] = numOfFileFound[0] + 1
    print(str(numOfFileFound[0])+" File Found")
           
def saveFilesInThisPage(numOfFileFound):
    filelinks = findElementsByXpath(trees[1],'//*[@id="code_search_results"]/div[1]/div/div[1]/a')
    print('files page is loaded')
    for filelink in filelinks:
        fileName = str('https://github.com'+filelink.attrib['href'])
        fileName = fileName.split("/")
        fileName = str(fileName[len(fileName)-2])+'.'+str(fileName[len(fileName)-1])
        if mode == 'add':
            if not os.path.exists(path+fileName):
                saveNewFile('https://github.com'+filelink.attrib['href'],fileName,numOfFileFound)
            else : 
                 print('the file has created')
        elif mode == 'reset' :
            saveNewFile('https://github.com'+filelink.attrib['href'],fileName,numOfFileFound)
        #print('https://github.com'+filelink.attrib['href'])

def checkAllRepoPages(numOfFileFound):
    elems = [1]
    while len(elems)>0 :
        check = False
        elems = findElementsByXpath(trees[1],'//*[@id="code_search_results"]/div[2]/div/a')
        for elem in elems:
            if 'Next' in elem.text:
                changeTab(1,'https://github.com'+elem.attrib['href'])
                check = True
                break
        if check :
            saveFilesInThisPage(numOfFileFound)
        else :
            break
        #input()
    closeTab(1)

def reposearch(numOfFileFound,numOfReposChecked):
    repos = findElementsByXpath(trees[0],'//h3/a')
    for repo in repos:
        if (not repo.attrib['href']+'\n' in open(path+'REPOS.txt').read()) or (mode == 'reset'):
            openNewRepo(repo)
            searchDotLan(lan)
            selectLan(Lan)
            saveFilesInThisPage(numOfFileFound)
            #input()
            checkAllRepoPages(numOfFileFound)
            numOfReposChecked[0] = numOfReposChecked[0] + 1
            if mode == 'add':
                f = open(path+'REPOS.txt' ,'a')
            else:
                f = open(path+'REPOS.txt' ,'w')
            f.write(repo.attrib['href']+'\n')
            f.close()
        print("---------------------------------")
        print (str(numOfReposChecked)+" Repository Checked")
        print("---------------------------------")
        
def main():
    numOfFileFound = [0]
    numOfReposChecked = [0]
    newTab(0.1,mainUrl)
    provideLanReposPage()
    reposearch(numOfFileFound,numOfReposChecked)
    #while (numOfFileFound < goal)
    #    showMore = browser.find_element_by_xpath('//*[@id="js-pjax-container"]/div/div/div[2]/form/button')
    #    showMore.click()
    #    reposearch()

if __name__== "__main__":
    lanProperties(lan,Lan,extension,lanSearch)
    print(lan + ' ' + Lan[0] +  ' '+ extension[0] + ' ' + lanSearch[0])
    path = 'collectedFiles/'+Lan[0]+'/'
    if not os.path.exists(path):
        os.makedirs(path)
    f = open(path+'REPOS.txt' ,'w')
    f.close()
    main()