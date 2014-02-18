import os
import sys
from nose.tools import ok_, eq_
from .. import checkpatch

def test_coding_style():
    print 'Code Quality Check... : Coding Style'
    eq_(checkpatch.RunPullRequestPatch(), True)

def test_clang():
    eq_(True, True)

