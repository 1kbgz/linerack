part = "layout"; // "base", "lid", "assembly", "snap-fit-test", or "layout"

$fn = 48;

outer_length = 60;
outer_width = 31;
base_height = 18.3;
lid_thickness = 2.4;
wall = 2;
floor = 2;
corner_radius = 3;

fit_clearance = 0.3;
skirt_thickness = 1.2;
skirt_depth = 6;
snap_centers = [14, 36];
snap_tab_width = 8;
snap_slot_width = 0.8;
snap_anchor_z = 7.5;
snap_engagement = 0.45;
snap_height = 1.2;
snap_nub_z = base_height - 2;

seed_envelope = [51.26, 18.24, 10];
seed_origin = [2.5, 2.5, floor];

oled_board = [33.02, 21.59, 6];
oled_origin = [4, (outer_width - oled_board[1]) / 2];
oled_hole_spacing = [27.94, 16.51];
oled_window = [26, 8.5];
oled_center = [oled_origin[0] + oled_board[0] / 2,
               oled_origin[1] + oled_board[1] / 2];

trrs_board = [17.145, 17.78, 6.5];
trrs_origin = [outer_length - wall - trrs_board[0],
               (outer_width - trrs_board[1]) / 2];
trrs_hole_x = trrs_origin[0] + 14.605;
trrs_hole_y = [trrs_origin[1] + 2.54, trrs_origin[1] + 15.24];

mount_standoff = 1.5;
mount_post_diameter = 4.8;
mount_pilot_diameter = 1.6;
mount_pilot_start = 1.2;

button_body = [12, 6, 12];
button_center = [49, outer_width - wall - button_body[1] / 2, floor + 6];
button_actuator_diameter = 7.4;
button_holder_wall = 1.3;
button_fit_clearance = 0.3;

usb_cutout = [13, 9];
usb_center_y = seed_origin[1] + seed_envelope[1] / 2;
usb_bottom_z = 3.5;

audio_cutout_diameter = 7.5;
audio_center_y = outer_width / 2;
audio_center_z = base_height - mount_standoff - trrs_board[2] / 2;

module rounded_prism(size, radius) {
    linear_extrude(height = size[2])
        offset(r = radius)
            offset(delta = -radius)
                square([size[0], size[1]]);
}

module snap_nubs() {
    for (x = snap_centers) {
        translate([x - (snap_tab_width - 2) / 2,
                   wall - 0.1,
                   snap_nub_z])
            cube([snap_tab_width - 2,
                  snap_engagement + 0.1,
                  snap_height]);
        translate([x - (snap_tab_width - 2) / 2,
                   outer_width - wall - snap_engagement,
                   snap_nub_z])
            cube([snap_tab_width - 2,
                  snap_engagement + 0.1,
                  snap_height]);
    }
}

module snap_slot_cuts() {
    for (x = snap_centers)
        for (side = [0, 1])
            for (offset = [-snap_tab_width / 2 - snap_slot_width,
                           snap_tab_width / 2])
                translate([x + offset,
                           side == 0 ? -0.1 : outer_width - wall - 0.1,
                           snap_anchor_z])
                    cube([snap_slot_width,
                          wall + 0.2,
                          base_height - snap_anchor_z + 0.2]);
}

module seed_cradle() {
    guide_length = 4;
    guide_width = 0.8;
    guide_height = 1.5;
    for (x = [seed_origin[0] + 6,
              seed_origin[0] + seed_envelope[0] - 10]) {
        translate([x, seed_origin[1] - guide_width, floor])
            cube([guide_length, guide_width, guide_height]);
        translate([x, seed_origin[1] + seed_envelope[1], floor])
            cube([guide_length, guide_width, guide_height]);
    }
    translate([seed_origin[0] + seed_envelope[0],
               seed_origin[1],
               floor])
        cube([guide_width, seed_envelope[1], guide_height]);
}

module button_holder() {
    x0 = button_center[0] - button_body[0] / 2;
    y0 = outer_width - wall - button_body[1] - button_fit_clearance;
    z0 = floor;
    for (x = [x0, x0 + button_body[0] - 2])
        translate([x, y0 - button_holder_wall, z0])
            cube([2, button_holder_wall, button_body[2]]);
    for (x = [x0 - button_holder_wall - button_fit_clearance / 2,
              x0 + button_body[0] + button_fit_clearance / 2])
        translate([x, y0 - button_holder_wall, z0])
            cube([button_holder_wall,
                  button_body[1] + button_holder_wall,
                  2]);
}

module base_shell() {
    difference() {
        rounded_prism([outer_length, outer_width, base_height], corner_radius);
        translate([wall, wall, floor])
            rounded_prism([outer_length - 2 * wall,
                           outer_width - 2 * wall,
                           base_height - floor + 0.1],
                          max(corner_radius - wall, 0.1));
    }
}

module base() {
    difference() {
        union() {
            base_shell();
            snap_nubs();
            seed_cradle();
            button_holder();
        }

        translate([-0.1,
                   usb_center_y - usb_cutout[0] / 2,
                   usb_bottom_z])
            cube([wall + 0.2, usb_cutout[0], usb_cutout[1]]);

        translate([outer_length - wall - 0.1,
                   audio_center_y,
                   audio_center_z])
            rotate([0, 90, 0])
                cylinder(h = wall + 0.2, d = audio_cutout_diameter);

        translate([button_center[0], outer_width + 0.1, button_center[2]])
            rotate([90, 0, 0])
                cylinder(h = wall + 0.2, d = button_actuator_diameter);

        snap_slot_cuts();
    }
}

module lid_skirt() {
    x0 = wall + fit_clearance;
    y0 = wall + fit_clearance;
    rail_length = outer_length - 2 * (wall + fit_clearance);
    union() {
        translate([x0, y0, lid_thickness])
            cube([rail_length, skirt_thickness, skirt_depth]);
        translate([x0,
                   outer_width - wall - fit_clearance - skirt_thickness,
                   lid_thickness])
            cube([rail_length, skirt_thickness, skirt_depth]);
        translate([x0, y0 + skirt_thickness, lid_thickness])
            cube([skirt_thickness,
                  outer_width - 2 * (y0 + skirt_thickness),
                  skirt_depth]);
    }
}

module oled_post_positions() {
    for (x = [-oled_hole_spacing[0] / 2, oled_hole_spacing[0] / 2])
        for (y = [-oled_hole_spacing[1] / 2, oled_hole_spacing[1] / 2])
            translate([oled_center[0] + x, oled_center[1] + y, 0]) children();
}

module trrs_post_positions() {
    for (y = trrs_hole_y)
        translate([trrs_hole_x, y, 0]) children();
}

module mount_post() {
    translate([0, 0, lid_thickness - 0.1])
        cylinder(h = mount_standoff + 0.1, d = mount_post_diameter);
}

module lid_mounts() {
    oled_post_positions() mount_post();
    trrs_post_positions() mount_post();
    for (y = trrs_hole_y)
        translate([trrs_origin[0] + 2.2,
                   y,
                   lid_thickness - 0.1])
            cylinder(h = mount_standoff + 0.1, d = 3);
}

module snap_window_cuts() {
    y0 = wall + fit_clearance;
    for (x = snap_centers) {
        translate([x - 3, y0 - 0.1, 3.15])
            cube([6, skirt_thickness + 0.2, snap_height + 0.1]);
        translate([x - 3,
                   outer_width - wall - fit_clearance - skirt_thickness - 0.1,
                   3.15])
            cube([6, skirt_thickness + 0.2, snap_height + 0.1]);
    }
}

module mount_pilot_cuts() {
    oled_post_positions()
        translate([0, 0, mount_pilot_start])
            cylinder(h = lid_thickness + mount_standoff - mount_pilot_start + 0.2,
                     d = mount_pilot_diameter);
    trrs_post_positions()
        translate([0, 0, mount_pilot_start])
            cylinder(h = lid_thickness + mount_standoff - mount_pilot_start + 0.2,
                     d = mount_pilot_diameter);
}

module lid() {
    difference() {
        union() {
            rounded_prism([outer_length, outer_width, lid_thickness], corner_radius);
            lid_skirt();
            lid_mounts();
        }

        translate([oled_center[0] - oled_window[0] / 2,
                   oled_center[1] - oled_window[1] / 2,
                   -0.1])
            cube([oled_window[0], oled_window[1], lid_thickness + 0.2]);

        snap_window_cuts();
        mount_pilot_cuts();
    }
}

module base_component_preview() {
    color("black") translate(seed_origin) cube(seed_envelope);
    color("darkslategray")
        translate([button_center[0] - button_body[0] / 2,
                   outer_width - wall - button_body[1],
                   floor])
            cube(button_body);
}

module lid_component_preview() {
    color("navy")
        translate([oled_origin[0],
                   oled_origin[1],
                   lid_thickness + mount_standoff])
            cube(oled_board);
    color("royalblue")
        translate([trrs_origin[0],
                   trrs_origin[1],
                   lid_thickness + mount_standoff])
            cube(trrs_board);
}

module position_lid_for_assembly() {
    translate([0, outer_width, base_height + lid_thickness])
        rotate([180, 0, 0]) children();
}

module assembly() {
    color("gainsboro", 0.8) base();
    position_lid_for_assembly() color("whitesmoke", 0.8) lid();
    base_component_preview();
    position_lid_for_assembly() lid_component_preview();
}

module snap_fit_test() {
    coupon_x = snap_centers[0];
    coupon_width = 12;
    coupon_z = snap_anchor_z - 1;
    translate([-coupon_x + coupon_width / 2, 0, -coupon_z])
        intersection() {
            base();
            translate([coupon_x - coupon_width / 2, 0, coupon_z])
                cube([coupon_width, 6, base_height - coupon_z]);
        }
    translate([-coupon_x + coupon_width / 2, 10, 0])
        intersection() {
            lid();
            translate([coupon_x - coupon_width / 2, 0, 0])
                cube([coupon_width, 6, lid_thickness + skirt_depth]);
        }
}

if (part == "base") {
    base();
} else if (part == "lid") {
    lid();
} else if (part == "assembly") {
    assembly();
} else if (part == "snap-fit-test") {
    snap_fit_test();
} else {
    base();
    translate([0, outer_width + 8, 0]) lid();
}
