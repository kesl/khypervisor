import collections
import command
import os
import re
import sys
import urllib2

CI_DIR = os.path.dirname(os.path.abspath(__file__))
TRUNK_DIR = os.path.dirname(CI_DIR)
ROOT_DIR = os.path.dirname(TRUNK_DIR)

PATCH_FILE = "pull_request.patch"

def GetPullRequestPatch() :
    url = str(os.getenv('GIT_URL'))
    if url[-1:] == '/':
        url += "pull/"
    else:
        url += "/pull/"
    url += str(os.getenv('ghprbPullId')) + ".patch"
    print url
    u = urllib2.urlopen(url)
    patch_file = open(PATCH_FILE, 'w')
    patch_file.write(u.read())
    patch_file.close()

def CheckPatch(fname, verbose=False):
    fields = ['ok', 'problems', 'errors', 'warnings', 'checks', 'lines',
              'stdout']
    result = collections.namedtuple('CheckPatchResult', fields)
    result.ok = False
    result.errors, result.warning, result.checks = 0, 0, 0
    result.lines = 0
    result.problems = []

    check_cmd = str(ROOT_DIR) + "/scripts/checkpatch.pl"
    result.stdout = command.Output(check_cmd, '--no-tree', fname)
    #pipe = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    #stdout, stderr = pipe.communicate()

    # total: 0 errors, 0 warnings, 159 lines checked
    # or:
    # total: 0 errors, 2 warnings, 7 checks, 473 lines checked
    re_stats = re.compile('total: (\\d+) errors, (\d+) warnings, (\d+)')
    re_stats_full = re.compile('total: (\\d+) errors, (\d+) warnings, (\d+)'
                               ' checks, (\d+)')
    re_ok = re.compile('.*has no obvious style problems')
    re_bad = re.compile('.*has style problems, please review')
    re_error = re.compile('ERROR: (.*)')
    re_warning = re.compile('WARNING: (.*)')
    re_check = re.compile('CHECK: (.*)')
    re_file = re.compile('#\d+: FILE: ([^:]*):(\d+):')

    for line in result.stdout.splitlines():
        if verbose:
            print line

        match = re_stats_full.match(line)
        if not match:
            match = re_stats.match(line)
        if match:
            result.errors = int(match.group(1))
            result.warnings = int(match.group(2))
            if len(match.groups()) == 4:
                result.checks = int(match.group(3))
                result.lines = int(match.group(4))
            else:
                result.lines = int(match.group(3))
        elif re_ok.match(line):
            result.ok = True
        elif re_bad.match(line):
            result.ok = False
        err_match = re_error.match(line)
        warn_match = re_warning.match(line)
        file_match = re_file.match(line)
        check_match = re_check.match(line)

    return result

def RunPullRequestPatch():
    '''Run the checkpatch.pl script'''
    GetPullRequestPatch()
    error_count, warning_count, check_count = 0, 0, 0
    fname = PATCH_FILE

    result = CheckPatch(fname, 1)
    if not result.ok:
        error_count += result.errors
        warning_count += result.warnings
        check_count += result.checks
        print '%d errors, %d warnings, %d checks for %s:' % (result.errors,
                result.warnings, result.checks, fname)
    if error_count or warning_count or check_count:
        str = 'checkpatch.pl found %d error(s), %d warning(s), %d checks(s)'
        print str % (error_count, warning_count, check_count)
        return False
    return True

if __name__ == '__main__':
    print 'checkpatch for pull request'
    os.environ["ghprbPullId"] = "46"
    RunPullRequestPatch()
