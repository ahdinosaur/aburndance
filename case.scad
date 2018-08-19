use <hull.scad>

// originally copied from https://gist.github.com/hexagon5un/0b119627e736658db186de3212fdad5f

// 50mm
// 23mm
// 19mm

w = 19.05;
l = 45.72;
pin_locations = [
  [ w/2, -l/2, 0],
  [-w/2, -l/2, 0],
  [-w/2, l/2, 0], 
  [ w/2, l/2, 0]
];
pin_height = 6;
edge_from_pins = 2.54;
thickness = 1;
$fn = 24;

module place(i){
	translate(pin_locations[i])
		children(0);
}

module foot(){
	cylinder(d=2 * edge_from_pins, h=thickness);
}

module standoff(height = pin_height){
	cylinder(d1=6.5, d2=4.5, h=3);
	cylinder(d=4.5, h=height);
}

module stand() {
  hull() {
    for (i=[2:3]){
      place(i) standoff(height = pin_height - 1);
    }
  }
}

module bottom(){
	hull(){
		place(0) foot();
		place(1) foot();
		place(2) foot();
		place(3) foot();
	}
}


module bumper(){
	difference(){
		rounded_box(pin_locations, radius=1 + edge_from_pins, height=10);
		translate([0,0,-1])
			rounded_box(pin_locations, radius=edge_from_pins, height=12);
	}
}

module PCB_case(){
	difference(){ 
		// frame
		union(){
      bottom();
			bumper();
			
      for (i=[0:1]){
        place(i) standoff();
      }

      stand();
		}
		// difference screw holes
		for (i=[0:1]){
			place(i) cylinder(d=2.5, h=100, center=true);
		}
    // difference usb
    translate([-4, -l, pin_height - 3.5])
    cube(
      [8, l, 4]
    ); 
	}
}

PCB_case();