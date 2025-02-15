<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
  #version 300 es
  precision highp float;

  uniform samplerCube s_texture6;
  in vec2 v_texture;
  out vec4 fragColor;

  const float PI = 3.1415926535897932384626433832795;
  uniform float Exposure;
  uniform int lodLevel;

  vec3 uvToXYZ(vec2 uv) {
      // Convert equirectangular UV to spherical coordinates
      float phi = uv.x * 2.0 * PI - PI;       // -PI to PI
      float theta = uv.y * PI;                // 0 to PI
    
      // Convert spherical coordinates to Cartesian coordinates
      float x = cos(phi) * sin(theta);
      float y = cos(theta);
      float z = sin(phi) * sin(theta);
    
      return vec3(x, y, z);
  }

  void main() {
      // Input UV is in [0,1] range for equirectangular projection
      vec2 uv = v_texture;
    
      // Convert UV to 3D direction vector
      vec3 direction = uvToXYZ(uv);
    
      // Sample the cubemap using the direction vector
	    vec3 color = textureLod(s_texture6, direction, float(lodLevel)).rgb;
    
      // TODO: Inverse exposure
      color = -log(1.0 - color) / Exposure;
    
      fragColor = vec4(color, 1.0);
  }
	-->
	</source>
</shader>
