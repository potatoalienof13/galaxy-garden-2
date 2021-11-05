// NUM_USED and NUM_POINTS are defined before the start of this file.
// The #defines for them are prepended to this file. 

// This file contains some lines of commented out code.
// This is not because I changed it and forgot to remove the old version.
// It is so those lines of code can be swapped in quickly, replacing
// other pieces of code that do similar things.

out vec4 FragColor;

uniform float time;
uniform vec2 points[NUM_POINTS];
uniform vec3 point_colors[NUM_POINTS];

float distances[NUM_POINTS];

void main()
{
	FragColor = vec4(0., 0., 0., 0.);
	
	int top_distance_indexes[NUM_USED];
	float total_distance = 0;

	// Distance ranking. 
	for (int i = 0; i < points.length(); i++) {
		distances[i] = sin(distance(points[i], gl_FragCoord.xy) / 30.) + 1;
		//distances[i] = distance(points[i], gl_FragCoord.xy);
	}

	// This for loop adds some reasonable numbers into top_distance_indexes.
	// The for loop after this one needs those reasonable numbers. 
	for (int i = 0; i < NUM_USED; i++) {
		top_distance_indexes[i] = i;
	}

	// This puts the indexes of the points with the lowest distance into top_distance_indexes
	for (int i = 0; i < points.length(); i++) {
		int set_flag = 0;
		
		for (int j = 0; j < NUM_USED; j++) {
			if (distances[i] < distances[top_distance_indexes[j]]) {
				set_flag = 1;
				break;
			}
		}
		if (set_flag == 1) {
			int least_distance = 0; 
			
			for (int j = 0; j < NUM_USED; j++) {
				if (distances[top_distance_indexes[j]] > distances[top_distance_indexes[least_distance]]) {
					least_distance = j;
				}
			}
			top_distance_indexes[least_distance] = i; 
		}
		
	}
	// This just counts up the total distance, so it can be used in the next for loop. 
	for(int i = 0; i < NUM_USED; i++){
		//distances[top_distance_indexes[i]] = sin(distances[top_distance_indexes[i]] / 30.) + 1;
		total_distance += distances[top_distance_indexes[i]];
	}

	// This line of code actually creates the return color, from the distances and point_colors given.
	for (int i = 0; i < NUM_USED; i++) { 
		vec3 color = (sin(point_colors[top_distance_indexes[i]] * 360 + time) + 1) / 2;
		FragColor += vec4(distances[top_distance_indexes[i]] / total_distance * color, 1.);
	}

	// This draws a circle around all points, useful for making sure things work. 
	/*
	for (int i = 0; i < NUM_POINTS; i++){
		if(distances[i] < 10){
			FragColor = vec4(1.,1.,1., 1.);
		}
	}
	*/ 
}

