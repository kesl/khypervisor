#!/bin/bash

wget https://releases.linaro.org/13.05/android/arndale/boot.tar.bz2
wget https://releases.linaro.org/13.05/android/arndale/system.tar.bz2
wget https://releases.linaro.org/13.05/android/arndale/userdata.tar.bz2

wget --no-check-certificate 'https://docs.google.com/uc?export=download&id=0B8VxwJmrTtCBWE9Del9nOEdCOEk' -O AngryBirds_1.3.2_rio.apk

wget --no-check-certificate 'https://docs.google.com/uc?export=download&id=0B8VxwJmrTtCBSGhKQTBkUWUxQzg' -O com.antutu.ABenchMark-2.9.1-free-a3f3-www.apkhere.com.apk

wget --no-check-certificate 'https://docs.google.com/uc?export=download&id=0B8VxwJmrTtCBeEdLTkp1NnJJdFE' -O Geekbench_3_v3.1.4.apk

tar -xvf userdata.tar.bz2

mv AngryBirds_1.3.2_rio.apk com.antutu.ABenchMark-2.9.1-free-a3f3-www.apkhere.com.apk Geekbench_3_v3.1.4.apk ./data/app

tar -cjvf userdata.tar.bz2 data


