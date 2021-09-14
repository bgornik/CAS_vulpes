#!/bin/bash

rsync --info=progress2 -r --include='*.cpp' --include='*.hpp' --include='*.txt' --include='*.sh' --include='*/' --exclude='*' ~/cfiles/solve /run/user/1000/gvfs/google-drive:host=gmail.com,user=bgornik
