<shader>
  <type name = "vertexShader" />
  <uniform name = "ProjectViewModel" />
  <source>
	<!--
      #version 300 es
      precision highp int;
      
      layout(location = 0) in vec3 vPosition;
      layout(location = 1) in vec3 vNormal;
      layout(location = 2) in vec2 vTexture;
      layout(location = 3) in vec3 vBiTan;
      
      uniform mat4 ProjectViewModel;
      
      out vec2 v_texture;
      
      void main()
      {
        gl_Position = ProjectViewModel * vec4(vPosition, 1.0f);
        v_texture   = vTexture;
      }
	-->
	</source>
</shader>