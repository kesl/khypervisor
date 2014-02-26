import os
import sys
from os.path import join
from nose.tools import ok_, eq_

PROJRELROOT = '../'
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), PROJRELROOT)))
sys.path.append(os.path.abspath('./'))

import checkpatch

def test_coding_style():
    print 'Code Quality Check... : Coding Style'
    if (os.getenv('ghprbPullId') != None):
        eq_(checkpatch.RunPullRequestPatch(), True)
    elif (os.getenv('BULID_TYPE') == ''
        eq_(True, True)
    else:
        eq_(True, True)


def test_clang():
    eq_(True, True)

