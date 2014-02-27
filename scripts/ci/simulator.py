import os
import shutil
import subprocess
import sys
import re
import time
import signal

CI_DIR = os.path.dirname(os.path.abspath(__file__))
SCRIPT_DIR = os.path.dirname(CI_DIR)
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
PLATFORM_DIR = os.path.join(ROOT_DIR, 'platform-device')

SIMULATOR_OPTION = str(os.getenv('SIMULATOR_OPTION'))
RTSM_LOG = [' -C motherboard.pl011_uart0.out_file=',
            ' -C motherboard.pl011_uart1.out_file=',
            ' -C motherboard.pl011_uart2.out_file='
]

HYPERVISOR_LOG = 'hypervisor'
GUEST_LOG = 'guest'

def RunSimulator(duration):
    print '@@ Run Simulator @@'

    product = str(os.getenv('TARGET_PRODUCT'))
    hypervisor_image = str(os.getenv('HYPERVISOR_BIN'))
    target_dir = PLATFORM_DIR + "/" + product + "/"
    
    os.chdir(target_dir)
    sys.stdout.flush()
    env = os.environ.copy()
    
    simul = SIMULATOR_OPTION + PLATFORM_DIR + "/" + product + "/"
    simul += hypervisor_image

    log_option = RTSM_LOG[0] + HYPERVISOR_LOG + ".log"
    guest_count = int(os.getenv('GUEST_COUNT'))

    print "guest_count : " + str(guest_count)
    for n in range(0, guest_count):
        if RTSM_LOG[n + 1] == None:
            print '@@@ overflow log path is not valid @@@'
            return False
        log_option += RTSM_LOG[n + 1] + GUEST_LOG + str(n) + ".log"

    simul += log_option

    print simul

    pro = subprocess.Popen(
          ['/bin/bash',
           '-c', simul],
          stdout=subprocess.PIPE,
          preexec_fn=os.setsid,
          cwd=target_dir, env=env)

    print 'wating for %d sec'% (duration)
    time.sleep(duration)

    os.killpg(pro.pid, signal.SIGTERM)
    print '@@ Kill Simulator@@'
    
    return True

if __name__ == '__main__':

    RunSimulator(int(sys.argv[1]))
