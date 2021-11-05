
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
	
	for (int i = 0; i < points.length(); i++) {
		distances[i] = sin(distance(points[i], gl_FragCoord.xy) / 30.) + 1;
	}
	
	for (int i = 0; i < NUM_USED; i++) {
		top_distance_indexes[i] = i;
	}
	
	int lowest_included_distance = 0;
	
	for (int i = 0; i < points.length(); i++) {
		int set_flag = 0;
		
		for (int j = 0; j < NUM_USED; j++) {
			if (distances[i] > distances[top_distance_indexes[j]]) {
				set_flag = 1;
				break;
			}
		}
		if (set_flag == 1) {
			int least_distance = 0; 
			
			for (int j = 0; j < NUM_USED; j++) {
				if (distances[top_distance_indexes[j]] < distances[top_distance_indexes[least_distance]]) {
					least_distance = j;
				}
			}
			top_distance_indexes[least_distance] = i; 
		}
		
	}

	for(int i = 0; i < NUM_USED; i++){
		total_distance += distances[top_distance_indexes[i]];

	}

	
	for (int i = 0; i < NUM_USED; i++) {
		FragColor += vec4(distances[top_distance_indexes[i]] / total_distance * point_colors[top_distance_indexes[i]], 1.);
	}
	
}

