#version 330

in vec3 vertex_position; // 3D world position of vertex
in vec2 vertex_texcoord; // UV texture coordinates
in vec3 vertex_normal;

// Uniforms (set from c code)
uniform mat4 mvp;			// Model view projection matrix 
uniform mat4 mat_model;

// Outputs to fragment shader
out vec2 frag_texcoord;	// Pass texture coordinates to fragment shader
out vec3 frag_worldpos;
out vec3 frag_normal;

mat3 inverse(mat3 m) {
  float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];
  float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];
  float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];

  float b01 = a22*a11 - a12*a21;
  float b11 = -a22*a10 + a12*a20;
  float b21 = a21*a10 - a11*a20;

  float det = a00*b01 + a01*b11 + a02*b21;

  return mat3(b01, (-a22*a01 + a02*a21), (a12*a01 - a02*a11),
              b11, (a22*a00 - a02*a20), (-a12*a00 + a02*a10),
              b21, (-a21*a00 + a01*a20), (a11*a00 - a01*a10))/det;
}

mat3 transpose(mat3 m) {
  return mat3(m[0][0], m[1][0], m[2][0],
              m[0][1], m[1][1], m[2][1],
              m[0][2], m[1][2], m[2][2]);
}

void main() {
	// Forward texture coordinates
	frag_texcoord = vertex_texcoord;
	frag_worldpos = vec3(mat_model * vec4(vertex_position, 1.0));

    mat3 normal_matrix = transpose(inverse(mat3(mat_model)));
    frag_normal = normalize(normal_matrix * vertex_normal);

	gl_Position = mvp * vec4(vertex_position, 1.0);
}
