/* rbba.scad 

    3D geometry of a 3D-printable breathing assistance system that utilizes
    a standard resuscitator.
    
    The different parts can be selected via the render_ctl variable by 
    setting it to the available render_ctl options (see below). Instead,
    all parts can be rendered via the render_stl.sh script.
    
    The 3D geometry defined here uses the OpenSCAD-Library "Getriebe.scad"
    by Dr Jörg Janssen in Version 2.3 . The library is contained in the
    3rd-party folder and should be put into the library-folder of your
    OpenSCAD installation. You can find the library folder by clicking on
    "File->Show Library Folder..." in OpenSCAD.

    The device consists of three major parts: two identical outer parts
    and a center piece that may has to be adapted to the particular 
    requirements of the geared motor used.
    
    To adjust the model to your printer, the variable "hole_extra" allows
    you to tweak the size of negative shapes in the model. There is a 
    calibration piece that can be selected with render_ctl = rc_calibration;
    You should be able to put a 17mm bearing snugly into the hole of the
    calibration piece. Please adjust the hole_extra variable until that is
    the case.
    
    The bearings used here are 17mm outside diameter, 6mm inner diameter, 
    and 6mm height. If you want to use bearings with slightly different
    dimensions you can adjust the variables bearing_outer, bearing_inner,
    and bearing_height.
*/
use <Getriebe.scad>;

hole_extra = 0.3;

pr_diam   = 250;
pr_zaehne = 96;
pr_modul  = pr_diam/pr_zaehne;
pr_breite = 15;
pr_loch   = 17;

bag_dia = 130;

hole_drive = 5;
notch_drive = 4;
teeth_drive = 12;
diam_drive = pr_modul * teeth_drive;

bearing_outer  = 17;
bearing_inner  = 6;
bearing_height = 6;

bearing_offset = 0.5;

//gear_angle = 27;
gear_angle = 0;

motor_mount_width = 35.6;
motor_mount_extra = 2;

// render_ctl options: 
rc_all             = "all";
rc_calibration     = "calibration";
rc_gear            = "gear";
rc_gear_base       = "gear_base";
rc_gear_plug       = "gear_plug";
rc_drive_gear      = "drive_gear";
rc_motormount      = "motormount";
rc_bearingmount    = "bearingmount";
rc_bagmount        = "bagmount";
rc_bagmount_spacer = "bagmount_spacer";
rc_leg             = "leg";
rc_switchmount     = "switchmount";
rc_motor_gear1     = "motor_gear1";
rc_motor_gear2     = "motor_gear2";
rc_centermount     = "centermount";
rc_wormgearmount   = "wormgearmount";

render_ctl = rc_all;

module bearing() {
    color("silver")
    difference(){
        cylinder(d=bearing_outer,h=bearing_height,$fn=72);   
        translate([0,0,-1]) 
        cylinder(d=bearing_inner,h=bearing_height+2,$fn=30);
    }
}

module bearings() {    
    translate([diam_drive/2+10,0,0])
    translate([pr_diam/2-10,0,0])
    rotate([0,0,0])
    translate([-pr_diam/2+10,0,0])
    translate([-bearing_height/2,0,-bearing_outer/2])
    rotate([90,0,90])
    bearing();

    translate([pr_diam/2+diam_drive/2,0,pr_breite+bearing_outer/2])
    rotate([0,0,13.668])
    translate([-pr_diam/2,0,0])
    translate([6.5,0,0])
    rotate([90,0,90])
    bearing();

    mirror([1,0,0])
    translate([pr_diam/2+diam_drive/2,0,pr_breite+bearing_outer/2])
    rotate([0,0,13.668])
    translate([-pr_diam/2,0,0])
    translate([6.5,0,0])
    rotate([90,0,90])
    bearing();

    rotate([0,0,180]){
        translate([pr_diam/2+diam_drive/2,0,pr_breite+bearing_outer/2])
        rotate([0,0,13.668])
        translate([-pr_diam/2,0,0])
        translate([6.5,0,0])
        rotate([90,0,90])
        bearing();

        mirror([1,0,0])
        translate([pr_diam/2+diam_drive/2,0,pr_breite+bearing_outer/2])
        rotate([0,0,13.668])
        translate([-pr_diam/2,0,0])
        translate([6.5,0,0])
        rotate([90,0,90])
        bearing();
    }

    /*
    translate([diam_drive/2+10,0,0])
    translate([pr_diam/2-10,0,0])
    rotate([0,0,-30])
    translate([-pr_diam/2+10,0,0])
    translate([-bearing_height/2,0,-bearing_outer/2])
    rotate([90,0,90])
    bearing();

    translate([diam_drive/2+10,0,0])
    translate([pr_diam/2-10,0,0])
    rotate([0,0,30])
    translate([-pr_diam/2+10,0,0])
    translate([-bearing_height/2,0,-bearing_outer/2])
    rotate([90,0,90])
    bearing();
    */

}

module bag() {
    sphere(d=bag_dia,$fn=75);

    translate([0,5/2-100,0])
    rotate([90,0,0])
    cylinder(d=45,h=5);

    translate([0,5/2+90,0])
    rotate([90,0,0])
    cylinder(d=70,h=5);
}


module pusher_knob() {
    translate([0,0,0])
    rotate([0,90,0])
    difference(){
        scale([1,0.5,0.5])
        sphere(d=60,$fn=75);
        translate([-30,-30,0])
        cube([60,60,30]);        
    }    
}

module push_rod(){
    translate([0,-15/2,-30/2])
    cube([30,15,30]);

    translate([15,-15/2,-30-30/2])
    cube([15,15,30]);

    translate([15,-15/2,-85-30-30/2])
    cube([15,15,85]);
}

module drive_gear() {
        
    render()
    difference(){
        union(){
            stirnrad (pr_modul, teeth_drive, pr_breite, hole_drive+hole_extra, optimiert=false);

            rotate([0,0,180/teeth_drive])
            translate([-(hole_drive+hole_extra)/2,(hole_drive+hole_extra)/2-(hole_drive-notch_drive),0])
            cube([hole_drive+hole_extra,hole_drive+hole_extra/2,pr_breite]);
        }
        translate([0,0,-2.5])
        rotate([0,0,180/teeth_drive])
        translate([0,0,pr_breite/2]){
            rotate([270,0,0])
            cylinder(d=3+hole_extra,h=40,$fn=30);
            translate([-(5.5+hole_extra)/2,diam_drive/6,-(1.5+hole_extra)-1.8])
            cube([5.5+hole_extra,2.5+hole_extra,pr_breite/2+1.8+hole_extra+10]);
        }
    }
}

module drive_gear2() {
        
    render()
    difference(){
        union(){
            stirnrad (pr_modul, teeth_drive, pr_breite, 6+hole_extra, optimiert=false);
        }
        translate([0,0,-1])
        for(i = [0:60:359])
        rotate([0,0,i])
        translate([-(6+hole_extra-0.25)/2,-(10+hole_extra+0.2)/2,0])
        cube([6+hole_extra-0.25,10+hole_extra+0.2,5+1]);
    }
    
}

module gear_plug() {

    translate([0,0,pr_breite-1.5])
    difference(){
        cylinder(d=pr_loch,h=1.5,$fn=50);
        translate([0,0,-1])
        cylinder(d=pr_loch-4.5+hole_extra,h=3.5,$fn=50);
    }

    translate([0,0,pr_breite])
    difference(){
        cylinder(d=pr_loch*2,h=1.5,$fn=50);
        translate([0,0,-1])
        cylinder(d=pr_loch-4.5+hole_extra,h=3.5,$fn=50);
    }
    
}

module cake(angle){
    color("blue")
    difference(){
        cylinder(d=pr_diam-30,h=pr_breite+2,$fn=75);
        translate([-pr_diam/2,0,-1])
        cube([pr_diam,pr_diam/2,pr_breite+4]);
        rotate([0,0,angle])
        translate([-pr_diam/2,0,-1])
        cube([pr_diam,pr_diam/2,pr_breite+4]);
        translate([0,0,-1])
        cylinder(d=2*pr_loch+30,h=pr_breite+4,$fn=75);
    }
}

module gear(offset = false) {
    render()
    difference(){
        intersection(){
            rotate([0,0,offset ? 180/pr_zaehne : 0])
            stirnrad (pr_modul, pr_zaehne, pr_breite, pr_loch+hole_extra, optimiert=false);

            translate([0,0,-1])
            linear_extrude(height=pr_breite*2)
            minkowski(){
                union(){
                    rotate([0,0,115])
                    square([pr_diam/2+10,pr_diam/2+10]);
                    rotate([0,0,135.3])
                    square([pr_diam/2+10,pr_diam/2+10]);
                }
                circle(d=pr_loch*2);
            }
        }    
        translate([0,0,-1])
        rotate([0,0,135.3+90])
        cake(70);
    }
    
    rotate([0,0,170])
    translate([10,-15/2,0])
    cube([pr_diam/2-20,15,pr_breite]);
    
    difference(){
        cylinder(d=pr_loch+1,h=1.5,$fn=50);
        translate([0,0,-1])
        cylinder(d=pr_loch-4.5+hole_extra,h=3.5,$fn=50);
    }

    difference(){
        intersection(){
            difference(){
                translate([0,0,0])
                cylinder(d1=pr_diam+5+15,d2=pr_diam+5+15+diam_drive/2+13,h=bag_dia/2+5+20+pr_breite,$fn=75);

                translate([0,0,-1])
                cylinder(d1=pr_diam+5-30+7.5,d2=pr_diam+5-30+7.5+diam_drive/2+20,h=bag_dia/2+5+20+pr_breite+2,$fn=75);
                
                rotate([0,0,180-36.8])
                translate([-(pr_diam+50+diam_drive)/2,0,-1])
                cube([pr_diam+50+diam_drive,pr_diam/2+50+diam_drive,bag_dia/2+5+20+pr_breite+2]);

                rotate([0,0,287])
                translate([-(pr_diam+50+diam_drive)/2,0,-1])
                cube([pr_diam+50+diam_drive,pr_diam/2+50+diam_drive,bag_dia/2+5+20+pr_breite+2]);
            }    
            rotate([0,0,16.5])
            scale([0.7,1,1])
            translate([-105,0,0])
            rotate([270,0,0])
            cylinder(d=210,h=150,$fn=75);
        }
        rotate([0,0,43])
        scale([0.5,1,1.3])
        translate([-50,0,0])
        rotate([270,0,0])
        cylinder(d=100,h=150,$fn=75);
    }

    translate([0,0,bag_dia/2+10+5+pr_breite])
    rotate([0,0,-37])
    translate([-pr_diam/2-diam_drive/2,0,0])
    rotate([0,0,90])
    pusher_knob();
}




module micro_switch(helper = false) {

    if (helper == false) {
        difference(){
            cube([29,16,10]);
            translate([1.5+1.5,1.5+1.5,-1])
            cylinder(d=3,h=12,$fn=18);
            translate([28.2-3,16-3,-1])
            cylinder(d=3,h=12,$fn=18);
        }
        
        translate([29,3.8,1.25])
        cube([11,0.8,6.5]);

        translate([29,9.4,1.25])
        cube([11,0.8,6.5]);
        
        translate([15.3,-3,1.25])
        cube([0.8,3,6.5]);
        
        translate([15.3,-3.8,1.25])
        cube([10,0.8,6.5]);
        
        translate([15.5,16,3.25])
        cube([0.5,2,4.5]);

        translate([15.5,18,3.25])
        rotate([0,0,7])
        translate([-27.3,0,0])
        union(){
            cube([27.3,0.5,4.5]);
            translate([1.5,3.6,0.5])
            cylinder(d=5.4,h=3.9,$fn=20);
        }
    } else {
        translate([1.5+1.5,1.5+1.5,-1-20])
        cylinder(d=2.5+hole_extra,h=52,$fn=18);
        translate([28.2-3,16-3,-1-20])
        cylinder(d=2.5+hole_extra,h=52,$fn=18);
    }
}

module triangle(size) {
    
    linear_extrude(height=size[2])
    polygon([[0,0],[size[0],0],[0,size[1]]]);    
    
}

module side_holder() {
    
    translate([-diam_drive/2-5,0,0]) {
        translate([-5,0,0])
        rotate([90,0,90])
        union(){
            difference(){
                cylinder(d=85,h=10,$fn=75);
                translate([0,0,-1])
                cylinder(d=70,h=12,$fn=75);
                translate([-45,0,-1])
                cube([90,45,12]);
            }        
            translate([35,0,0])
            difference(){
                cube([7.5,15,10]);
                translate([-1,5,2.5])
                cube([10,5,5]);
            }
            translate([-35-7.5,0,0])
            difference(){
                cube([7.5,15,10]);
                translate([-1,5,2.5])
                cube([10,5,5]);
            }
        }
        difference(){
            rotate([0,0,90])
            translate([-25,-5,-33-40])
            cube([50,10,40]);
            translate([-5,0,0])
            rotate([90,0,90])
            translate([0,0,-1])
            cylinder(d=70,h=12,$fn=75);
        }
    }
    difference(){
        rotate([0,0,90])
        translate([-25,-55,-33-40])
        cube([50,50+diam_drive/2+5,5]);
        translate([pr_diam/2-105,0,-33-40-1])
        cylinder(d=6+hole_extra,h=7,$fn=30);
        translate([pr_diam/2+30-10+15/2-105,-50/5,-33-40-1])
        cylinder(d=5+hole_extra,h=10,$fn=30);
        translate([pr_diam/2+30-10+15/2-105,+50/5,-33-40-1])
        cylinder(d=5+hole_extra,h=10,$fn=30);
    }
    
    translate([-16,-15,-68])
    rotate([90,0,0])
    triangle([55,30,10]);

    translate([-16,+25,-68])
    rotate([90,0,0])
    triangle([55,30,10]);
}

module side_holder_spacer()
{
    difference(){
        cylinder(d=10,h=1.5+7,$fn=30);
        translate([0,0,-1])
        cylinder(d=6+hole_extra,h=1.5+7+2,$fn=30);
    }
}

module gear_post(){
    difference(){
        union(){
            cylinder(d1=60,d2=26,h=30+1.5,$fn=75);
            translate([0,0,30])
            cylinder(d=10,h=3.5+1.5+1.5,$fn=30);
        }
        translate([0,0,-1])
        cylinder(d=5+hole_extra,h=40,$fn=30);
    }
}

module center_base() {
    
    motor_mount_delta = motor_mount_width/2-diam_drive/2;
    
    translate([motor_mount_delta,-45/2,0])
    cube([pr_diam/2+30+5-motor_mount_delta-60,45,15]);
    translate([motor_mount_delta+(pr_diam/2+30+5-motor_mount_delta-60),-60/2,0])
    cube([60,60,15]);
    difference(){
        translate([pr_diam/2+30-10,-60/2,0])
        cube([15,60,40+27]);
        translate([pr_diam/2+30-10-1,-50/2,40+27-5])
        cube([15+2,50,6]);
        translate([pr_diam/2+30-10+15/2,-50/5,23+5])
        cylinder(d=4.2+hole_extra,h=40,$fn=30);
        translate([pr_diam/2+30-10+15/2,+50/5,23+5])
        cylinder(d=4.2+hole_extra,h=40,$fn=30);
    }
    difference(){
        translate([motor_mount_delta,-80/2,0])
        cube([15,80,15]);
        translate([motor_mount_delta,-3*80/8,0])
        union(){
            translate([-1,0,15/2])
            rotate([0,90,0])
            cylinder(d=6+hole_extra,h=17,$fn=30);        
            /*
            translate([15-4,0,15/2])
            rotate([0,90,0])
            cylinder(d=10,h=5,$fn=45);
            */
        }        
        translate([motor_mount_delta,3*80/8,0])
        union(){
            translate([-1,0,15/2])
            rotate([0,90,0])
            cylinder(d=6+hole_extra,h=17,$fn=30);        
            /*
            translate([15-4,0,15/2])
            rotate([0,90,0])
            cylinder(d=10,h=5,$fn=45);
            */
        }        
    }

    translate([pr_diam/2,0,5])
    gear_post();
    
    difference(){
        union(){
            translate([diam_drive/2-1,0,-bearing_outer/2+40])
            rotate([0,90,0])
            cylinder(d=bearing_outer-2,h=25,$fn=50);
            
            translate([diam_drive/2-1,-(bearing_outer-2)/2,0])
            cube([25,bearing_outer-2,-bearing_outer/2+40]);
        }

        translate([diam_drive/2-2,0,-bearing_outer/2+40+bearing_offset])
        rotate([0,90,0])
        cylinder(d=4.2+hole_extra,h=20,$fn=50);
    }
    
    translate([motor_mount_delta+(pr_diam/2+30+5-motor_mount_delta-60)-15,-60/2,0])
    difference(){
        cube([15,10,2]);
        translate([2,2,-1])
        cube([3,6,4]);
        translate([10,2,-1])
        cube([3,6,4]);
    }
    
    translate([motor_mount_delta+(pr_diam/2+30+5-motor_mount_delta),0,0])
    difference(){
        cylinder(d=15,h=2,$fn=50);
        translate([0,0,-1])
        cylinder(d=10,h=4,$fn=50);
    }
}

module motor_plate() {
    difference(){
        translate([0,-60-60,-5])
        cube([60,90,5]);

        translate([pr_diam/2,0,0])
        rotate([0,0,40])
        translate([-pr_diam/2,0,0])
        translate([-diam_drive/2,0,0])
        rotate([0,0,-40-0])
        translate([0,0,33.5-3])
        motor_mount(true);
    }
    translate([0,-60,0])
    mirror([0,1,0])
    difference(){
        translate([0,-60/2,-5])
        cube([60,15,20]);    
        union(){
            translate([60/2,-60/2-1,15/2])
            rotate([0,90,90])
            cylinder(d=5,h=17,$fn=30);     
            /*
            translate([60/2,-60/2+15-4,15/2])
            rotate([0,90,90])
            cylinder(d=10,h=5,$fn=45);
            */
        }        
        translate([60/3,0,0])
        union(){
            translate([60/2,-60/2-1,15/2])
            rotate([0,90,90])
            cylinder(d=5,h=17,$fn=30);
            /*
            translate([60/2,-60/2+15-4,15/2])
            rotate([0,90,90])
            cylinder(d=10,h=5,$fn=45);
            */
        }        
    }
}

module motor2(helper = false) {
    
    translate([-2.88,-5,0])
    if (helper == false) {
        color("green")
        translate([2.88,5,-2.5])
        cylinder(d=10,h=2.5,$fn=75);    
        translate([2.88,5,0])
        cylinder(d=5,h=10,$fn=75);

        difference(){
            translate([0,0,-74-2.5])
            cylinder(d=30,h=74,$fn=75);
            color("blue")
            union(){
                translate([25/2,0,-2.5-5])
                cylinder(d=2.5+hole_extra,h=6,$fn=18);
                translate([-25/2,0,-2.5-5])
                cylinder(d=2.5+hole_extra,h=6,$fn=18);
            }

            color("blue")
            rotate([0,0,30])
            translate([25/2,0,-2.5-5])
            cylinder(d=2.5+hole_extra,h=6,$fn=18);

            rotate([0,0,-30])
            color("blue")
            union(){
                translate([25/2,0,-2.5-5])
                cylinder(d=2.5+hole_extra,h=6,$fn=18);
                translate([-25/2,0,-2.5-5])
                cylinder(d=2.5+hole_extra,h=6,$fn=18);
            }

            rotate([0,0,-60])
            color("blue")
            union(){
                translate([25/2,0,-2.5-5])
                cylinder(d=2.5+hole_extra,h=6,$fn=18);
                translate([-25/2,0,-2.5-5])
                cylinder(d=2.5+hole_extra,h=6,$fn=18);
            }

            color("blue")
            rotate([0,0,-90])
            translate([25/2,0,-2.5-5])
            cylinder(d=2.5+hole_extra,h=6,$fn=18);
        }        
    } else {
        translate([2.88,5,-2.5])
        cylinder(d=10.5+hole_extra,h=25,$fn=75);    

        //translate([0,0,-74-2.5])
        //cylinder(d=30,h=74,$fn=75);
        
        rotate([0,0,30])
        translate([25/2,0,-2.5])
        cylinder(d=3+hole_extra,h=25,$fn=18);

        rotate([0,0,150])
        translate([25/2,0,-2.5])
        cylinder(d=3+hole_extra,h=25,$fn=18);

        rotate([0,0,-90])
        translate([25/2,0,-2.5])
        cylinder(d=3+hole_extra,h=25,$fn=18);
    }
        
}

module motor_mount2() {

    wall_t = 1.8;
    top_t  = 2;

    mheight = 52-3;

    translate([0,0,-55+3])
    difference(){
        cylinder(d=bearing_outer+8,h=mheight,$fn=72);
        translate([0,0,-1])
        cylinder(d=bearing_outer+hole_extra,h=bearing_height+1,$fn=72);
        translate([0,0,-1])
        cylinder(d=11+hole_extra,h=mheight+2,$fn=72);
        translate([0,0,mheight-bearing_height])
        cylinder(d=bearing_outer+hole_extra,h=bearing_height+1,$fn=72);
    }
    
    difference(){
        translate([-(32+2*wall_t+motor_mount_extra)/2,-80/2,-40])
        cube([32+2*wall_t+motor_mount_extra,80,15]);
        
        translate([0,0,-55+3])
        translate([0,0,-1])
        cylinder(d=11+hole_extra,h=mheight+2,$fn=72);
        
        translate([-40/2,3*80/8,-40+15/2])
        rotate([0,90,0])
        cylinder(d=6+hole_extra,h=40,$fn=30);

        translate([-40/2,-3*80/8,-40+15/2])
        rotate([0,90,0])
        cylinder(d=6+hole_extra,h=40,$fn=30);

        translate([0,-3*80/8,-40-1])
        translate([-(20+hole_extra)/2,-(12+hole_extra)/2-5,0])
        cube([20+hole_extra,12+hole_extra+5,30]);

        rotate([0,0,180])
        translate([0,-3*80/8,-40-1])
        translate([-(20+hole_extra)/2,-(12+hole_extra)/2-5,0])
        cube([20+hole_extra,12+hole_extra+5,30]);
    }
        
}

module motor_mount3() {

    wall_t = 1.8;
    top_t  = 2;

    rotate([0,0,30])
    difference(){
        translate([-2.88,-5,-25])
        cylinder(d=32+2*wall_t,h=20+top_t,$fn=75);

        translate([-2.88,-5,-40-5-1])
        cylinder(d=32,h=40+1,$fn=75);
        
        translate([0,0,-20])
        motor2(true);
    }
    
    difference(){
        translate([-2.6,-12.75,-25])
        cube([32+2*wall_t+motor_mount_extra,17.5,16.2]);

        rotate([0,0,30])
        translate([-2.88,-5,-40-5-1])
        cylinder(d=32,h=40+1,$fn=75);

        translate([27.5,-3*80/8+25-hole_extra,-45])
        cylinder(d=6+hole_extra,h=40,$fn=30);
    }
    
        
}

module top_bearings() {

    color("orange")
    difference(){
        translate([0,-3*80/8,-40])
        translate([-20/2,-12/2-4,0])
        cube([20,12+4,40+pr_breite+bearing_outer-1]);

        translate([-40/2,-3*80/8,-40+15/2])
        rotate([0,90,0])
        cylinder(d=6+hole_extra,h=40,$fn=30);

        
        translate([pr_diam/2+diam_drive/2,0,pr_breite+bearing_outer/2-bearing_offset])
        rotate([0,0,13.668])
        translate([-pr_diam/2,0,0])
        rotate([0,90,0])
        translate([0,0,-20])
        cylinder(d=4.2+hole_extra,h=26,$fn=50);
        
    }
    
    difference(){
        color("green")
        translate([pr_diam/2+diam_drive/2,0,pr_breite+3])
        rotate([0,0,13.668])
        translate([-pr_diam/2,0,0])
        rotate([0,0,180])
        translate([-5,-(12+4)/2,0])
        cube([16.38,12+4.,bearing_outer-2-2]);
        
        translate([pr_diam/2+diam_drive/2,0,pr_breite+bearing_outer/2-bearing_offset])
        rotate([0,0,13.668])
        translate([-pr_diam/2,0,0])
        rotate([0,90,0])
        translate([0,0,-20])
        cylinder(d=4.2+hole_extra,h=26,$fn=50);
    }

    mirror([1,0,0])
    difference(){
        color("green")
        translate([pr_diam/2+diam_drive/2,0,pr_breite+3])
        rotate([0,0,13.668])
        translate([-pr_diam/2,0,0])
        rotate([0,0,180])
        translate([-5,-(12+4)/2,0])
        cube([16.38,12+4.,bearing_outer-2-2]);
        
        translate([pr_diam/2+diam_drive/2,0,pr_breite+bearing_outer/2-bearing_offset])
        rotate([0,0,13.668])
        translate([-pr_diam/2,0,0])
        rotate([0,90,0])
        translate([0,0,-20])
        cylinder(d=4.2+hole_extra,h=26,$fn=50);
    }
    
}

module calibration() {
    
    difference(){
        translate([-25/2,-25/2,0])
        cube([25,25,10]);
        translate([0,0,-1])
        cylinder(d=10+hole_extra,h=10-bearing_height+2,$fn=60);
        translate([0,0,10-bearing_height])
        cylinder(d=bearing_outer+hole_extra,h=bearing_height+1,$fn=60);
    }
    
}

module leg() {
    motor_mount_delta = motor_mount_width/2-diam_drive/2;
    
    multmatrix(m = [ [1, 0, 0, motor_mount_delta+(pr_diam/2+30+5-motor_mount_delta-60)],
                     [0, 1, 0.7, -60/2+10],
                     [0, 0, 1, 0],
                     [0, 0, 0, 1]
                  ])
    rotate([180,0,0])
    cube([30,15,45]);
    translate([motor_mount_delta+(pr_diam/2+30+5-motor_mount_delta-60),-60/2-5,0])
    cube([30,5,10]);    
}

module wormgearmount() {
    motor_mount_delta = motor_mount_width/2-diam_drive/2;
       
    difference(){
        union(){
            translate([motor_mount_delta+15,-80/2,0])
            cube([15,17.5,15]);
            multmatrix(m = [ [1, 0, 0, motor_mount_delta+15],
                             [0, 1, 0.3, -80/2+17.5],
                             [0, 0, 1, 0],
                             [0, 0, 0, 1]
                          ])
            rotate([180,0,0])
            cube([15,17.5,12]);
            translate([1+3,-(36*1.25)/2-(10*1.25)-0.25,-20])
            rotate([0,90,0])
            cylinder(d=bearing_outer+8,h=28.175,$fn=72);
        }
        translate([motor_mount_delta+15,-3*80/8,0])
        translate([-1,0,15/2])
        rotate([0,90,0])
        cylinder(d=6+hole_extra,h=17,$fn=30);        
        translate([1+3,-(36*1.25)/2-(10*1.25)-0.25,-20])
        rotate([0,90,0]){
            cylinder(d=11,h=28.175+2,$fn=72);
            translate([0,0,-1])
            cylinder(d=bearing_outer+hole_extra,h=bearing_height+1,$fn=72);
            translate([0,0,28.175-bearing_height])
            cylinder(d=bearing_outer+hole_extra,h=bearing_height+1,$fn=72);
        }
    }        
}

if (render_ctl == "calibration") 
{
    calibration();
}

if (render_ctl == "all") 
{
    bearings();
}


if (render_ctl == "all") 
{
    /*
    color("white",0.5)
    translate([0,0,bag_dia/2+10+5+pr_breite])
    rotate([0,0,90])
    bag();
    */
}

if ((render_ctl == "all") ||
    (render_ctl == "gear"))
{
    translate([pr_diam/2+diam_drive/2,0,0])
    rotate([0,0,gear_angle])
    gear();
}

if ((render_ctl == "all") ||
    (render_ctl == "gear_plug"))
{
    translate([pr_diam/2+diam_drive/2,0,0])
    gear_plug();
}

if (render_ctl == "all") 
{
    rotate([0,0,180])
    translate([pr_diam/2+diam_drive/2,0,0])
    rotate([0,0,gear_angle])
    gear();
}

if (render_ctl == "all") 
{
    rotate([0,0,180])
    translate([pr_diam/2+diam_drive/2,0,0])
    gear_plug();
}

if ((render_ctl == "all") ||
    (render_ctl == "drive_gear"))
{
    rotate([0,0,-180/teeth_drive])
    drive_gear2();
}

if (render_ctl == "all") 
{
    rotate([0,0,90])
    translate([(36*1.25)/2+(10*1.25)+0.25,-24,-60])
    rotate([270,-90,0])
    rotate([0,0,30])
    translate([0,0,-2.5])
    motor2();
}


if ((render_ctl == "all") ||
    (render_ctl == "centermount"))
{
    motor_mount2();
}

if ((render_ctl == "all") ||
    (render_ctl == "motormount"))
{
    rotate([0,0,90])
    translate([(36*1.25)/2+(10*1.25)+0.25,-24,-60])
    rotate([270,-90,0])
    motor_mount3();    
}

if ((render_ctl == "all") ||
    (render_ctl == "wormgearmount"))
{
    rotate([0,0,180])
    translate([diam_drive/2,0,-40])
    wormgearmount();
}

if ((render_ctl == "all") ||
    (render_ctl == "motor_gear1") || 
    (render_ctl == "motor_gear2"))
{    
    // version 1
    /*
    mdl   = 1.25;
    zzahl = 36;
    gzahl = 7;
    lng   = 40;
    offstx = 0;
    offsty = 0;
    offbore = 0;
    */

    // version 2
    
    mdl   = 2.5;
    zzahl = 18;
    gzahl = 4;
    lng   = 40;    
    offstx = 0;
    offsty = 2.75;
    offbore = 3;
    
    
    
    if (render_ctl == "all") {
        rotate([0,0,90])
        translate([(36*1.25)/2,0,-60])
        schneckenradsatz(
            modul=mdl, 
            zahnzahl=zzahl, 
            gangzahl=gzahl, 
            breite=10, 
            laenge=lng, 
            bohrung_schnecke=hole_drive, 
            bohrung_rad=6, 
            eingriffswinkel=20, 
            steigungswinkel=20, 
            optimiert=false, 
            zusammen_gebaut=true);
    } else {
        if (render_ctl == "motor_gear1") {
            difference(){
                stirnrad(mdl, zzahl, 10, 6, eingriffswinkel=-20, 
                         schraegungswinkel=-20, optimiert=false);
                translate([0,0,5])
                for(i = [0:60:359])
                rotate([0,0,i])
                translate([-(6+hole_extra-0.25)/2,-(10+hole_extra+0.2)/2,0])
                cube([6+hole_extra-0.25,10+hole_extra+0.2,5+1]);
            }
        } else
        if (render_ctl == "motor_gear2") {
            difference(){
                union(){
                    schnecke(mdl, gzahl, lng, hole_drive+hole_extra, 
                             eingriffswinkel=20, steigungswinkel=20, 
                             zusammen_gebaut=true);
                    rotate([0,0,-35])
                    translate([-6/2,notch_drive-hole_drive/2,0])
                    cube([6,5,10]);        
                }
                translate([0,0,5])
                rotate([0,0,-35])
                rotate([270,0,0])
                cylinder(d=2.5+hole_extra,h=20,$fn=30);
            }
        }
    }
}

if ((render_ctl == "all") ||
    (render_ctl == "gear_base"))
{
    translate([diam_drive/2,0,-40])
    center_base();
}

if ((render_ctl == "all") ||
    (render_ctl == "leg"))
{
    translate([diam_drive/2,0,-40])
    leg();
}

if (render_ctl == "all") 
{
    mirror([0,1,0])
    translate([diam_drive/2,0,-40])
    leg();
}

if ((render_ctl == "all") ||
    (render_ctl == "bagmount"))
{
    translate([105+diam_drive/2,0,bag_dia/2+10+5+pr_breite])
    side_holder();
}

if ((render_ctl == "all") ||
    (render_ctl == "bagmount_spacer"))
{
    translate([pr_diam/2+diam_drive/2,0,pr_breite-1.5])
    side_holder_spacer();
}


if ((render_ctl == "all") ||
    (render_ctl == "bearingmount"))
{
    top_bearings();
}


if (render_ctl == "all") 
{
    rotate([0,0,180]){
        translate([diam_drive/2,0,-40])
        center_base();

        translate([diam_drive/2,0,-40])
        leg();

        mirror([0,1,0])
        translate([diam_drive/2,0,-40])
        leg();

        translate([105+diam_drive/2,0,bag_dia/2+10+5+pr_breite])
        side_holder();

        translate([pr_diam/2+diam_drive/2,0,pr_breite-1.5])
        side_holder_spacer();

        top_bearings();
    }
}


if (render_ctl == "all") 
{
    translate([pr_diam/2+7.5+diam_drive/2,75,12.5])
    color("green")
    rotate([0,180,115])
    micro_switch();


    translate([pr_diam/2+16+diam_drive/2,-75,2.5])
    color("green")
    rotate([0,0,73])
    micro_switch();
}

module switch_holder() {

    difference(){
        translate([pr_diam/2+7.5+diam_drive/2,75,12.5])
        rotate([0,0,115])
        translate([-31,-1,0])
        cube([32,18,19.5]);

        translate([pr_diam/2+7.5+diam_drive/2,75,12.5])
        rotate([0,180,115])
        micro_switch(true);
    }

    difference(){
        translate([pr_diam/2+16+diam_drive/2,-75,2.5])
        rotate([0,0,73])
        translate([-1,-1,10])
        cube([32,18,19.5]);

        translate([pr_diam/2+16+diam_drive/2,-75,2.5])
        rotate([0,0,73])
        micro_switch(true);
    }
    
    difference(){
        translate([105+diam_drive/2+40,-60/2,bag_dia/2-38])
        cube([15,60,5]);
        
        translate([105+diam_drive/2,0,bag_dia/2-41]){
            translate([pr_diam/2+30-10+15/2-105,-50/5,0])
            cylinder(d=5+hole_extra,h=10,$fn=30);
            translate([pr_diam/2+30-10+15/2-105,+50/5,0])
            cylinder(d=5+hole_extra,h=10,$fn=30);
        }
    }

    multmatrix(m = [ [1, -0.55, 0, 105+diam_drive/2+40],
                     [0, 1, 0, 60/2],
                     [0, 0, 1, bag_dia/2-38],
                     [0, 0, 0, 1]
                  ])    
    cube([15,35,5]);

    multmatrix(m = [ [1, 0.3, 0, 105+diam_drive/2+40+15],
                     [0, 1, 0, -60/2],
                     [0, 0, 1, bag_dia/2-38],
                     [0, 0, 0, 1]
                  ])    
    rotate([0,0,180])
    cube([15,38,5]);
    
}


if ((render_ctl == "all") ||
    (render_ctl == "switchmount"))
{
    switch_holder();
}



