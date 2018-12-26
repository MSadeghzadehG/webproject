from selenium import webdriver
import selenium.webdriver.support.ui as ui
from selenium.webdriver.common.keys import Keys
from time import sleep
import os
import sys

slownessOfNetSpeed = int(sys.argv[2]) #1 to 10
lan = sys.argv[1]
Lan = lan.capitalize()
goal = sys.argv[3]
path = Lan+'/'
if not os.path.exists(path):
    os.makedirs(path)       
browser = webdriver.Firefox()    
mainUrl="https://github.com/topics/"

def provideLanReposPage():
    browser.switch_to_window(browser.window_handles[0])
    #while "Load more..." in browser.find_element_by_xpath('//*[@id="js-pjax-container"]/div[3]/div[1]/form/button').text:
    elem = browser.find_element_by_xpath('//*[@id="js-pjax-container"]/div[3]/div[1]/form/button')
    elem.click()    
    #print("finished")
    elems = browser.find_elements_by_xpath('/html/body/div[4]/div[1]/div[3]/div[1]/ul/li/a/div/p[1]')
    #print(len(elems))
    for elem in elems:
        if Lan in elem.text:
            elem.click()
            break
    b = browser.find_element_by_xpath('//*[@id="js-pjax-container"]/div/div/div[2]/div/div/div[1]/button')
    b.click()
    c = browser.find_elements_by_xpath('//*[@id="js-pjax-container"]/div/div/div[2]/div/div/div[1]/div/div/div[2]/a/span')
    for d in c:
        if Lan in d.text:
            d.click()
            break


def openNewRepo(repo):
    browser.switch_to_window(browser.window_handles[0])
    repo.send_keys(Keys.CONTROL + Keys.RETURN)
    browser.find_element_by_tag_name('body').send_keys(Keys.CONTROL + Keys.TAB)
    sleep(slownessOfNetSpeed)
    

def searchDotLan(lanName):
    browser.switch_to_window(browser.window_handles[1])
    elem1 = browser.find_element_by_class_name('header-search-input')
    if not lan == 'c++':
        elem1.send_keys('.' + lanName)
    else:
        elem1.send_keys('.' + 'cpp')
    elem1.send_keys(Keys.ENTER)
    sleep(slownessOfNetSpeed)


def selectLan(lanName):
    browser.switch_to_window(browser.window_handles[1])
    links = browser.find_elements_by_xpath('//*[@id="js-repo-pjax-container"]/div/div[1]/div/div[1]/div[1]/div/ul/li/a')
    #print(links)
    for link in links:
        if  lanName in link.text:   
            link.click()
            sleep(slownessOfNetSpeed)
            break
        #print(link.text)
        #input()


def saveNewFile(filelink,numOfFileFound):
    browser.switch_to_window(browser.window_handles[1])
    filelink.send_keys(Keys.CONTROL + Keys.RETURN)
    browser.find_element_by_tag_name('body').send_keys(Keys.CONTROL + Keys.TAB)
    sleep(slownessOfNetSpeed)
    browser.switch_to_window(browser.window_handles[2])
    file1 = browser.find_element_by_xpath('//*[@id="raw-url"]')
    file1.click()
    code = browser.find_element_by_xpath('/html/body/pre').text
    fileName = browser.current_url
    fileName = fileName.split("/")
    f = open(path+str(fileName[len(fileName)-2])+str(fileName[len(fileName)-1]),'w')
    f.write(str(code))
    f.close()            
    browser.close()
    numOfFileFound[0] = numOfFileFound[0] + 1
    print(str(numOfFileFound[0])+" File Found")

def saveFilesInThisPage(numOfFileFound):
    browser.switch_to_window(browser.window_handles[1])
    filelinks = browser.find_elements_by_xpath('//*[@id="code_search_results"]/div[1]/div/div[1]/a')        
    for filelink in filelinks:
        saveNewFile(filelink,numOfFileFound)

def checkAllRepoPages(numOfFileFound):
    browser.switch_to_window(browser.window_handles[1])
    elems = browser.find_elements_by_xpath('//*[@id="code_search_results"]/div[2]/div/a')
    while len(elems)>0 :
        browser.switch_to_window(browser.window_handles[1])
        elems = browser.find_elements_by_xpath('//*[@id="code_search_results"]/div[2]/div/a')
        for elem in elems:
            if 'Next' in elem.text:
                elem.click()
                sleep(slownessOfNetSpeed)
                break
        saveFilesInThisPage(numOfFileFound)
    browser.switch_to_window(browser.window_handles[1])
    browser.close()


def reposearch(numOfFileFound,numOfReposChecked):
    browser.switch_to_window(browser.window_handles[0])
    repos = browser.find_elements_by_xpath('//h3/a')
    for repo in repos:
        openNewRepo(repo)
        searchDotLan(lan)
        selectLan(Lan)
        saveFilesInThisPage(numOfFileFound)
        checkAllRepoPages(numOfFileFound)
        numOfReposChecked[0] = numOfReposChecked[0] + 1
        print("---------------------------------")
        print (str(numOfReposChecked)+" Repository Checked")
        print("---------------------------------")
    browser.quit()
   
 


def main():    
    numOfFileFound = [0]
    numOfReposChecked = [0]
    #main_window = browser.current_window_handle
    browser.get(mainUrl)
    provideLanReposPage()
    reposearch(numOfFileFound,numOfReposChecked)

    #while (numOfFileFound < goal)
    #    showMore = browser.find_element_by_xpath('//*[@id="js-pjax-container"]/div/div/div[2]/form/button')
    #    showMore.click()
    #    reposearch()

if __name__== "__main__":
  main()