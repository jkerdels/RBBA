#!/bin/bash

# set name of openscad executable
os=openscad-ai

$os -q -o ./stl/calibration.stl -D render_ctl=rc_calibration rbba.scad
$os -q -o ./stl/gear.stl -D render_ctl=rc_gear rbba.scad
$os -q -o ./stl/gear_base.stl -D render_ctl=rc_gear_base rbba.scad
$os -q -o ./stl/gear_plug.stl -D render_ctl=rc_gear_plug rbba.scad
$os -q -o ./stl/drive_gear.stl -D render_ctl=rc_drive_gear rbba.scad
$os -q -o ./stl/motormount.stl -D render_ctl=rc_motormount rbba.scad
$os -q -o ./stl/bearingmount.stl -D render_ctl=rc_bearingmount rbba.scad
$os -q -o ./stl/bagmount.stl -D render_ctl=rc_bagmount rbba.scad
$os -q -o ./stl/bagmount_spacer.stl -D render_ctl=rc_bagmount_spacer rbba.scad
$os -q -o ./stl/leg.stl -D render_ctl=rc_leg rbba.scad
$os -q -o ./stl/switchmount.stl -D render_ctl=rc_switchmount rbba.scad
$os -q -o ./stl/motor_gear_1.stl -D render_ctl=rc_motor_gear1 rbba.scad
$os -q -o ./stl/motor_gear_2.stl -D render_ctl=rc_motor_gear2 rbba.scad
$os -q -o ./stl/centermount.stl -D render_ctl=rc_centermount rbba.scad
$os -q -o ./stl/wormgearmount.stl -D render_ctl=rc_wormgearmount rbba.scad
