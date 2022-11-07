<shader>
	<type name = "vertexShader" />
   <include name = "skinning.shader" />
	<uniform name = "ProjectViewModel" />
	<uniform name = "InverseTransModel" />
	<uniform name = "Model" />
	<source>
	<!--
      #version 300 es

      layout(location = 0) in vec3 vPosition;
      layout(location = 1) in vec3 vNormal;
      layout(location = 2) in vec2 vTexture;
      layout(location = 3) in vec3 vBiTan;

      uniform mat4 ProjectViewModel;
      uniform mat4 InverseTransModel;
      uniform mat4 Model;
      uniform uint isSkinned;
      out vec3 v_pos;
      out vec3 v_normal;
      out vec2 v_texture;
      out vec3 v_bitan;

      void main()
      {
         gl_Position = vec4(vPosition, 1.0f);
         if(isSkinned > 0u){
            gl_Position = skin(gl_Position);
         }
         
         v_pos = (Model * gl_Position).xyz;
         gl_Position = ProjectViewModel * gl_Position;
         v_texture = vTexture;
         v_normal = (InverseTransModel * vec4(vNormal, 1.0)).xyz;
         v_bitan = vBiTan;
      }
	-->
	</source>
</shader>