#!/bin/bash

sudo mount tizen-image/rootfs.img ./mntdir
sudo cp test_binary ./mntdir/root
sudo umount ./mntdir