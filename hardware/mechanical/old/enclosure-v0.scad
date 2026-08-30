part = "layout"; // "base", "lid", "oled-clamps", "switch-fit-test", "assembly", or "layout"
hardware_variant = "m2-panel"; // "m2-panel" or "m4-tactile"

$fn = 48;

outer_length = 105;
outer_width = 42;
base_height = 24;
wall = 2;
floor = 2;
lid_thickness = 2.4;
corner_radius = 3;
fit_clearance = 0.25;

is_m4_tactile = hardware_variant == "m4-tactile";

// The default variant retains the original panel-switch placeholder. The M4
// variant fits Adafruit product 1119's stated 12 x 12 x 6 mm envelope.
button_cutout_diameter = 12.2;
button_center = [82, outer_width / 2];
tactile_body = [12, 12, 6];
tactile_fit_clearance = 0.3;
tactile_actuator_diameter = 7.4;
tactile_holder_wall = 1.6;
tactile_holder_shelf = 1.4;

oled_board = [33.02, 21.59];
oled_hole_spacing = [27.94, 16.51];
oled_hole_diameter = 2.8;
oled_window = [26, 8.5];
oled_center = [48, outer_width / 2];
oled_stack_height = 4;
oled_clamp_thickness = 2.4;
oled_clamp_width = 4.8;
oled_clamp_screw_y = [4.8, outer_width - 4.8];
oled_clamp_x = [oled_center[0] - oled_board[0] / 2 + oled_clamp_width / 2,
                oled_center[0] + oled_board[0] / 2 - oled_clamp_width / 2];

usb_cutout = [13, 9];
usb_center_y = outer_width / 2;
usb_bottom_z = 6;

audio_cutout_diameter = 7.5;
audio_center_y = outer_width / 2;
audio_center_z = 10;

// BOOT and RESET coordinates depend on the installed Seed3/header stack.
// Leave disabled until the physical assembly has been measured.
service_holes_enabled = false;
boot_hole = [14, 12];
reset_hole = [14, 30];
service_hole_diameter = 3;

screw_inset = is_m4_tactile ? 6 : 5.5;
screw_clearance = is_m4_tactile ? 4.5 : 2.4;
boss_diameter = is_m4_tactile ? 10 : 6;
boss_pilot = is_m4_tactile ? 3.4 : 1.7;
component_boss_diameter = 8.5;
component_screw_clearance = 4.5;
component_screw_pilot = 3.4;

seed_envelope = [51.26, 18.24, 8];
seed_origin = [8, (outer_width - seed_envelope[1]) / 2, floor + 3];
trrs_envelope = [17.78, 17.145, 6.5];
trrs_origin = [outer_length - wall - trrs_envelope[0] - 4,
               (outer_width - trrs_envelope[1]) / 2,
               floor + 3];

module rounded_prism(size, radius) {
    linear_extrude(height = size[2])
        offset(r = radius)
            offset(delta = -radius)
                square([size[0], size[1]]);
}

module screw_positions() {
    for (x = [screw_inset, outer_length - screw_inset])
        for (y = [screw_inset, outer_width - screw_inset])
            translate([x, y, 0]) children();
}

module oled_clamp_positions() {
    for (x = oled_clamp_x)
        for (y = oled_clamp_screw_y)
            translate([x, y, 0]) children();
}

module tactile_holder(center = button_center) {
    pocket = [tactile_body[0] + tactile_fit_clearance,
              tactile_body[1] + tactile_fit_clearance];
    holder_height = tactile_body[2] + tactile_fit_clearance;
    side_length = pocket[1] + tactile_holder_wall;
    back_block_width = (pocket[0] - 5) / 2;

    for (side = [-1, 1]) {
        x = side < 0
            ? center[0] - pocket[0] / 2 - tactile_holder_wall
            : center[0] + pocket[0] / 2;
        translate([x,
                   center[1] - pocket[1] / 2 - tactile_holder_wall,
                   lid_thickness - 0.1])
            cube([tactile_holder_wall,
                  side_length,
                  holder_height + tactile_holder_shelf + 0.1]);

        shelf_x = side < 0
            ? center[0] - pocket[0] / 2
            : center[0] + pocket[0] / 2 - tactile_holder_shelf;
        translate([shelf_x,
                   center[1] - pocket[1] / 2 - tactile_holder_wall,
                   lid_thickness + holder_height])
            cube([tactile_holder_shelf,
                  side_length,
                  tactile_holder_shelf]);
    }

    for (side = [-1, 1]) {
        x = side < 0
            ? center[0] - pocket[0] / 2
            : center[0] + 2.5;
        translate([x,
                   center[1] - pocket[1] / 2 - tactile_holder_wall,
                   lid_thickness - 0.1])
            cube([back_block_width,
                  tactile_holder_wall,
                  holder_height + tactile_holder_shelf + 0.1]);
    }
}

module rounded_bar(width, height, hole_spacing, hole_diameter) {
    difference() {
        hull()
            for (y = [-hole_spacing / 2, hole_spacing / 2])
                translate([0, y, 0]) cylinder(h = height, d = width);
        for (y = [-hole_spacing / 2, hole_spacing / 2])
            translate([0, y, -0.1])
                cylinder(h = height + 0.2, d = hole_diameter);
    }
}

module oled_clamps() {
    hole_spacing = oled_clamp_screw_y[1] - oled_clamp_screw_y[0];
    for (x = oled_clamp_x)
        translate([x, outer_width / 2, 0])
            rounded_bar(oled_clamp_width,
                        oled_clamp_thickness,
                        hole_spacing,
                        component_screw_clearance);
}

module base() {
    difference() {
        union() {
            difference() {
                rounded_prism([outer_length, outer_width, base_height], corner_radius);
                translate([wall, wall, floor])
                    rounded_prism(
                        [outer_length - 2 * wall,
                         outer_width - 2 * wall,
                         base_height - floor + 1],
                        max(corner_radius - wall, 0.1));
            }
            screw_positions()
                cylinder(h = base_height - lid_thickness, d = boss_diameter);
        }

        translate([-1, usb_center_y - usb_cutout[0] / 2, usb_bottom_z])
            cube([wall + 2, usb_cutout[0], usb_cutout[1]]);

        translate([outer_length - wall - 1, audio_center_y, audio_center_z])
            rotate([0, 90, 0])
                cylinder(h = wall + 2, d = audio_cutout_diameter);

        screw_positions()
            translate([0, 0, floor])
                cylinder(h = base_height, d = boss_pilot);
    }
}

module lid() {
    difference() {
        union() {
            rounded_prism([outer_length, outer_width, lid_thickness], corner_radius);
            translate([wall + fit_clearance, wall + fit_clearance, lid_thickness])
                difference() {
                    rounded_prism(
                        [outer_length - 2 * (wall + fit_clearance),
                         outer_width - 2 * (wall + fit_clearance), 2],
                        max(corner_radius - wall - fit_clearance, 0.1));
                    translate([1.2, 1.2, -0.1])
                        rounded_prism(
                            [outer_length - 2 * (wall + fit_clearance + 1.2),
                             outer_width - 2 * (wall + fit_clearance + 1.2), 2.2],
                            max(corner_radius - wall - fit_clearance - 1.2, 0.1));
                }

            if (is_m4_tactile) {
                tactile_holder();
                oled_clamp_positions()
                    translate([0, 0, lid_thickness - 0.1])
                        cylinder(h = oled_stack_height + 0.1,
                                 d = component_boss_diameter);
            }
        }

        translate([oled_center[0] - oled_window[0] / 2,
                   oled_center[1] - oled_window[1] / 2, -0.1])
            cube([oled_window[0], oled_window[1], lid_thickness + 0.2]);

        if (!is_m4_tactile) {
            for (x = [-oled_hole_spacing[0] / 2, oled_hole_spacing[0] / 2])
                for (y = [-oled_hole_spacing[1] / 2, oled_hole_spacing[1] / 2])
                    translate([oled_center[0] + x, oled_center[1] + y, -0.1])
                        cylinder(h = lid_thickness + 0.2, d = oled_hole_diameter);
        }

        translate([button_center[0], button_center[1], -0.1])
            cylinder(h = lid_thickness + 0.2,
                     d = is_m4_tactile
                         ? tactile_actuator_diameter
                         : button_cutout_diameter);

        if (service_holes_enabled) {
            for (point = [boot_hole, reset_hole])
                translate([point[0], point[1], -0.1])
                    cylinder(h = lid_thickness + 0.2, d = service_hole_diameter);
        }

        screw_positions()
            translate([0, 0, -0.1])
                cylinder(h = lid_thickness + 0.2, d = screw_clearance);

        if (is_m4_tactile) {
            oled_clamp_positions()
                translate([0, 0, lid_thickness - 0.1])
                    cylinder(h = oled_stack_height + 0.2,
                             d = component_screw_pilot);
        }
    }
}

module base_component_preview() {
    color("black")
        translate(seed_origin) cube(seed_envelope);
    color("royalblue")
        translate(trrs_origin) cube(trrs_envelope);
}

module lid_component_preview() {
    color("navy")
        translate([oled_center[0] - oled_board[0] / 2,
                   oled_center[1] - oled_board[1] / 2,
                   lid_thickness])
            cube([oled_board[0], oled_board[1], 4]);

    if (is_m4_tactile) {
        color("darkslategray")
            translate([button_center[0] - tactile_body[0] / 2,
                       button_center[1] - tactile_body[1] / 2,
                       lid_thickness])
                cube(tactile_body);
        color("silver")
            translate([0, 0, lid_thickness + oled_stack_height])
                oled_clamps();
    }
}

module position_lid_for_assembly() {
    translate([0, outer_width, base_height + lid_thickness])
        rotate([180, 0, 0]) children();
}

module switch_fit_test() {
    test_size = 22;
    difference() {
        translate([button_center[0] - test_size / 2,
                   button_center[1] - test_size / 2, 0])
            cube([test_size, test_size, lid_thickness]);
        translate([button_center[0], button_center[1], -0.1])
            cylinder(h = lid_thickness + 0.2, d = tactile_actuator_diameter);
    }
    tactile_holder();
}

module assembly() {
    color("gainsboro", 0.8) base();
    position_lid_for_assembly() color("whitesmoke", 0.8) lid();
    base_component_preview();
    position_lid_for_assembly() lid_component_preview();
}

if (part == "base") {
    base();
} else if (part == "lid") {
    lid();
} else if (part == "oled-clamps") {
    oled_clamps();
} else if (part == "switch-fit-test") {
    translate([-button_center[0] + 11, -button_center[1] + 11, 0])
        switch_fit_test();
} else if (part == "assembly") {
    assembly();
} else {
    base();
    translate([0, outer_width + 8, 0]) lid();
    if (is_m4_tactile)
        translate([outer_length + 8, 0, 0]) oled_clamps();
}
