fragments=20;
module halfPad() {
    color("Gold", a=1) {
        difference() {
            translate([0, 0, 0.5]) {
            		cube(size=[0.85,0.90,1], center=true);
        		};
			};
    };
};

module padHoles(x, y, zRot = 0) {
    rotate([0, 0, zRot]) {
        translate([x, y, 0.5]) {
            cylinder(h=2,d=0.6,center=true,$fn=fragments);
        };
    };
};

module pad(x, y, zRot = 0) {
    rotate([0, 0, zRot]) {
        translate([x, y, 0]) {
            halfPad();
        };
    };
};

module pads() {
	
	pad(17.58,-12);
	pad(17.58,-10.5);
   pad(17.58,-9);
   pad(17.58,-7.5);
   pad(17.58,-6);
   pad(17.58,-4.5);
   pad(17.58,-3);
   pad(17.58,-1.5);
   pad(17.58,0);

   pad(-0.42,0,180);
   pad(-0.42,1.5,180);
   pad(-0.42,3,180);
   pad(-0.42,4.5,180);
   pad(-0.42,6,180);
   pad(-0.42,7.5,180);
   pad(-0.42,9,180);
   pad(-0.42,10.5,180);
   pad(-0.42,12,180);

};

module holes() {

   padHoles(18,-12);
   padHoles(18,-10.5);
   padHoles(18,-9);
   padHoles(18,-7.5);
   padHoles(18,-6);
   padHoles(18,-4.5);
   padHoles(18,-3);
   padHoles(18,-1.5);
   padHoles(18,0);
   
   padHoles(0,0,180);
   padHoles(0,1.5,180);
   padHoles(0,3,180);
   padHoles(0,4.5,180);
   padHoles(0,6,180);
   padHoles(0,7.5,180);
   padHoles(0,9,180);
   padHoles(0,10.5,180);
   padHoles(0,12,180);
};

// Translate so that pin 1 and the bottom of the board
// is the reference point (0,0,0)
translate([0,0,0]) {
	// pcb
   difference() {
       union() {
           color("Blue", a=1) {
               translate([0, -13, 0]) {
                   cube(size=[18, 20, .98], center = false);
               };
           };
           pads();
       };
       holes();
   };

   //shield
   translate([1.5, -12, 1]) {
		color("Silver", a=1) {
			cube(size=[15, 12, 1.5], center = false);
		};
	};
};