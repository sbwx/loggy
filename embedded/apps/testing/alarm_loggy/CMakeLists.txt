# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
set(BOARD nucleo_l432kc)
set(CONF_FILE prj.conf)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(loggy)

target_sources(app PRIVATE src/main.c)