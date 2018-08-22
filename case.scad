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

PCB_PITCH = 0.1 * INCHES_TO_MM;

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

BETWEEN_PCB_DISTANCE = 5.8;

TOP_PCB_LENGTH_Z = 1.8;

MAX_TOP_LENGTH_Z = 1;

TOP_WALL_LENGTH_Z = 3;

PCB_SCREW_SOCKET_DIAMETER = 4;
PCB_SCREW_SOCKET_Z_LENGTH = MAX_BOTTOM_LENGTH_Z * LARGE_TOLERANCE;
PCB_SCREW_SOCKET_Z_WALL_LENGTH = 3;
PCB_SCREW_PLUG_RADIUS = 2.5 * 1.05;

BOTTOM_WALL_LENGTH_Z = PCB_SCREW_SOCKET_Z_LENGTH + BETWEEN_PCB_DISTANCE + MAX_TOP_LENGTH_Z;


EXTERNAL_SCREW_SOCKET_RADIUS = 8;
EXTERNAL_SCREW_PLUG_RADIUS = 5;
EXTERNAL_CONNECTOR_THICKNESS = 3;

BOTTOM_EXTERNAL_CONNECTORS= [
  // side connectors
  [
    0,
    PCB_LENGTH_Y / 2 + WALL_THICKNESS,
    BOTTOM_WALL_LENGTH_Z
  ],
  [
    0,
    -PCB_LENGTH_Y / 2 - WALL_THICKNESS,
    BOTTOM_WALL_LENGTH_Z
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

TOP_EXTERNAL_CONNECTORS = [
  // side connectors
  [
    0,
    PCB_LENGTH_Y / 2 + WALL_THICKNESS,
    TOP_WALL_LENGTH_Z
  ],
  [
    0,
    -PCB_LENGTH_Y / 2 - WALL_THICKNESS,
    TOP_WALL_LENGTH_Z
  ]
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

module top () {
  bottom();
}

module inner_body () {
  rounded_box(
    PCB_PINS,
    radius=PCB_EDGE_RADIUS * (1/SMALL_TOLERANCE),
    height=BOTTOM_WALL_LENGTH_Z + WALL_THICKNESS
  );
}

module bumper(height){
	difference(){
    translate([0,0, -WALL_THICKNESS])
		rounded_box(
      PCB_PINS,
      radius=PCB_EDGE_RADIUS * (1/SMALL_TOLERANCE) + WALL_THICKNESS,
      height=height + WALL_THICKNESS
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

module external_connectors (positions) {
  difference () {
    union () {
      for (i = [0:5]) {
        translate(
          [
            positions[i][0],
            positions[i][1],
            -WALL_THICKNESS
          ]
        )
        difference () {
          cylinder(
            r = EXTERNAL_SCREW_SOCKET_RADIUS,
            h = max(positions[i][2], EXTERNAL_CONNECTOR_THICKNESS) + WALL_THICKNESS
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

module bottom_case(){
	difference(){ 
		// frame
		union(){
      bottom();
			bumper(height = BOTTOM_WALL_LENGTH_Z);
			
      for (i=[0: 1]){
        place_pin(i) standoff();
      }

      stand();
      
      external_connectors(BOTTOM_EXTERNAL_CONNECTORS);
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

module encoder_hole () {
  encoder_length = 15;
  
  translate(
    [
      (-1/2) * PCB_LENGTH_X + 1,
      (-1/2) * encoder_length,
      (-1/2) * INFINITY
    ]
  )
  cube(
    [
      encoder_length,
      encoder_length,
      INFINITY
    ]
  );
}

module buttons_hole () {
  buttons_length_x = 33;
  buttons_length_y = 8 * MEDIUM_TOLERANCE;

  translate(
    [
      (1/2) * PCB_LENGTH_X - buttons_length_x,
      -2 * PCB_PITCH * MEDIUM_TOLERANCE,
      (-1/2) * INFINITY
    ]
  )
  cube(
    [
      buttons_length_x,
      buttons_length_y,
      INFINITY
    ]
  );
}

module headers_hole () {
  headers_length_x = 9 * PCB_PITCH * SMALL_TOLERANCE;
  headers_length_y = 1 * PCB_PITCH * MEDIUM_TOLERANCE;

  translate(
    [
      (1/2) * PCB_LENGTH_X - headers_length_x - 2 * PCB_PITCH,
      (3/2) * PCB_PITCH * SMALL_TOLERANCE,
      (-1/2) * INFINITY
    ]
  )
  cube(
    [
      headers_length_x,
      headers_length_y,
      INFINITY
    ]
  );
}

module top_case () {
  mirror([1, 0, 0])
  difference () {
    union () {
      top();
      bumper(height = TOP_WALL_LENGTH_Z);
      
      external_connectors(TOP_EXTERNAL_CONNECTORS);
    }
    
    encoder_hole();
    buttons_hole();
    headers_hole();
  }
}

echo(pcb_edge_radius = PCB_EDGE_RADIUS);
echo(bottom_wall_length_z = BOTTOM_WALL_LENGTH_Z);

bottom_case();
// top_case();