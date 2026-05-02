from cadquery import *

# parameters
dist_h=3
screw=4

#esp32c3 board
esp_w=18
esp_l=21.2
esp_h=5.5

#antenna connector
ant_con_x=3.5
ant_con_y=2.5

#sd card SPI interface
sd_w=17
sd_l=22.5
sd_h=3.5
sd_overhang=10.7
sd_card_offs_h=2
sd_card_h=1.5
sd_card_w=11+1 #+1=tolerance

#LiIon battery
bat_w=12
bat_l=22.5
bat_h=6

#on/off switch
swi_w=3.9
swi_l=8.7
swi_h=4.5
butt_h=3.2 #height off nubsy to switch

#helpers
wall_t=2.5
sd_offset=sd_overhang-wall_t
inner_l=esp_l+sd_l+sd_offset
inner_h=esp_h+bat_h+1 #+1=tolerance
inner_w=esp_w
hook_screw_d=4 #m4
lid_lip=1
screw_lip=4
screw=1.5
tol=0.3 #tolerance


# make the Object
esp = (
    Workplane("XY")
    .box(esp_w, esp_l, esp_h)
    .union(
        Workplane("XY")
        .box(9.5,4,3.5)
        .translate([0,esp_l/2+2,1.5])
        .edges("|Y")
        .fillet(1)
        )
    .translate([0,inner_l/2-esp_l/2,esp_h/2])
    )

sd = (
    Workplane("XY")
    .box(sd_w, sd_l, sd_h)
    .union(
        Workplane("XY")
        .box(sd_card_w,sd_overhang,sd_card_h)
        .translate([0,sd_l/2+sd_overhang/2,-sd_card_offs_h/2])
        )
    .rotate([0,0,0],[0,0,1],180)
    .translate([0,-inner_l/2+sd_l/2+sd_offset,sd_h/2])
    )

bat = (
    Workplane("XY")
    .box(bat_w, bat_l, bat_h)
    .translate([0,-inner_l/2+bat_l/2,esp_h+bat_h/2])
    )

swi_protrusion=1.5
swi = (
    Workplane("XY")
    .box(swi_w, swi_l, swi_h)
    .union(
        Workplane("XY")
        .box(butt_h,5,2)
        .translate([-(swi_w/2+butt_h/2),0,0])
        )
    .rotate([0,0,0],[0,0,1],-90)
    .translate([0,-swi_h/2+inner_l/2+swi_protrusion,-swi_h/2+esp_h+bat_h])
       # ,loc=Location(z=bat_h/2+esp_h/2+1,x=-(esp_w-swi_w)/2)
    )
ant = (
    Workplane("XY")
    .box(wall_t,ant_con_x,ant_con_y)
    .translate([inner_w/2+wall_t/2,0,inner_h/2])
    )
hook_screw = (
    Workplane("XZ")
    .cylinder(wall_t,hook_screw_d/2)
    .translate([0,-(inner_l/2+wall_t/2),inner_h/2])
    )

inner_wall = (
    Workplane("XY")
    .box(inner_w,inner_l,inner_h)
    .translate([0,0,inner_h/2])
)

outer_wall = (
    Workplane("XY")
    .box(inner_w+wall_t*2,inner_l+wall_t*2,inner_h+wall_t*2)
    .translate([0,0,inner_h/2])
)

sd_cutout = (
    Workplane("XY")
    .cylinder(wall_t,sd_card_w-0.3)
    .translate([0,-inner_l/2-sd_card_w,-wall_t/2])
)

lidcut = (
    Workplane("XY")
    .box(inner_w+wall_t,inner_l,wall_t)
    .edges("|Y")[2]
    .fillet(1)
    .union(
        Workplane()
        .box((inner_w+wall_t)/3,inner_l+wall_t,wall_t/2)
        .translate([(inner_w+wall_t)/3,0,-wall_t/4])
        )
    .union(
        Workplane()
        .box(wall_t,7,wall_t/2)
        .translate([-inner_w/2-wall_t,0,-wall_t/4])
    )
)
lid = (
    Workplane("XY")
    .box(inner_w+wall_t-tol,inner_l-tol*2,wall_t)
    .translate([tol/2,0,0])
    .edges("|Y")[2]
    .fillet(1)
    .union(
        Workplane()
        .box((inner_w+wall_t)/3-tol*2,inner_l+wall_t-tol,wall_t/2-tol/2)
        .translate([(inner_w+wall_t)/3+tol,0,-wall_t/4-tol/4])
        )
    .union(
        Workplane()
        .box(wall_t+tol,7-tol*2,wall_t/2-tol/2)
        .translate([-inner_w/2-wall_t+tol/2,0,-wall_t/4-tol/4])
    )
)

lip = (
    Workplane("XY")
    .box(screw_lip,screw_lip,screw_lip)
    .translate([screw_lip/2-inner_w/2,-screw_lip/2+inner_l/2,-screw_lip/2+inner_h])
    .faces(">Z")
    .hole(screw)
    )

box = (
    outer_wall
    .edges("|Z")
    .fillet(1)
    .edges("|X")
    .fillet(1)
    .cut(inner_wall)
    .cut(swi)
    .cut(esp)
    .cut(sd)
    .cut(ant)
    .cut(hook_screw)
    .cut(lidcut.translate([wall_t/2,0,inner_h+wall_t/2]))
    .cut(sd_cutout)
    .union(lip)
)

lid= (lid
    .faces(">Z")
    .workplane()
    .rect(inner_w+wall_t-screw_lip, inner_l-screw_lip, forConstruction=True)
    .vertices()[3]
    .hole(screw)
    .translate([wall_t/2,0,inner_h+wall_t/2])
)

stack = (
    Assembly()
    .add(esp,color=Color(0,0,1,0.3))
    .add(sd
        ,color=Color(1,1,1,0.3)
        )
    .add(bat
        ,color=Color(0.5,0.5,0.5,0.3)
        )
    .add(swi
        ,color=Color(0,0,0,0.3)
        )
     .add(ant
        ,color=Color(1,0,0,0.3)
        )
    .add(hook_screw
        ,color=Color(0,1,0,0.3)
        )
    )

walls = (
    Assembly()
    .add(inner_wall
        ,color=Color(0.8,0.5,0.5,0.1)
        )
    .add(outer_wall
        ,color=Color(0.3,0.3,0.5,0.1)
        )
)


parts = (
    Assembly()
    .add(box
        ,color=Color(0.2,0.5,0.2,0.3)
        )
    .add(lid
        ,color=Color(0.5,0.2,0.2,0.3)
        )
    )
# Render Object
show_object(stack)
#show_object(walls)
show_object(parts)
#show_object(lip)

# Export
exporters.export(lid, "keychain_lid_long.stl")
exporters.export(box, "keychain_box_long.stl")