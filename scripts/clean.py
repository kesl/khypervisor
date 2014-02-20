import os
import shutil
import subprocess
import sys
import multiprocessing

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
PLATFORM_DIR = os.path.join(ROOT_DIR, 'platform-device')

def CleanTarget(product, dir, clean_script):
    print '@@@CLEAN_TARGET' + product + '@@@'
    target_dir = PLATFORM_DIR + "/" + product + "/" + dir
    os.chdir(target_dir)
    sys.stdout.flush()
    env = os.environ.copy()

    print clean_script
    retcode = subprocess.call(
          ['/bin/bash',
           '-c', clean_script],
          cwd=target_dir, env=env)
    if retcode:
        print '@@@STEP_CLEAN_BMGUEST_TARGET_FAILURE@@@'
        return 1

    return 0

def CleanNativeUboot(product):
    retcode = 0
    if (os.getenv('UBOOT_DIR') != None) :
        dir = str(os.getenv('UBOOT_DIR'))
        print dir
        clean_script = str(os.getenv('UBOOT_CLEAN_SCRIPT'))
        print clean_script
        retcode += CleanTarget(product, dir, clean_script)
        if retcode:
            print '@@@CLEAN_STEP failures@@@'
            sys.exit(retcode)


def CleanGuests(product):
    retcode = 0
    print '@CleanGeust'
    guest_count = int(os.getenv('GUEST_COUNT'))
    print "guest_count" + str(guest_count)
    for n in range(0, guest_count):
        guest_count = int(os.getenv('GUEST_COUNT'))
        guest_dir_string = "GUEST" + str(n) + "_DIR"
        guest_bin_string = "GUEST" + str(n) + "_BIN"
        guest_script_string = "GUEST" + str(n) + "_CLEAN_SCRIPT"

        dir = str(os.getenv(guest_dir_string))
        print dir
        image_name = str(os.getenv(guest_bin_string))
        print image_name
        clean_script = str(os.getenv(guest_script_string))
        print clean_script
        retcode += CleanTarget(product, dir, clean_script)
        if retcode:
            print '@@@CLEAN_STEP failures@@@'
            sys.exit(retcode)
    guest_dir = str(os.getenv('GUEST_IMAGE_DIR'))
    os.system("rm -f " + PLATFORM_DIR + "/" + product + "/" + guest_dir + "/*")

def CleanHypervisor(product):
    print '@Clean Hypervisor'
    retcode = 0
    dir = ""
    clean_script = str(os.getenv('HYPERVISOR_CLEAN_SCRIPT'))
    print clean_script
    retcode += CleanTarget(product, dir, clean_script)
    if retcode:
        print '@@@CLEAN_STEP failures@@@'
        sys.exit(retcode)

def CleanAll(product):
    print "@product clean start " + product

    CleanNativeUboot(product)
    CleanGuests(product)
    CleanHypervisor(product)

if __name__ == '__main__':
    clean_type = sys.argv[1]
    print clean_type
    print '@clean start'

    if (os.getenv('TARGET_PRODUCT') != None) :
        product = str(os.getenv('TARGET_PRODUCT'))
        print product
        if (clean_type == 'hypervisor'):
            CleanHypervisor(product)
        elif(clean_type == 'guest'):
            CleanGuests(product)
        else :
            CleanAll(product)
    else:
        print 'source platform-device/<platform_name>/build/<guest>'

