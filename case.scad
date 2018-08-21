use <hull.scad>

// originally copied from https://gist.github.com/hexagon5un/0b119627e736658db186de3212fdad5f

// 50mm
// 23mm
// 19mm

// PLUG and SOCKET
INFINITESIMAL = 0.001;
INFINITY = 100;
INCHES_TO_MM = 25.4;
LARGE_TOLERANCE = 1.4;
MEDIUM_TOLERANCE = 1.2;
SMALL_TOLERANCE = 1.1;
$fn = 24;

WALL_THICKNESS = 1;
PCB_LENGTH_X = 2 * INCHES_TO_MM;
PCB_PIN_OFFSET_X = 0.1 * INCHES_TO_MM;
PCB_LENGTH_Y = 0.9 * INCHES_TO_MM;
PCB_PIN_OFFSET_Y = 0.075 * INCHES_TO_MM;

BOTTOM_PCB_LENGTH_Z = 1.8;
BATTERY_SOCKET_LENGTH_Z = 7.25 - BOTTOM_PCB_LENGTH_Z;
USB_SOCKET_LENGTH_Y = 8;
USB_SOCKET_LENGTH_Z = 4.55 - BOTTOM_PCB_LENGTH_Z;
ANTENNA_LENGTH_Z = 2.6 - BOTTOM_PCB_LENGTH_Z;
MAX_BOTTOM_LENGTH_Z = BATTERY_SOCKET_LENGTH_Z;

BETWEEN_PCB_DISTANCE = 5.4;

TOP_PCB_LENGTH_Z = 1.8;

MAX_TOP_LENGTH_Z = 1;

WALL_LENGTH_Z = WALL_THICKNESS + MAX_BOTTOM_LENGTH_Z + BETWEEN_PCB_DISTANCE + MAX_TOP_LENGTH_Z;

PCB_SCREW_SOCKET_DIAMETER = 4;
PCB_SCREW_SOCKET_Z_LENGTH = MAX_BOTTOM_LENGTH_Z * LARGE_TOLERANCE;
PCB_SCREW_SOCKET_Z_WALL_LENGTH = 3;
PCB_SCREW_PLUG_RADIUS = 2.5 * 1.05;

EXTERNAL_SCREW_SOCKET_RADIUS = 8;
EXTERNAL_SCREW_PLUG_RADIUS = 5;
EXTERNAL_CONNECTOR_THICKNESS = 4;

EXTERNAL_CONNECTORS= [
  // side connectors
  [
    0,
    PCB_LENGTH_Y / 2 + WALL_THICKNESS,
    WALL_LENGTH_Z
  ],
  [
    0,
    -PCB_LENGTH_Y / 2 - WALL_THICKNESS,
    WALL_LENGTH_Z
  ],
  // body connectors
  [
    (1/4) * PCB_LENGTH_X,
    (1/2) * PCB_LENGTH_Y + WALL_THICKNESS,
    0
  ],
  [
    (-1/4) * PCB_LENGTH_X,
    (-1/2) * PCB_LENGTH_Y - WALL_THICKNESS,
    0
  ],
  [
    (1/4) * PCB_LENGTH_X,
    (-1/2) * PCB_LENGTH_Y - WALL_THICKNESS,
    0
  ],
   [
    (-1/4) * PCB_LENGTH_X,
    (1/2) * PCB_LENGTH_Y + WALL_THICKNESS,
    0
  ],
];

PCB_EDGE_RADIUS = sqrt(pow(PCB_PIN_OFFSET_X, 2) + pow(PCB_PIN_OFFSET_Y, 2));
PCB_PINS = [
  [
    (PCB_LENGTH_X / 2) - PCB_PIN_OFFSET_X,
    (PCB_LENGTH_Y / 2) - PCB_PIN_OFFSET_Y,
    0
  ],
  [
    (PCB_LENGTH_X / 2) - PCB_PIN_OFFSET_X,
    (-PCB_LENGTH_Y / 2) + PCB_PIN_OFFSET_Y,
    0
  ],
  [
    (-PCB_LENGTH_X / 2) + PCB_PIN_OFFSET_X,
    (-PCB_LENGTH_Y / 2) + PCB_PIN_OFFSET_Y,
    0
   ],
  [
    (-PCB_LENGTH_X / 2) + PCB_PIN_OFFSET_X,
    (PCB_LENGTH_Y / 2) - PCB_PIN_OFFSET_Y,
    0
  ]
];

module place_pin (i) {
	translate(PCB_PINS[i])
    children(0);
}

module foot () {
	cylinder(d=2 * PCB_EDGE_RADIUS, h=WALL_THICKNESS);
}

module standoff () {
	cylinder(d1=2 * PCB_EDGE_RADIUS, d2=PCB_SCREW_SOCKET_DIAMETER, h=PCB_SCREW_SOCKET_Z_WALL_LENGTH);
	cylinder(d=PCB_SCREW_SOCKET_DIAMETER, h=PCB_SCREW_SOCKET_Z_LENGTH);
}

module stand () {
  intersection () {
    inner_body();
    
    hull() {
      for (i=[2: 3]) {
        place_pin(i)
        translate(
          [
            - PCB_EDGE_RADIUS,
            - PCB_EDGE_RADIUS,
            0
          ]
        )
        cube(
          [
            2 * PCB_EDGE_RADIUS,
            2 * PCB_EDGE_RADIUS,
            PCB_SCREW_SOCKET_Z_LENGTH - ANTENNA_LENGTH_Z
          ]
        );
      }
    }
  }
}

module bottom(){
	hull(){
		place_pin(0) foot();
		place_pin(1) foot();
		place_pin(2) foot();
		place_pin(3) foot();
	}
}

module inner_body () {
  rounded_box(
    PCB_PINS,
    radius=PCB_EDGE_RADIUS * (1/SMALL_TOLERANCE),
    height=WALL_LENGTH_Z + WALL_THICKNESS
  );
}

module bumper(){
	difference(){
    translate([0,0, -WALL_THICKNESS])
		rounded_box(
      PCB_PINS,
      radius=PCB_EDGE_RADIUS * (1/SMALL_TOLERANCE) + WALL_THICKNESS,
      height=WALL_LENGTH_Z + WALL_THICKNESS
    );

    inner_body();
	}
}

module usb_hole () {
  length_x = 2 * WALL_THICKNESS;
  length_y = USB_SOCKET_LENGTH_Y * SMALL_TOLERANCE;
  length_z = USB_SOCKET_LENGTH_Z * LARGE_TOLERANCE;
  
  offset_x = PCB_LENGTH_X / 2;
  offset_y = - length_y / 2;
  offset_z = (PCB_SCREW_SOCKET_Z_LENGTH * MEDIUM_TOLERANCE) - length_z;
  
  translate([offset_x, offset_y, offset_z])
    cube([length_x, length_y, length_z]);
}

module external_connectors () {
  difference () {
    union () {
      for (i = [0:5]) {
        translate(
          [
            EXTERNAL_CONNECTORS[i][0],
            EXTERNAL_CONNECTORS[i][1],
            -WALL_THICKNESS
          ]
        )
        difference () {
          cylinder(
            r = EXTERNAL_SCREW_SOCKET_RADIUS,
            h = max(EXTERNAL_CONNECTORS[i][2], EXTERNAL_CONNECTOR_THICKNESS) + WALL_THICKNESS
          );
          
          translate([0, 0, -1/2 * INFINITY])
          cylinder(
            r = EXTERNAL_SCREW_PLUG_RADIUS,
            h = INFINITY
          );
        }
      }
    }
 
    inner_body();
  }
}

module PCB_case(){
	difference(){ 
		// frame
		union(){
      bottom();
			bumper();
			
      for (i=[0: 1]){
        place_pin(i) standoff();
      }

      stand();
      
      external_connectors();
		}
    
    /*
		// difference screw holes
		for (i=[0: 1]){
			place(i) cylinder(d=SCREW_PLUG_RADIUS, h=INFINITY, center=true);
		}
    */
    
    // difference usb
    usb_hole();
	}
}

echo(pcb_edge_radius = PCB_EDGE_RADIUS);

PCB_case();