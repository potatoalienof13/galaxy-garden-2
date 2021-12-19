#line 2
// NUM_USED and NUM_POINTS are defined before the start of this file.
// The #defines for them are prepended to this file.

// This file contains some lines of commented out code.
// This is not because I changed it and forgot to remove the old version.
// It is so those lines of code can be swapped in quickly, replacing
// other pieces of code that do similar things.

#define PI 3.1415926535897932384626433832795

out vec4 FragColor;

uniform float time;
uniform vec2 points[NUM_POINTS];
uniform vec3 point_colors[NUM_POINTS];
uniform float speed;

float distances[NUM_POINTS];

void rotate(inout vec2 p0, in float angle)
{
	p0 = vec2(p0.x * cos(angle) - p0.y * sin(angle), p0.x * sin(angle) - p0.y * cos(angle));
}

float get_angle_with_rotation(in float adj, in vec2 v1, in vec2 v2)
{
	vec2 change = v1 - v2;
	change = normalize(change);
	rotate(change, adj);
	
	return abs(atan(change.x / change.y));
	//return abs(change.x) / 10;
}

float sorting_ranker(in vec2 point, in vec3 pc)
{
	float adj = time * 1 + pc.r * 2 * PI;
	float dist = distance(point, gl_FragCoord.xy);
	float angle = get_angle_with_rotation(adj, point, gl_FragCoord.xy);
#ifdef SORT_USE_BODY
	SORT_ALGO
#else
	return SORT_ALGO;
#endif
}

float value_ranker(in vec2 point, in vec3 pc, in float value)
{
	float adj = time * 1 + pc.r * 2 * PI;
	float dist = distance(point, gl_FragCoord.xy);
	float angle = get_angle_with_rotation(adj, point, gl_FragCoord.xy);
#ifdef VALUE_USE_BODY
	VALUE_ALGO
#else
	return VALUE_ALGO;
#endif
}

void main()
{
	FragColor = vec4(0., 0., 0., 0.);
	
	int top_distance_indexes[NUM_USED];
	float total_distance = 0;
	
	// Distance ranking.
	for (int i = 0; i < points.length(); i++)
		distances[i] = sorting_ranker(points[i], point_colors[i]);
		
	//distances[0] = 100;
	
	// The loop after needs this array to already be initialized
	for (int i = 0; i < NUM_USED; i++)
		top_distance_indexes[i] = i;
		
	// This for loop adds the indexes of the top values in
	// Start at NUM_USED because the slots before that were already filled.
	for (int i = NUM_USED; i < points.length(); i++) {
		int set_flag = 0;
		
		for (int j = 0; j < NUM_USED; j++) {
			if (distances[i] ORDERING  distances[top_distance_indexes[j]]) {
				set_flag = 1;
				break;
			}
		}
		
		if (set_flag == 1) {
			int least_distance = 0;
			
			for (int j = 0; j < NUM_USED; j++) {
				if (distances[top_distance_indexes[least_distance]] ORDERING  distances[top_distance_indexes[j]])
					least_distance = j;
			}
			
			top_distance_indexes[least_distance] = i;
		}
	}
	
	int least_distance = 0;
	
	for (int j = 1; j < NUM_USED; j++) {
		if (distances[top_distance_indexes[least_distance]] <  distances[top_distance_indexes[j]])
			least_distance = j;
	}
	
	float real_least_distance = distances[top_distance_indexes[least_distance]];
	
	// This just counts up the total distance, so it can be used in the next for loop.
	for (int i = 0; i < NUM_USED; i++) {
		//distances[top_distance_indexes[i]] = distance(points[top_distance_indexes[i]], gl_FragCoord.xy);
		//distances[top_distance_indexes[i]] = sin(distances[top_distance_indexes[i]] * 0.1  + 1);
		//distances[top_distance_indexes[i]] -= real_least_distance;
		distances[top_distance_indexes[i]] = value_ranker(points[top_distance_indexes[i]],
		                                                  point_colors[top_distance_indexes[i]], distances[top_distance_indexes[i]]);
		total_distance += distances[top_distance_indexes[i]];
	}
	
	// This line of code actually creates the return color, from the distances and point_colors given.
	for (int i = 0; i < NUM_USED; i++) {
#ifdef ROTATE_COLORS
		vec3 color = (sin(point_colors[top_distance_indexes[i]] * 2 * PI * time +
		                  PI * 2 * point_colors[top_distance_indexes[i - 1]]) + 1) / 2;
#else
		vec3 color = point_colors[top_distance_indexes[i]];
#endif
		FragColor += vec4(distances[top_distance_indexes[i]] / total_distance * color, 1.);
	}
	
	// This draws a circle around all points, useful for making sure things work.
#ifdef DRAW_CIRCLES
	for (int i = 0; i < NUM_POINTS; i++) {
		if (distance(points[i], gl_FragCoord.xy) < 10)
			FragColor = vec4(1., 1., 1., 1.);
	}
#endif
}