import sys
import os
import shutil
import tarfile

CI_DIR = os.path.dirname(os.path.abspath(__file__))
SCRIPT_DIR = os.path.dirname(CI_DIR)
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
PLATFORM_DIR = os.path.join(ROOT_DIR, 'platform-device')

IMAGE_DIR_NAME = 'out'
TARGET_NAME = 'k-hypervisor_full_images.tar.gz'

class ZipFolder():
    def __init__(self, location):
        self.location = location

    def makeCompress(self):
        try:
            if os.path.exists(self.location):
                print str(self.location)
                compressTar = tarfile.open(TARGET_NAME, "w:gz")
                compressTar.add(self.location)
                compressTar.close()
                print "Compress complete ", self.location
                return True
            else:
                print " (ZipFile)No Such Folder ",self.location
                return False
        except:
            print str(sys.exc_info())
            return False

def ArchiveImage():
    print '@@ Compress images @@'
    product = str(os.getenv('TARGET_PRODUCT'))

    os.chdir(IMAGE_DIR_NAME)

    target = product + "/" + str(os.getenv('CI_BUILD_DIR')) + "/" + TARGET_NAME

    zipFolder = ZipFolder(product)
    if zipFolder.makeCompress() == False :
        return False
    try:
        shutil.move(TARGET_NAME, target)
    except IOError, e:
        print "Unable to copy file. %s" % TARGET_NAME
        return False

    print '@@ Compress Done@@'
    return True

if __name__ == '__main__':
    ArchiveImage()

