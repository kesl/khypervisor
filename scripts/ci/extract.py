import sys
import os
import shutil
import distutils.core

CI_DIR = os.path.dirname(os.path.abspath(__file__))
SCRIPT_DIR = os.path.dirname(CI_DIR)
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
PLATFORM_DIR = os.path.join(ROOT_DIR, 'platform-device')

IMAGE_DIR_NAME = 'out'

def ExtractImage():
    print '@@ extract images @@'

    product = str(os.getenv('TARGET_PRODUCT'))

    image_dir = ROOT_DIR + "/" + IMAGE_DIR_NAME + "/" + product + "/"
    image_dir += str(os.getenv('CI_BUILD_DIR'))
    print image_dir

    if not os.path.exists(image_dir):
        os.makedirs(image_dir)

    target_dir = PLATFORM_DIR + "/" + product

    # copy hypervisor image
    print 'copy hypervisor image'
    hypervisor_image = str(os.getenv('HYPERVISOR_BIN'))
    try:
        shutil.copy2(target_dir + "/" + hypervisor_image, image_dir + "/")
    except IOError, e:
        print "Unable to copy file. %s" % hypervisor_image
        return False

    # copy guest images
    print 'copy guest image'
    guest_image = str(os.getenv('GUEST_IMAGE_DIR'))
    try:
        distutils.dir_util.copy_tree(target_dir + "/" + guest_image,
                                     image_dir + "/")
    except IOError, e:
        print "Unable to copy file. %s" % guest_image
        return False

    # copy bootloader images
    if (os.getenv('UBOOT_DIR') != "") :
        print 'copy bootloader image'
        bootloader_image = str(os.getenv('UBOOT_DIR'))
        bootloader_image = bootloader_image + "/" + str(os.getenv('UBOOT'))
        try:
            shutil.copy2(target_dir + "/" + bootloader_image, image_dir + "/")
        except IOError, e:
            print "Unable to copy file. %s" % bootloader_image
            return False

    print '@@ extract done@@'
    return True

if __name__ == '__main__':
    ExtractImage()

