from lxml import html
from collections import Iterable
import requests
import os
import sys
from termcolor import colored
import time

mainUrl = "https://github.com/topics"
tabs = []   
trees = []
lan = sys.argv[1]
Lan = lan.capitalize()
mode = sys.argv[3]
goal = sys.argv[2]
path = Lan+'/'

def newTab(url):
    #, headers={'Content-Type': 'text/html'}
    try:
        start_time = time.time()
        request = requests.get(url)
        #while request.status_code != 200:
        #for x in range (0,5):  
        #    b = "Loading" + "." * x
        #    print(request.status_code)
        #    print (b, end="\r")
        #    time.sleep(1)
        if request.status_code == 200:
                tabs.append(request)
                trees.append(html.fromstring(tabs[len(tabs)-1].content))
                print('newTab '+ str(len(tabs)) + ' : ' +tabs[len(tabs)-1].url)
                print("--- %s seconds ---" % (time.time() - start_time))
        else:
            print('Web site does not responding'+ request.status_code)
    except requests.exceptions.ConnectionError:
        print(colored("Connection refused",'red'))
        newTab(url)
    except (Exception) as e :
        if hasattr(e, 'message'):
            print(colored('Unknown Error :' + e.message,'red'))
        else:
            print(colored('Unknown Error :' + str(e),'red'))
        newTab(url)

def changeTab(num,url):
    try:
        start_time = time.time()
        request = requests.get(url)
        if request.status_code == 200:
            lastUrl = tabs[num].url
            tabs[num] = request
            trees[num] = html.fromstring(tabs[num].content)
            print('changeTab ' + str(num+1) + ' : ' + lastUrl +' changed to ' + str(tabs[num].url)) 
            print("--- %s seconds ---" % (time.time() - start_time))
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
                    if Lan in text or lan in text:
                        print('Lan repos page found :')
                        changeTab(0,a)
                        bigCheck = False
    b = findElementsByXpath(trees[0],'//*[@id="js-pjax-container"]/div/div/div[2]/div/div/div[1]/button')
    b[0].set('aria-expanded' , 'true')
    c = findElementsByXpath(trees[0],'//*[@id="js-pjax-container"]/div/div/div[2]/div/div/div[1]/div/div/div[2]/a')
    bigCheck = True
    for d in c:
        for child in d.iter():
            if child.tag == 'span':
                if Lan in child.text and bigCheck:
                    print('repos language selected :')
                    changeTab(0,d.attrib['href'])
                    bigCheck = False
                    break
    print('repos page : '+ str(tabs[0].url))
    
def openNewRepo(repo):
    #browser.switch_to_window(browser.window_handles[0])
    print('repo is now open :')
    newTab('https://github.com' + repo.attrib['href'])
    #print(mainUrl+ repo.attrib['href'])
    
def searchDotLan(lanName):
    #elem = findElementsByXpath(trees[1],'/html/body/div[1]/header/div/div[2]/div/div/div/form/label/input[1]')
    #elem[0].set('text' , '.'+str(lan))
    print('search lan in repo done')
    Lansearch = Lan
    lansearch = lan
    if lan == 'c#':
        lansearch = 'cs'
        Lansearch = 'C%23'
    elif lan == 'c++':
        lansearch = 'cpp'
        Lansearch = 'C%2B%2B'
    elif lan == 'javascript':
        lansearch = 'js'
    changeTab(1,tabs[1].url+'/search?l='+Lansearch+'&q=.'+lansearch+'&type=&utf8=✓')
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
    newTab(filelink)
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
    saveFilesInThisPage(numOfFileFound)
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
        openNewRepo(repo)
        searchDotLan(lan)
        selectLan(Lan)
        saveFilesInThisPage(numOfFileFound)
        #input()
        checkAllRepoPages(numOfFileFound)
        numOfReposChecked[0] = numOfReposChecked[0] + 1
        print("---------------------------------")
        print (str(numOfReposChecked)+" Repository Checked")
        print("---------------------------------")

def main():
    numOfFileFound = [0]
    numOfReposChecked = [0]
    newTab(mainUrl)
    provideLanReposPage()
    reposearch(numOfFileFound,numOfReposChecked)
    #while (numOfFileFound < goal)
    #    showMore = browser.find_element_by_xpath('//*[@id="js-pjax-container"]/div/div/div[2]/form/button')
    #    showMore.click()
    #    reposearch()

if __name__== "__main__":
    if lan == 'javascript':
        Lan = 'JavaScript'
    elif lan == 'html':
        Lan = 'HTML'
        
    if not os.path.exists(path):
        os.makedirs(path)      
    main()