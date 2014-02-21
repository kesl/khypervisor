import os
import shutil
import subprocess
import sys
import multiprocessing

from datetime import *

BUILDBOT_DIR = os.path.dirname(os.path.abspath(__file__))
TRUNK_DIR = os.path.dirname(BUILDBOT_DIR)
ROOT_DIR = os.path.dirname(TRUNK_DIR)
IMAGE_DIR_NAME = 'out'

def weekdays(day):
    days = ['Monday',
            'Tuesday',
            'Wednesday',
            'Thursday',
            'Friday',
            'Saturday',
            'Sunday'
    ]
    name = days[day] # get the index of the selected day
    return name

def UploadNightly(server_ip, user, release_dir, product_name):

    print '@@@Upload To File Server@@@'
    sys.stdout.flush()
    env = os.environ.copy()

    workweeks = "ww" + str(date.today().isocalendar()[1])
    print "workweeks : " + workweeks
    weekday = str(date.today().weekday())
    weekday_name = weekdays(int(weekday))
    print "weekday : " + weekday_name

    # create remote directory for daily images
    daily_create_cmd = "ssh " + user + "@" + server_ip + " \"mkdir -p "
    daily_create_cmd += release_dir + "/" + product_name + "/daily-image/"
    daily_create_cmd += workweeks + "/" + weekday_name + "\""
    print daily_create_cmd

    retcode = subprocess.call(
          ['/bin/bash',
           '-c', daily_create_cmd],
          cwd=ROOT_DIR, env=env)
    if retcode:
        print '@@@DAILY UPLOAD STEP_FAILURE@@@'
        return 1

    # copy daily images to file server
    cmd_string = "scp -p -r " + IMAGE_DIR_NAME + "/* " + user
    cmd_string +=  "@" + server_ip + ":" + release_dir + "/" + product_name
    print cmd_string
    daily_upload_string = cmd_string + "/daily-image/" + workweeks + "/"
    daily_upload_string += weekday_name + "/"
    print daily_upload_string

    retcode = subprocess.call(
          ['/bin/bash',
           '-c', daily_upload_string],
          cwd=ROOT_DIR, env=env)
    if retcode:
        print '@@@DAILY UPLOAD STEP_FAILURE@@@'
        return 1

    # change permission
    permission_cmd = "ssh " + user + "@" + server_ip + " \"chmod 755 -R "
    permission_cmd += release_dir + "/" + product_name + "/daily-image/"
    permission_cmd += workweeks + "/" + weekday_name + "\""
    print permission_cmd

    retcode = subprocess.call(
          ['/bin/bash',
           '-c', permission_cmd],
          cwd=ROOT_DIR, env=env)
    if retcode:
        print '@@@DAILY UPLOAD CHANGE PERMISSION STEP_FAILURE@@@'
        return 1

    # Sunday is 6
    if int(weekday) == 4 :
        # create remote directory for weekly images
        weekly_create_cmd = "ssh " + user + "@" + server_ip + " \"mkdir -p "
        weekly_create_cmd += release_dir + "/" + product_name + "/weekly-image/"
        weekly_create_cmd += workweeks + "\""
        print weekly_create_cmd

        retcode = subprocess.call(
              ['/bin/bash',
              '-c', weekly_create_cmd],
              cwd=ROOT_DIR, env=env)
        if retcode:
            print '@@@DAILY UPLOAD STEP_FAILURE@@@'
            return 1

        cmd_string = "scp -p -r " + IMAGE_DIR_NAME + "/* " + user
        cmd_string += "@" + server_ip + ":" + release_dir + "/" + product_name
        print cmd_string
        weekly_upload_string = cmd_string + "/weekly-image/"
        weekly_upload_string += workweeks + "/"
        print weekly_upload_string

        retcode = subprocess.call(
            ['/bin/bash',
             '-c', weekly_upload_string],
            cwd=ROOT_DIR, env=env)
        if retcode:
          print '@@@DAILY UPLOAD STEP_FAILURE@@@'
          return 1

        weekly_permission_cmd = "ssh " + user + "@" + server_ip
        weekly_permission_cmd += " \"chmod 755 -R " + release_dir
        weekly_permission_cmd += "/" + product_name + "/weekly-image/"
        weekly_permission_cmd += workweeks + "\""
        print weekly_permission_cmd

        retcode = subprocess.call(
            ['/bin/bash',
             '-c', weekly_permission_cmd],
            cwd=ROOT_DIR, env=env)
        if retcode:
          print '@@@WEEKLY UPLOAD CHANGE PERMISSION STEP_FAILURE@@@'
          return 1

    if retcode:
        print '@@@STEP_FAILURE@@@'
        return 1

    return 0


def UploadRelease(server_ip, user, release_dir, product_name):
    print '@@@Upload Release Image To File Server@@@'
    return 0


def UploadLatest(server_ip, user, release_dir, product_name):

    print '@@@Latest File Upload To File Server@@@'
    sys.stdout.flush()
    env = os.environ.copy()

    # clean up latest-image directory
    del_dir_cmd = "ssh " + user + "@" + server_ip + " \"rm -rf "
    del_dir_cmd += release_dir + "/" + product_name + "/latest-image/"
    del_dir_cmd += product_name + "/"
    del_dir_cmd += str(os.getenv('CI_BUILD_DIR')) + "/* \""
    print del_dir_cmd

    retcode = subprocess.call(
          ['/bin/bash',
           '-c', del_dir_cmd],
          cwd=ROOT_DIR, env=env)
    if retcode:
        print '@@@ LATEST UPLOAD FAILURE@@@'
        return 1

    latest_create_cmd = "ssh " + user + "@" + server_ip + " \"mkdir -p "
    latest_create_cmd += release_dir + "/" + product_name
    latest_create_cmd += "/latest-image\""
    print latest_create_cmd

    retcode = subprocess.call(
          ['/bin/bash',
           '-c', latest_create_cmd],
          cwd=ROOT_DIR, env=env)
    if retcode:
        print '@@@LATEST UPLOAD STEP_FAILURE@@@'
        return 1

    cmd_string = "scp -p -r " + IMAGE_DIR_NAME + "/* " + user
    cmd_string +=  "@" + server_ip + ":" + release_dir + "/" + product_name
    print cmd_string
    latest_upload_string = cmd_string + "/latest-image/"
    print latest_upload_string

    retcode = subprocess.call(
          ['/bin/bash',
           '-c', latest_upload_string],
          cwd=ROOT_DIR, env=env)

    if retcode:
        print '@@@ LATEST UPLOAD FAILURE@@@'
        return 1

    permission_cmd = "ssh " + user + "@" + server_ip + " \"chmod 755 -R "
    permission_cmd += release_dir + "/" + product_name + "/latest-image \""
    print permission_cmd

    retcode = subprocess.call(
          ['/bin/bash',
           '-c', permission_cmd],
          cwd=ROOT_DIR, env=env)
    if retcode:
        print '@@@ LATEST CHANGE PERMISSION FAILURE@@@'
        return 1

    os.system("rm -rf " + IMAGE_DIR_NAME)

    return 0


def Upload():
    print '@@UPLOAD_STEP START@@'

    retcode = 0

    build_type = str(os.getenv('BULID_TYPE'))
    print build_type
    server_ip = str(os.getenv('FILE_SERVER_IP'))
    print server_ip
    server_user = str(os.getenv('FILE_SERVER_USER'))
    print server_ip
    release_dir = str(os.getenv('FILE_SERVER_RELEASE'))
    print release_dir
    product_name = str(os.getenv('PRODUCT_NAME'))
    print product_name

    if (build_type == 'latest'):
        retcode += UploadLatest(
            server_ip, server_user, release_dir, product_name)
    elif (build_type == 'release'):
        retcode += UploadRelease(
            server_ip, server_user, release_dir, product_name)
    else:
        retcode += UploadNightly(
            server_ip, server_user, release_dir, product_name)

    if retcode:
        print '@@UPLOAD_STEP failures@@'
        return False

    print '@Upload Done@'
    return True

if __name__ == '__main__':
    Upload()
