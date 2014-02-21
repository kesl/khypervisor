import os
import sys
from os.path import join
from nose.tools import ok_, eq_

PROJRELROOT = '../'
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), PROJRELROOT)))
sys.path.append(os.path.abspath('./'))

import extract
import archive
import upload

def test_extract():
    print 'Extract Release Files... '
    eq_(extract.ExtractImage(), True)

def test_archive():
    print 'Archive Release Files... '
    eq_(archive.ArchiveImage(), True)

def test_upload():
    print 'Upload Release Files... '
    if (os.getenv('BULID_TYPE') != None):
        eq_(upload.Upload(), True)
    else:
        eq_(True, True)
