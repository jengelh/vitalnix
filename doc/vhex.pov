/*
 *	Vitalnix three-hexagons logo
 *	by Jan Engelhardt
 *	original idea: 2008-12-17
 *	Povray implementation: 2009-09-04
 *	Licensed under CC-BY-SA 3.0
 */
#include "colors.inc"

global_settings {
	photons {
		spacing 0.1
	}
}

background {
	color rgb 224/225
}

#declare rad = 0.05;
#declare h = 0.866025;

#declare min_x = -1 - rad;
#declare max_x = 2 + rad;
#declare min_y = -2 * h;
#declare max_y = 2 * h;
#declare min_z = -2 * rad - rad;
#declare max_z = 2 * rad + rad;

/* union {
	cylinder { <min_x, min_y, min_z>, <max_x, max_y, max_z>, rad/4 }

	cylinder { <min_x, min_y, min_z>, <max_x, min_y, min_z>, rad/4 }
	cylinder { <min_x, max_y, min_z>, <max_x, max_y, min_z>, rad/4 }
	cylinder { <min_x, min_y, max_z>, <max_x, min_y, max_z>, rad/4 }
	cylinder { <min_x, max_y, max_z>, <max_x, max_y, max_z>, rad/4 }

	cylinder { <min_x, min_y, min_z>, <min_x, max_y, min_z>, rad/4 }
	cylinder { <min_x, min_y, max_z>, <min_x, max_y, max_z>, rad/4 }
	cylinder { <max_x, min_y, min_z>, <max_x, max_y, min_z>, rad/4 }
	cylinder { <max_x, min_y, max_z>, <max_x, max_y, max_z>, rad/4 }

	pigment { color Red }
} */

camera {
	location <(min_x + max_x) / 2, (min_y + max_y) / 2, -3.8>
	look_at <(min_x + max_x) / 2, (min_y + max_y) / 2, 0>
	right 0.5
}

light_source { <min_x, min_y, -1> color 0.75 * White }
light_source { <max_x, max_y, -1> color 0.75 * White }

#macro vhex_hexagon()
	union {
		polygon {
			7,
			<0.5, h, 0>
			<1, 0, 0>
			<0.5, -h, 0>
			<-0.5, -h, 0>
			<-1, 0, 0>
			<-0.5, h, 0>
			<0.5, h, 0>
			pigment { color rgbt <187/255, 198/255, 208/255, 64/255> }
			finish { ambient 0.6 }
		}
		merge {
			sphere { <0.5, h, 0>, rad }
			sphere { <1, 0, 0>, rad }
			sphere { <0.5, -h, 0>, rad }
			sphere { <-0.5, -h, 0>, rad }
			sphere { <-1, 0, 0>, rad }
			sphere { <-0.5, h, 0>, rad }
			cylinder { <0.5, h, 0>, <1, 0, 0>, rad }
			cylinder { <1, 0, 0>, <0.5, -h, 0>, rad }
			cylinder { <0.5, -h, 0>, <-0.5, -h, 0>, rad }
			cylinder { <-0.5, -h, 0>, <-1, 0, 0>, rad }
			cylinder { <-1, 0, 0>, <-0.5, h, 0>, rad }
			cylinder { <-0.5, h, 0>, <0.5, h, 0>, rad }
#if(defined(VHEX_MATTE))
			pigment { color rgb 192/255 }
			finish { specular 1 }
#else
			pigment { color rgb <224/255,224/255,255/255> }
			finish { specular 1 }
#end
		}
	}
#end

vhex_hexagon()
object {
	vhex_hexagon()
	translate <0.5, -h, -2 * rad>
}
object {
	vhex_hexagon()
	translate <1, h, 2 * rad>
}
