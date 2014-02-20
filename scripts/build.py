import os
import shutil
import subprocess
import sys
import multiprocessing

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
PLATFORM_DIR = os.path.join(ROOT_DIR, 'platform-device')

def BuildTarget(product, dir, build_script):
    print '@@@BUILD_TARGET' + product + '@@@'
    target_dir = PLATFORM_DIR + "/" + product + "/" + dir
    os.chdir(target_dir)
    sys.stdout.flush()
    env = os.environ.copy()

    print build_script
    retcode = subprocess.call(
          ['/bin/bash',
           '-c', build_script],
          cwd=target_dir, env=env)
    if retcode:
        print '@@@STEP_BUILD_BMGUEST_TARGET_FAILURE@@@'
        return 1

    return 0

def RenameGeustImage(guest, product, dir, build_script, image_name):
    print '@@@RENAME GUEST' + product + '@@@'
    target_dir = PLATFORM_DIR + "/" + product + "/" + dir
    os.chdir(target_dir)
    sys.stdout.flush()
    env = os.environ.copy()

    src_image = PLATFORM_DIR + "/" + product + "/" + dir + "/" + image_name
    print src_image
    guest_dir = str(os.getenv('GUEST_IMAGE_DIR'))
    dest_image = PLATFORM_DIR + "/" + product + "/" + guest_dir + "/guest" + str(guest) + ".bin"
    print dest_image

    shutil.copy2(src_image, dest_image)

    return 0

def BuildNativeUboot(product):
    retcode = 0
    if (os.getenv('UBOOT_DIR') != "") :
        dir = str(os.getenv('UBOOT_DIR'))
        print dir
        build_script = str(os.getenv('UBOOT_BUILD_SCRIPT'))
        print build_script
        retcode += BuildTarget(product, dir, build_script)
        if retcode:
            print '@@@BUILD_STEP failures@@@'
            sys.exit(retcode)

def BuildGuests(product):
    retcode = 0
    print '@BuildGeust'
    guest_count = int(os.getenv('GUEST_COUNT'))
    print "guest_count" + str(guest_count)
    for n in range(0, guest_count):
        guest_count = int(os.getenv('GUEST_COUNT'))
        guest_dir_string = "GUEST" + str(n) + "_DIR"
        guest_bin_string = "GUEST" + str(n) + "_BIN"
        guest_script_string = "GUEST" + str(n) + "_BUILD_SCRIPT"

        dir = str(os.getenv(guest_dir_string))
        print dir
        image_name = str(os.getenv(guest_bin_string))
        print image_name
        build_script = str(os.getenv(guest_script_string))
        print build_script
        retcode += BuildTarget(product, dir, build_script)
        if retcode:
            print '@@@BUILD_STEP failures@@@'
            sys.exit(retcode)

        retcode += RenameGeustImage(n, product, dir, build_script, image_name)
        if retcode:
            print '@@@BUILD_STEP failures@@@'
            sys.exit(retcode)

def BuildHypervisor(product):
    print '@Build Hypervisor'
    retcode = 0
    dir = ""
    build_script = str(os.getenv('HYPERVISOR_BUILD_SCRIPT'))
    print build_script
    retcode += BuildTarget(product, dir, build_script)
    if retcode:
        print '@@@BUILD_STEP failures@@@'
        sys.exit(retcode)

def BuildAll(product):
    print "@product build start " + product

    BuildNativeUboot(product)
    BuildGuests(product)
    BuildHypervisor(product)

if __name__ == '__main__':
    build_type = sys.argv[1]
    print build_type
    print '@build start'

    if (os.getenv('TARGET_PRODUCT') != None) :
        product = str(os.getenv('TARGET_PRODUCT'))
        print product
        if (build_type == 'hypervisor'):
            BuildHypervisor(product)
        elif(build_type == 'guest'):
            BuildGuests(product)
        else :
            BuildAll(product)
    else:
        print 'source platform-device/<platform_name>/build/<guest>'
