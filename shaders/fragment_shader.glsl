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
uniform ivec2 window_size;

vec4 fg = gl_FragCoord;

// Only way I could find to generalize these functions.
#define CREATE_PFUNC(type) type psin(in type f) { return sin(f) + 1 ; }; type pcos(in type f) { return cos(f) + 1 ; }
CREATE_PFUNC(float);
CREATE_PFUNC(vec2);
CREATE_PFUNC(vec3);
CREATE_PFUNC(vec4);

float distances[NUM_POINTS];

vec2 rotate(in vec2 v, float a) {
	float s = sin(a);
	float c = cos(a);
	mat2 m = mat2(c, -s, s, c);
	return m * v;
}



vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+10.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  { 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.5 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 105.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
  }

float get_angle_with_rotation(in float adj, in vec2 v1, in vec2 v2)
{
	vec2 change = v1 - v2;
	change = normalize(change);
	change = rotate(change, adj);
	
	// using atan for the angle is working kinda weird, i switched it out for this weirdness
	return abs(atan(change.x / change.y));
	//return abs(change.x) / 10;
}

float sorting_ranker(in vec2 point, in vec3 pc, int index)
{
	float adj = time * ROTATE_ANGLE * (pc.r - 0.5) * 2 * PI + pc.g * 2 * PI;
	float dist = distance(point, fg.xy);
	float angle = get_angle_with_rotation(adj, point, fg.xy);
#ifdef SORT_USE_BODY
	SORT_ALGO
#else
	return SORT_ALGO;
#endif
}

float value_ranker(in vec2 point, in vec3 pc, in float value, int index)
{
	float adj = time * ROTATE_ANGLE * (pc.r - 0.5) * 2 * PI + pc.g * 2 * PI;
	float dist = distance(point, fg.xy);
	float angle = get_angle_with_rotation(adj, point, fg.xy);
#ifdef VALUE_USE_BODY
	VALUE_ALGO
#else
	return VALUE_ALGO;
#endif
}

void main()
{
	FragColor = vec4(0., 0., 0., 0.);
	fg.xy += gl_SamplePosition; // For multisampling
	
	PRERUN_BLOCK // Expands to -p argument
	
	int top_distance_indexes[NUM_USED];
	float total_distance = 0;
	
	// Distance ranking.
	for (int i = 0; i < points.length(); i++)
		distances[i] = sorting_ranker(points[i], point_colors[i], i);
		
		
	// Add the indexes of the points with the most valueable distance values to top_distance_indexes
	for (int i = 0; i < NUM_USED; i++)
		top_distance_indexes[i] = i;
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
	
	// This just counts up the total distance, so it can be used in the next for loop.
	for (int i = 0; i < NUM_USED; i++) {
#ifdef VALUE_ALGO
		distances[top_distance_indexes[i]] = value_ranker(points[top_distance_indexes[i]],
		                                                  point_colors[top_distance_indexes[i]], distances[top_distance_indexes[i]], top_distance_indexes[i]);
#endif
		total_distance += distances[top_distance_indexes[i]];
	}
	
	// This line of code actually creates the return color, from the distances and point_colors given.
	for (int i = 0; i < NUM_USED; i++) {
#ifdef ROTATE_COLORS
		vec3 color = (sin(point_colors[top_distance_indexes[i]] * 2 * PI * time * ROTATE_COLORS +
		                  PI * 2 * point_colors[top_distance_indexes[i]]) + 1) / 2;
#else
		vec3 color = point_colors[top_distance_indexes[i]];
#endif
		FragColor += vec4(distances[top_distance_indexes[i]] / total_distance * color, 1.);
	}
	
	// This draws a circle around all points, useful for making sure things work.
#ifdef DRAW_CIRCLES
	for (int i = 0; i < NUM_POINTS; i++) {
		if (distance(points[i], fg.xy) < 10)
			FragColor = vec4(1., 1., 1., 1.);
	}
#endif
}
