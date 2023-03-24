#!/bin/bash

sudo mount tizen-image/rootfs.img ./mntdir
sudo cp project1_test ./mntdir/root
sudo umount ./mntdir