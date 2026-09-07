part = "layout"; // "base", "lid", "assembly", or "layout"

$fn = 48;

outer_length = 80;
outer_width = 50;
total_height = 12;
base_thickness = 1.6;
lid_height = total_height - base_thickness;
roof = 1.6;
wall = 1.6;
corner_radius = 3;

case_screw_x = [2.7, outer_length - 2.7];
case_screw_y = [2.7, outer_width - 2.7];
case_boss_diameter = 4.6;
case_pilot_diameter = 1.8;
case_clearance_diameter = 2.8;
case_pilot_start = 3.5;

mount_standoff = 1.2;
component_z = roof + mount_standoff;
mount_post_diameter = 4.8;
mount_pilot_diameter = 1.8;
mount_pilot_floor = 0.4;

seed_envelope = [51.26, 18.24, 6.5];
seed_origin = [2.1, 5.5, component_z];

oled_board = [33.02, 21.59, 6];
oled_origin = [5.5, 26, component_z];
oled_hole_spacing = [27.94, 16.51];
oled_window = [26, 8.5];
oled_center = [oled_origin[0] + oled_board[0] / 2,
               oled_origin[1] + oled_board[1] / 2];

trrs_board = [17.145, 17.78, 6.5];
trrs_x = outer_length - wall - trrs_board[0] - 0.4;
trrs_origins = [
    [trrs_x, 5.5, component_z],
    [trrs_x, 26.7, component_z]
];
trrs_hole_x_offset = 14.605;
trrs_hole_y_offsets = [2.54, 15.24];

button_size = [6, 6, 6];
button_center = [48, 35.5];
button_body_depth = 3.6;
button_fit_clearance = 0.3;
button_holder_wall = 1;
button_standoff = 0;
button_z = roof + button_standoff;
button_access_diameter = 7;

usb_cutout = [12, 5.5];
usb_center_y = seed_origin[1] + seed_envelope[1] / 2;
usb_center_z = component_z + seed_envelope[2] / 2;
audio_cutout_diameter = 7.5;
audio_center_z = component_z + trrs_board[2] / 2;

module rounded_prism(size, radius) {
    linear_extrude(height = size[2])
        offset(r = radius)
            offset(delta = -radius)
                square([size[0], size[1]]);
}

module case_screw_positions() {
    for (x = case_screw_x)
        for (y = case_screw_y)
            translate([x, y, 0]) children();
}

module oled_post_positions() {
    for (x = [-oled_hole_spacing[0] / 2, oled_hole_spacing[0] / 2])
        for (y = [-oled_hole_spacing[1] / 2, oled_hole_spacing[1] / 2])
            translate([oled_center[0] + x, oled_center[1] + y, 0]) children();
}

module trrs_post_positions() {
    for (origin = trrs_origins)
        for (y_offset = trrs_hole_y_offsets)
            translate([origin[0] + trrs_hole_x_offset,
                       origin[1] + y_offset,
                       0]) children();
}

module lid_shell() {
    difference() {
        rounded_prism([outer_length, outer_width, lid_height], corner_radius);
        translate([wall, wall, roof])
            rounded_prism([outer_length - 2 * wall,
                           outer_width - 2 * wall,
                           lid_height - roof + 0.1],
                          max(corner_radius - wall, 0.1));
    }
}

module case_bosses() {
    case_screw_positions()
        translate([0, 0, roof - 0.1])
            cylinder(h = lid_height - roof + 0.1,
                     d = case_boss_diameter);
}

module component_mount_post() {
    translate([0, 0, roof - 0.1])
        cylinder(h = mount_standoff + 0.1,
                 d = mount_post_diameter);
}

module component_mounts() {
    oled_post_positions() component_mount_post();
    trrs_post_positions() component_mount_post();
}

module seed_guides() {
    guide_length = 3;
    guide_width = 0.7;
    guide_height = mount_standoff + 1.3;
    for (x = [seed_origin[0] + 6,
              seed_origin[0] + seed_envelope[0] - 9]) {
        translate([x,
                   seed_origin[1] - guide_width,
                   roof])
            cube([guide_length, guide_width, guide_height]);
        translate([x,
                   seed_origin[1] + seed_envelope[1],
                   roof])
            cube([guide_length, guide_width, guide_height]);
    }
    translate([seed_origin[0] + seed_envelope[0],
               seed_origin[1] + 6,
               roof])
        cube([guide_width, 6, guide_height]);
}

module button_holder() {
    pocket = [button_size[0] + button_fit_clearance,
              button_size[1] + button_fit_clearance];
    x0 = button_center[0] - pocket[0] / 2;
    y0 = button_center[1] - pocket[1] / 2;
    holder_height = button_standoff + button_body_depth + 0.6;
    lip_z = button_z + button_body_depth;

    translate([x0 - button_holder_wall,
               y0 - button_holder_wall,
               roof])
        cube([button_holder_wall,
              pocket[1] + 2 * button_holder_wall,
              holder_height]);
    for (y = [y0 - button_holder_wall, y0 + pocket[1]]) {
        translate([x0, y, roof])
            cube([pocket[0], button_holder_wall, holder_height]);
    }
    translate([x0, y0 - button_holder_wall, lip_z])
        cube([pocket[0], button_holder_wall + 0.8, 0.6]);
    translate([x0, y0 + pocket[1] - 0.8, lip_z])
        cube([pocket[0], button_holder_wall + 0.8, 0.6]);
}

module case_pilot_cuts() {
    case_screw_positions()
        translate([0, 0, case_pilot_start])
            cylinder(h = lid_height - case_pilot_start + 0.2,
                     d = case_pilot_diameter);
}

module component_pilot_cuts() {
    oled_post_positions()
        translate([0, 0, mount_pilot_floor])
            cylinder(h = component_z - mount_pilot_floor + 0.2,
                     d = mount_pilot_diameter);
    trrs_post_positions()
        translate([0, 0, mount_pilot_floor])
            cylinder(h = component_z - mount_pilot_floor + 0.2,
                     d = mount_pilot_diameter);
}

module connector_cuts() {
    translate([-0.1,
               usb_center_y - usb_cutout[0] / 2,
               usb_center_z - usb_cutout[1] / 2])
        cube([wall + 0.2, usb_cutout[0], usb_cutout[1]]);

    for (origin = trrs_origins)
        translate([outer_length - wall - 0.1,
                   origin[1] + trrs_board[1] / 2,
                   audio_center_z])
            rotate([0, 90, 0])
                cylinder(h = wall + 0.2, d = audio_cutout_diameter);
}

module lid() {
    difference() {
        union() {
            lid_shell();
            case_bosses();
            component_mounts();
            seed_guides();
            button_holder();
        }

        translate([oled_center[0] - oled_window[0] / 2,
                   oled_center[1] - oled_window[1] / 2,
                   -0.1])
            cube([oled_window[0], oled_window[1], roof + 0.2]);

        translate([button_center[0], button_center[1], -0.1])
            cylinder(h = roof + 0.2, d = button_access_diameter);

        connector_cuts();
        case_pilot_cuts();
        component_pilot_cuts();
    }
}

module base() {
    difference() {
        rounded_prism([outer_length,
                       outer_width,
                       base_thickness],
                      corner_radius);
        case_screw_positions()
            translate([0, 0, -0.1])
                cylinder(h = base_thickness + 0.2,
                         d = case_clearance_diameter);
    }
}

module component_preview() {
    color("black") translate(seed_origin) cube(seed_envelope);
    color("navy") translate(oled_origin) cube(oled_board);
    for (origin = trrs_origins)
        color("royalblue") translate(origin) cube(trrs_board);
    color("darkslategray")
        translate([button_center[0] - button_size[0] / 2,
                   button_center[1] - button_size[1] / 2,
                   button_z])
            cube(button_size);
}

module position_base_for_assembly() {
    translate([outer_length, 0, total_height])
        rotate([0, 180, 0]) children();
}

module assembly() {
    color("whitesmoke", 0.8) lid();
    position_base_for_assembly()
        color("gainsboro", 0.8) base();
    component_preview();
}

if (part == "base") {
    base();
} else if (part == "lid") {
    lid();
} else if (part == "assembly") {
    assembly();
} else {
    lid();
    translate([0, outer_width + 8, 0]) base();
}
