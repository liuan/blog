#!/bin/bash

#. /etc/rc.d/init.d/functions

SETCOLOR_SUCCESS="echo -en \\033[1;32m"
SETCOLOR_FAILURE="echo -en \\033[1;31m"
SETCOLOR_NORMAL="echo -en \\033[1;39m"
RES_COL=60
MOVE_TO_COL="echo -en \\033[${RES_COL}G"


OPENSTACK_PROJECTS=(nova cinder glance keystone quantum horizon)
RETVAL=0
#echo ${OPENSTACK_PROJECTS[3]}

#this function is just copy from /etc/rc.d/init.d/functions
# we can just use the echo_success without the below define
echo_success(){
    $MOVE_TO_COL
    echo -n "["
    $SETCOLOR_SUCCESS
    echo -n $"  OK  "
    $SETCOLOR_NORMAL
    echo -n "]"
    echo -ne "\r"
    return 0
}

echo_failure(){
    $MOVE_TO_COL
    echo -n "]"
    $SETCOLOR_FAILURE
    echo -n $"FAILED"
    $SETCOLOR_NORMAL
    echo -n "]"
    echo -ne "\r"
    return 0
}

for project in ${OPENSTACK_PROJECTS[@]}; do
    echo "[$project]"
    for module in `pip-python freeze | grep $project` ;
    do
        echo -n "$module "
        pip-python uninstall $module >/dev/null
        RETVAL=$?
        if [ $RETVAL -eq 0 ];then
            echo_success
        else
            echo_failure
        fi
        echo

    done;

done;

echo -e "\n Done!"
