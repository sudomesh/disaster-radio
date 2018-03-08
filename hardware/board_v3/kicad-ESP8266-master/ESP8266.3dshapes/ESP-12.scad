fragments=20;
module halfPad() {
    color("Gold", a=1) {
        difference() {
            cube(size=[1,1,1], center=true);
            cylinder(h=2,d=0.6,center = true, $fn=fragments);
        };
    };
};

module padHoles(x, y, zRot = 0) {
    rotate([0, 0, zRot]) {
        translate([x, y, 0]) {
            cylinder(h=2,d=0.6,center=true,$fn=fragments);
            translate([1,0,0]) {
                cylinder(h=2,d=0.6,center=true,$fn=fragments);
            };
        };
    };
};

module pad(x, y, zRot = 0) {
    rotate([0, 0, zRot]) {
        translate([x, y, 0]) {
            halfPad();
            difference() {
                translate([1,0,0]) {
                    halfPad();
                    };
                translate([1.25,0,0]) {
                    cube(size=[0.5,1.1,1.1], center=true);
                    };
            };
        };
    };
};

module pads() {
    pad(0,0);
    pad(0,2);
    pad(0,4);
    pad(0,6);
    pad(0,8);
    pad(0,10);
    pad(0,12);
    pad(0,14);
    pad(14,0,180);
    pad(14,-2,180);
    pad(14,-4,180);
    pad(14,-6,180);
    pad(14,-8,180);
    pad(14,-10,180);
    pad(14,-12,180);
    pad(14,-14,180);
};

module holes() {
    padHoles(0,0);
    padHoles(0,2);
    padHoles(0,4);
    padHoles(0,6);
    padHoles(0,8);
    padHoles(0,10);
    padHoles(0,12);
    padHoles(0,14);
    padHoles(14,0,180);
    padHoles(14,-2,180);
    padHoles(14,-4,180);
    padHoles(14,-6,180);
    padHoles(14,-8,180);
    padHoles(14,-10,180);
    padHoles(14,-12,180);
    padHoles(14,-14,180);
};

// Translate so that pin 1 and the bottom of the board
// is the reference point (0,0,0)
translate([14,-14,0.50]) {
    // pcb
    difference() {
        union() {
            color("Blue", a=1) {
                translate([-15, -1.6, -0.49]) {
                    cube(size=[16, 24, .98], center = false);
                };
            };
            pads();
        };
        holes();
    };

    //shield
    translate([-15 + 1.98, 0, 0.5]) {
        color("Silver", a=1) {
            cube(size=[12.1, 15.2, 2.3], center = false);
        };
    };

    //led
    translate([-2.5,15.5,0.5]){
        color("White", a=1) {
        cube(size=[2,1,0.4], center=false);
        };
    };
};
