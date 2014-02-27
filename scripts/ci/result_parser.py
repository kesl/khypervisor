import os
import shutil
import subprocess
import sys
import re
import time
import simulator

CI_DIR = os.path.dirname(os.path.abspath(__file__))
SCRIPT_DIR = os.path.dirname(CI_DIR)
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
PLATFORM_DIR = os.path.join(ROOT_DIR, 'platform-device')

HYPERVISOR_LOG = 'hypervisor'
GUEST_LOG = 'guest'

class TestLog:
    def __init__(self):
        self.list = []

    def PrintTestResult(self, a_class, a_name, a_result):
        print str(a_class + "  " + a_name + "  " + a_result)

    def GetResult(self, a_class, a_func):
        result = False
        for g in self.list:
            if str(g['test_class']) == a_class and \
                str(g['test_func']) == a_func :
                self.PrintTestResult(str(g['test_class']),
                                str(g['test_func']),
                                str(g['test_result']).strip())
                if (str(g['test_result']).strip() == 'PASS'):
                    result = True
                elif (str(g['test_result']).strip() == 'FAILED'):
                    result = False
                    break

        return result

    def AddResult(self, a_result):
        self.list.append(a_result)

class ResultParser:
    def __init__(self):
        self._re = re.compile(r'[\[K\-HYPERVISOR\]TEST\#](?P<test>[^#]+)[#]'
                      '(?P<test_class>[^#]+)[#](?P<test_func>[^#]+)[#]'
                      '(?P<test_result>[^#]+)')
        self.guest_log = 'guest'
        self.hypervisor_log = 'hypervisor'
        self.test_log = TestLog()
    
    def GetLogResult(self, a_class, a_name):
        print "Test " +  str(a_class) + " " + str(a_name)
        return self.test_log.GetResult(a_class, a_name)
        
        
    def SaveLog(self, product, input):
        print product
        print input

        for line in open(input, 'rt'):
            #print line
            node = self._re.match(line)
            if node is not None:
                g  = node.groupdict()
                test_class = str(g['test_class'])
                print test_class
                self.test_log.AddResult(g)

        return True

    def ParseLog(self, product):
        print '@@ Parse Log @@'
        product = str(os.getenv('TARGET_PRODUCT'))
        target_dir = PLATFORM_DIR + "/" + product + "/"

        self.SaveLog(product, target_dir + "/" + HYPERVISOR_LOG + ".log")
        guest_count = int(os.getenv('GUEST_COUNT'))
        print "guest_count" + str(guest_count)
        for n in range(0, guest_count):
            self.SaveLog(product, target_dir + 
                    GUEST_LOG + str(n) + ".log")

        print '@@ Check done@@'

if __name__ == '__main__':
    product = str(os.getenv('TARGET_PRODUCT'))
    result_parser = ResultParser()
    
    result_parser.ParseLog(product)
    result_parser.GetLogResult('INSTALLATION', 'BOOT')

