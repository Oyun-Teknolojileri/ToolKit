<shader>
  <type name = "fragmentShader" />
  <uniform name = "LightData" />
  <uniform name = "CamData" />
  <source>
  <!--
    #version 300 es
    precision mediump float;

    // Fixed Declaretions
    struct _LightData
    {
      vec3 pos;
      vec3 dir;
      vec3 color;
	  float intensity;
    };

    struct _CamData
    {
      vec3 pos;
      vec3 dir;
    };

    uniform _LightData LightData;
    uniform _CamData CamData;
    uniform sampler2D s_texture;

    in vec3 v_pos;
    in vec3 v_normal;
    in vec2 v_texture;

    out vec4 fragColor;

    void main()
    {
	  vec3 l = -LightData.dir;
      vec3 n = normalize(v_normal);
      vec3 e = normalize(CamData.pos - v_pos);
	  
	  // ambient
      float ambientStrength = 0.1;
      vec3 ambient = ambientStrength * LightData.color;
	  
	  // diffuse 
      float diff = max(dot(n, l), 0.0);
      vec3 diffuse = diff * LightData.color;
	  
	  // specular
	  float specularStrength = 8.5;
	  vec3 reflectDir = reflect(-l, n);  
      float spec = pow(max(dot(e, reflectDir), 0.0), 32.0);
      vec3 specular = specularStrength * spec * LightData.color;   
	  
	  vec4 objectColor = texture(s_texture, v_texture);
	  fragColor = vec4((ambient + diffuse + specular) * LightData.intensity, 1.0) * objectColor;
    }
  -->
  </source>
</shader>